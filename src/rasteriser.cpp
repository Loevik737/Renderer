#include "rasteriser.hpp"
#include "utilities/lodepng.h"
#include <vector>
#include <chrono>

// --- Overview ---

// I'm going to assume most of you who are reading through this file have never worked with computer graphics before.
// Let me therefore start by explaining the general idea of this file a little bit, because you'll need to at least _slightly_
// understand what is going on in this program if you are to optimise it effectively.

// We start off with two lists; one with vertices, and one with indices.
// Vertices are three-dimensional coordinates (even though in this file they have the type float4, where the last coordinate is always 1. The reason why is a long story).
// Indices represent the indices of vertices in the vertex buffer that should be connected together,
// much like you do in "connect the dots". Except here every three indices define a triangle.
// So for instance, index 0, 1, and 2 reference the three vertices of the first triangle. Indices 3, 4, and 5 the next. And so on.

// Rendering an image is done in two steps.
// First, all vertices are multiplied with a so-called "transformation matrix" in a section / program called the "vertex shader".
// How it works is not really relevant now. All you should know is that its purpose is to move objects around on the scene, and apply a "perspective" effect
// that causes objects to appear smaller the further they move away from the camera.
// The entire process only requires that all vertices are multiplied by a 4x4 matrix, using regular matrix multiplication (4x4 matrix with a 4x1 vertex)
// The second main stage involves rasterising the transformed triangles
// A "depth buffer" is used to make sure that triangles that should appear in front of others
// A "fragment shader" is run on each rendered pixel, which determines the colour the pixel should receive.
// The rendered image is stored in the so-called "framebuffer".


/**
 * Executes the vertex shader, transforms vertices and normals of the mesh object
 * @param mesh                    Mesh object with all vertices and normals
 * @param transformedVertexBuffer returned transformed vertices
 * @param transformedNormalBuffer returned transformed normals
 */
void runVertexShader( Mesh &mesh,
					  std::vector<float4> &transformedVertexBuffer,
					  std::vector<float4> &transformedNormalBuffer )
{
	// The & in front of the variable names cause the function to modify variables from the function
	// calling this one, rather than making a copy of them.

	// The matrices defined below are the ones used to transform the vertices and normals.

	// This projection matrix assumes a 16:9 aspect ratio, and an field of view (FOV) of 90 degrees.
	mat4x4 projectionMatrix(
		0.347270,   0, 			0, 		0,
		0,	  		0.617370, 	0,		0,
		0,	  		0,			-1, 	-0.2f,
		0,	  		0,			-1,		0);

	mat4x4 viewMatrix(
		0.5, 	0, 		0, 		5,
		0, 		-0.5, 	0, 		30,
		0, 		0, 		0.5, 	-55,
		0, 		0, 		0, 		1);

	mat4x4 normalMatrix(
		1, 0, 0, 0,
		0, 1, 0, 0,
		0, 0, 1, 0,
		0, 0, 0, 1);

	mat4x4 MVP = projectionMatrix * viewMatrix;

	for(unsigned int i = 0; i < transformedVertexBuffer.size(); i++) {
		float4 transformed = MVP * mesh.vertices[i];
		transformed.x /= transformed.w;
		transformed.y /= transformed.w;
		transformed.z /= transformed.w;
		transformedVertexBuffer.at(i) = transformed;
	}

	for(unsigned int j = 0; j < transformedNormalBuffer.size(); j++) {
		float4 normal;
		normal.x = mesh.normals[j].x;
		normal.y = mesh.normals[j].y;
		normal.z = mesh.normals[j].z;
		normal.w = 1;
		transformedNormalBuffer.at(j) = normalMatrix * normal;
	}

}

/**
 * Executes the fragment shader to calculate the colour of the pixel from its
 * normal
 * @param normal triangle pixel normal
 * @return		 colour of the pixel in RGBA
 */
inline std::vector<unsigned char> runFragmentShader( float3 const normal )
{
	std::vector<unsigned char> pixelColour(4);
	const float3 lightDirection(0.0f, 0.0f, 1.0f);

	// Computing the dot product between the surface normal and a light
	// direction gives a diffuse-like reflection. It looks more than
	// good enough for a few static images.
	float colour = normal.x * lightDirection.x +
		normal.y * lightDirection.y +
		normal.z * lightDirection.z;

	// We first scale the colour value from a range between 0 and 1,
	// to between 0 and 255.
	// Since single bytes are only able to go between 0 and 255,
	// we subsequently clamp the colour to lie within that range.
	unsigned char colourByte = (unsigned char) std::min(255.0f,
		std::max(colour * 255.0f, 0.0f));

	// And this writes the pixel to the pixelColor vector. The first three
	// channels are red, green, and blue. The fourth represents transparency.
	pixelColour.at(0) = colourByte;
	pixelColour.at(1) = colourByte;
	pixelColour.at(2) = colourByte;
	pixelColour.at(3) = 255;
	// This colour vector is supposed to go into the frame buffer
	return pixelColour;
}

/**
 * interpolates given normals using the barycentric weights
 *
 * Using float3 to return all three normals at once
 *
 * @param  n0 normal of triangle vertex
 * @param  n1 normal of triangle vertex
 * @param  n2 normal of triangle vertex
 * @param  w0 barycentric weight
 * @param  w1 barycentric weight
 * @param  w2 barycentric weight
 * @return    interpolated normal
 */
float3 interpolateNormals( float4 const n0,
						   float4 const n1,
					 	   float4 const n2,
					 	   float const w0,
						   float const w1,
						   float const w2 )
{
	float3 res;
	res.x = w0 * n0.x + w1 * n1.x + w2 * n2.x;
	res.y = w0 * n0.y + w1 * n1.y + w2 * n2.y;
	res.z = w0 * n0.z + w1 * n1.z + w2 * n2.z;
	return res;
}

/**
 * converts a vertex from clipping space to screen pixel coordinates
 * @param  vertex vertex
 * @param  width  screen width
 * @param  height screen height
 * @return        vertex in screen pixel coordinates
 */
 float4 convertClippingSpace( float4 const vertex,
							 unsigned int const width,
							 unsigned int const height )
{
	float4 res;
	res.x = (vertex.x + 0.5f) * float(width);
	res.y = (vertex.y + 0.5f) * float(height);
	res.z = vertex.z;
	res.w = vertex.w;
	return res;
}

/**
 * Computing barycentric weights. This is both for determining whether the point
 * lies within the triangle, as well as interpolating coordinates needed for
 * rendering. If you need an explanation, you can find one here:
 * https://codeplea.com/triangular-interpolation
 * Using float3 to return all weights at once (x -> w0, y -> w1, z -> w2)
 *
 * @param  v0 triangle vertex
 * @param  v1 triangle vertex
 * @param  v2 triangle vertex
 * @param  x  screen pixel x-coordinate
 * @param  y  screen pixel y-coordinate
 * @return    barycentric weights of the pixel in relation to the triangle vertices
 */
float3 getTriangleBarycentricWeights( float4 const v0,
									  float4 const v1,
									  float4 const v2,
									  unsigned int const x,
									  unsigned int const y )
{
	float3 res;
	float divisor = (((v1.y - v2.y) * (v0.x - v2.x)) + ((v2.x - v1.x) * (v0.y - v2.y)));
	res.x = (((v1.y - v2.y) * (x    - v2.x)) + ((v2.x - v1.x) * (y    - v2.y))) / divisor;
	res.y = (((v2.y - v0.y) * (x    - v2.x)) + ((v0.x - v2.x) * (y    - v2.y))) / divisor;
	res.z = 1 - res.x - res.y;
	return res;
}

/**
 * Calculates the triangle's pixel depth from the barycentric weights
 * @param  v0 triangle vertex
 * @param  v1 triangle vertex
 * @param  v2 triangle vertex
 * @param  w0 barycentric weight
 * @param  w1 barycentric weight
 * @param  w2 barycentric weight
 * @return    pixel depth (z)
 */
float getTrianglePixelDepth( float4 const v0,
							 float4 const v1,
							 float4 const v2,
							 float const w0,
							 float const w1,
							 float const w2 )
{
	return w0 * v0.z + w1 * v1.z + w2 * v2.z;
}

/**
 * The main procedure which rasterises all triangles on the framebuffer
 * @param mesh                    Mesh object
 * @param transformedVertexBuffer transformed vertices from the mesh obj
 * @param transformedNormalBuffer transformed normals from the mesh obj
 * @param frameBuffer             frame buffer for the rendered image
 * @param depthBuffer             depth buffer for every pixel on the image
 * @param width                   width of the image
 * @param height                  height of the image
 */
void rasteriseTriangles( Mesh &mesh,
                         std::vector<float4> &transformedVertexBuffer,
                         std::vector<float4> &transformedNormalBuffer,
                         std::vector<unsigned char> &frameBuffer,
                         std::vector<float> &depthBuffer,
                         unsigned int width,
                         unsigned int height )
{
	// We rasterise one triangle at a time
	unsigned int triangleCount = mesh.indexCount / 3;

	// Setting up variables to hold the average time used on different methods.
	unsigned long int pixelLoopTime = 0;
	std::chrono::_V2::system_clock::time_point start;

	// 3d float vector
	float3 *interpolatedNormal = new float3();

	// vertices
	float4 *vertex0 = new float4();
	float4 *vertex1 = new float4();
	float4 *vertex2 = new float4();

	// normals
	float4 *normal0 = new float4();
	float4 *normal1 = new float4();
	float4 *normal2 = new float4();

	// trinagle bounding box
	std::vector<unsigned int> bounding_box(4);

	// triangle x and y values for finding max and min
	std::vector<float> x_values;	
	std::vector<float> y_values;

	//index vector
	std::vector<unsigned int> indexes(3);

	// weight vector 
	float3 weights;

	// interpolated normals
	float3 interpolatedNormals;

	float pixelDepth = 0;

	float normalLength = 0;

	unsigned int pixelBaseCoordinate = 0;

	std::vector<unsigned char> pixelColour;


	for(unsigned int triangleIndex = 0; triangleIndex < triangleCount; triangleIndex++) {
		// '\r' returns to the beginning of the current line
		std::cout << "Rasterising triangle " << (triangleIndex + 1) << "/" << triangleCount << "\r" << std::flush;

		// As vertices are commonly reused within a model, rendering libraries use an
		// index buffer which specifies the indices of the vertices in the vertex buffer
		// which together make up the specific triangle.
		unsigned int triIndex = 3 * triangleIndex;
		indexes[0] = mesh.indices[triIndex + 0];
		indexes[1]= mesh.indices[triIndex + 1];
		indexes[2] = mesh.indices[triIndex + 2];

		// We look up those triangles here
		*vertex0 =  transformedVertexBuffer.at(indexes[0]);
		*vertex1 =  transformedVertexBuffer.at(indexes[1]);
		*vertex2 =  transformedVertexBuffer.at(indexes[2]);

		// Read the normals belonging to each vertex
		*normal0 = transformedNormalBuffer.at(indexes[0]);
		*normal1 = transformedNormalBuffer.at(indexes[1]);
		*normal2 = transformedNormalBuffer.at(indexes[2]);

		// These triangles are still in so-called "clipping space". We first convert them
		// to screen pixel coordinates
		*vertex0 = convertClippingSpace(*vertex0, width, height);
		*vertex1 = convertClippingSpace(*vertex1, width, height);
		*vertex2 = convertClippingSpace(*vertex2, width, height);

		//finding the triangles bounding box
		x_values = {vertex0->x,vertex1->x,vertex2->x};
		y_values = {vertex0->y,vertex1->y,vertex2->y};
		
		bounding_box[0] = x_values.at(std::distance(std::begin(x_values),std::min_element(std::begin(x_values), std::end(x_values))));
		bounding_box[1] = x_values.at(std::distance(std::begin(x_values),std::max_element(std::begin(x_values), std::end(x_values))));
		bounding_box[2] = y_values.at(std::distance(std::begin(y_values),std::min_element(std::begin(y_values), std::end(y_values))));
		bounding_box[3] = y_values.at(std::distance(std::begin(y_values),std::max_element(std::begin(y_values), std::end(y_values))));

		if(width <= bounding_box[1]) bounding_box[1] = width-1;
		if(height <= bounding_box[3]) bounding_box[3] = height-1;

		// We iterate over each pixel on the screen
		start = std::chrono::high_resolution_clock::now();
		for(unsigned int y= bounding_box[2]; y < bounding_box[3]+1; y++) {
			for(unsigned int x = bounding_box[0]; x <= bounding_box[1]+1; x++) {

				//Coordinate of the current pixel in the framebuffer, remember RGBA color code
				pixelBaseCoordinate = 4 * (x + y * width);

				// Calculating the barycentric weights of the pixel in relation to the triangle
				weights = getTriangleBarycentricWeights(*vertex0, *vertex1, *vertex2, x, y);

				// Now we can determine the depth of our pixel
				pixelDepth = getTrianglePixelDepth(*vertex0, *vertex1, *vertex2, weights.x, weights.y, weights.z);

				// But since a pixel can lie anywhere between the vertices, we compute an approximated normal
				// at the pixel location by interpolating the ones from the vertices.
				interpolatedNormals = interpolateNormals(*normal0, *normal1, *normal2, weights.x, weights.y, weights.z);
				interpolatedNormal->x = interpolatedNormals.x;				
				interpolatedNormal->y = interpolatedNormals.y;
				interpolatedNormal->z = interpolatedNormals.z;

				// This process can slightly change the length, so we normalise it here to make sure the lighting calculations
				// appear correct.
				normalLength = std::sqrt( interpolatedNormal->x * interpolatedNormal->x +
					interpolatedNormal->y * interpolatedNormal->y +
					interpolatedNormal->z * interpolatedNormal->z );

				interpolatedNormal->x /= normalLength;
				interpolatedNormal->y /= normalLength;
				interpolatedNormal->z /= normalLength;

				// And we can now execute the fragment shader to compute this pixel's colour.
				pixelColour = runFragmentShader(*interpolatedNormal);

				// Z-clipping discards pixels too close or too far from the camera
				if(pixelDepth >= -1 && pixelDepth <= 1) {
					// The weights have the nice property that if only one is negative, the pixel lies outside the triangle
					if(weights.x >= 0 && weights.y >= 0 && weights.z >= 0) {
						//Have we drawn a pixel above the current?
						if(pixelDepth < depthBuffer.at(y * width + x)) {
							// This pixel is going into the frame buffer,
							// save its depth to skip all next pixels underneath it
							depthBuffer.at(y * width + x) = pixelDepth;

							// Copy the calculated pixel colour into the frame buffer - RGBA
							frameBuffer.at(pixelBaseCoordinate + 0) = pixelColour.at(0);
							frameBuffer.at(pixelBaseCoordinate + 1) = pixelColour.at(1);
							frameBuffer.at(pixelBaseCoordinate + 2) = pixelColour.at(2);
							frameBuffer.at(pixelBaseCoordinate + 3) = pixelColour.at(3);
						}
					}
				}
			}
		}
		pixelLoopTime += std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();
	}
	// finish the progress output with a new line
	std::cout << std::endl;
	std::cout << "Pixle loop takes: " << pixelLoopTime << " milliseconds." << std::endl;

	// Cleanup
	delete interpolatedNormal;

	delete vertex0;
	delete vertex1;
	delete vertex2;

	delete normal0;
	delete normal1;
	delete normal2;

}

/**
 * Procedure to kick of the rasterisation process
 * @param mesh            Mesh object
 * @param outputImageFile path of the output image
 * @param width           width of the output image
 * @param height          height of the output image
 */
void rasterise(Mesh mesh, std::string outputImageFile, unsigned int width, unsigned int height) {
	// We first need to allocate some buffers.

	// The framebuffer contains the image being rendered.
	std::vector<unsigned char> frameBuffer;
	frameBuffer.resize(width * height * 4, 0);

	// The depth buffer is used to make sure that objects closer to the camera occlude/obscure objects that are behind it
	std::vector<float> depthBuffer;
	depthBuffer.resize(width * height, 1);

	// And these two buffers store vertices and normals processed by the vertex shader.
	std::vector<float4> transformedVertexBuffer;
	transformedVertexBuffer.resize(mesh.vertexCount);

	std::vector<float4> transformedNormalBuffer;
	transformedNormalBuffer.resize(mesh.vertexCount);

	// Initializing the framebuffer with RGBA (0,0,0,255), black, no
	// transparency
	for (unsigned int i = 0; i < 4; i++) {
		for (unsigned int y = 0; y < height; y++) {
			for(unsigned int x = 0; x < width; x++) {
				frameBuffer.at(4 * ( x + y * width ) + i) = 0;
				if ( i == 3 ) {
					//Transparency
					frameBuffer.at(4 * ( x + y * width ) + i) = 255;
				}
			}
		}
	}

	std::cout << "Running the vertex shader... " << std::endl;


	runVertexShader(mesh, transformedVertexBuffer, transformedNormalBuffer);

	std::cout << "complete!" << std::endl;
	// timing how long the rasterisation takes
	auto start = std::chrono::high_resolution_clock::now();
	rasteriseTriangles(mesh, transformedVertexBuffer, transformedNormalBuffer, frameBuffer, depthBuffer, width, height);
	auto rasterTime = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::high_resolution_clock::now() - start).count();

	std::cout << "Finished rendering! The rasterizeTriangles function takes: " << rasterTime << " milliseconds." << std::endl;

	std::cout << "Writing image to '" << outputImageFile << "'..." << std::endl;

	unsigned error = lodepng::encode(outputImageFile, frameBuffer, width, height);

	if(error)
	{
		std::cout << "An error occurred while writing the image file: " << error << ": " << lodepng_error_text(error) << std::endl;
	}
}