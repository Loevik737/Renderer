#include "OBJLoader.hpp"
#include <algorithm>
#include <exception>

float3 computeTriangleNormal(std::vector<float4> &vertices, unsigned int baseIndex);

void split(std::vector<std::string>* parts, const std::string &s, char delim) {

	std::stringstream ss;
	ss.str(s);
	std::string item;
	while (std::getline(ss, item, delim)) {
		parts->push_back(item);
	}
}

void deleteEmptyStrings(std::vector<std::string> &list) {
	for (unsigned int i = 0; i < list.size(); i++) {
		std::string item = list.at(i);
		if (item == "" || item == "\r") {
			list.erase(list.begin() + i);
			i--;
		}
	}
}

Mesh loadOBJ(std::string src)
{
	std::vector<std::string> lineParts;
	lineParts.reserve(32);
	std::vector<std::string> faceParts;
	faceParts.reserve(32);

	std::vector<float4> vertices;
	std::vector<float3> normals;

	std::vector<float4> vertexBuffer;
	std::vector<float3> normalBuffer;

	std::vector<unsigned int> indices;

	std::ifstream objFile(src);
	std::string line;

	unsigned int currentIndex = 0;

	if (objFile.is_open()) {
		// int lineNumber = -1;
		while (std::getline(objFile, line)) {
			lineParts.clear();
			split(&lineParts, line, ' ');
			deleteEmptyStrings(lineParts);

			if (lineParts.size() == 0) {
				continue;
			}

			if (lineParts.at(0) == "v") {
				float4 vertex;
				vertex.x = std::stof(lineParts.at(1));
				vertex.y = std::stof(lineParts.at(2));
				vertex.z = std::stof(lineParts.at(3));
				vertex.w = 1;
				vertexBuffer.push_back(vertex);
			}

			if (lineParts.at(0) == "vn") {
				float3 normal;
				normal.x = std::stof(lineParts.at(1));
				normal.y = std::stof(lineParts.at(2));
				normal.z = std::stof(lineParts.at(3));
				normalBuffer.push_back(normal);
			}

			if (lineParts.at(0) == "f") {
				for (int i = 1; i <= 3; i++) {
					faceParts.clear();
					std::string linePart = lineParts.at(i);
					split(&faceParts, linePart, '/');

					int vertexIndex = std::stoi(faceParts.at(0)) - 1;
					float4 vertex = vertexBuffer.at(unsigned(vertexIndex));
					vertices.push_back(vertex);

					int normalIndex = -1;

					if(faceParts.size() == 2) {
						normalIndex = std::stoi(faceParts.at(1)) - 1;
					}
					else if (faceParts.size() == 3 && faceParts.at(1) == "") {
						normalIndex = std::stoi(faceParts.at(2)) - 1;
					}
					else if (faceParts.size() == 3) {
						normalIndex = std::stoi(faceParts.at(2)) - 1;
					}

					// (partially) invalid files may contain normals later, or in some cases not at all.
					// This check accounts for that. If it fails, normals are computed after processing
					// the contents of the current line in the file
					if(normalIndex != -1 && normalBuffer.size() > (unsigned int) normalIndex) {
						normals.push_back(normalBuffer.at(normalIndex));
					}

					indices.push_back(currentIndex);
					currentIndex++;
				}

				// Special case for when the file defines quads (4 blocks with face indices)
				// I know I duplicated the section from above. Please forgive my software maintainability sins..
				if(lineParts.size() == 5) {
					for (int i = 1; i <= 4; i++) {
						if(i == 2) { continue; }

						faceParts.clear();
						std::string linePart = lineParts.at(i);
						split(&faceParts, linePart, '/');

						int vertexIndex = std::stoi(faceParts.at(0)) - 1;
						float4 vertex = vertexBuffer.at(unsigned(vertexIndex));
						vertices.push_back(vertex);

						int normalIndex = -1;

						if(faceParts.size() == 2) {
							normalIndex = std::stoi(faceParts.at(1)) - 1;
						}
						else if (faceParts.size() == 3 && faceParts.at(1) == "") {
							normalIndex = std::stoi(faceParts.at(2)) - 1;
						}
						else if (faceParts.size() == 3) {
							normalIndex = std::stoi(faceParts.at(2)) - 1;
						}

						// (partially) invalid files may contain normals later, or in some cases not at all.
						// This check accounts for that. If it fails, normals are computed after processing
						// the contents of the current line in the file
						if(normalIndex != -1 && normalBuffer.size() > (unsigned int) normalIndex) {
							normals.push_back(normalBuffer.at(normalIndex));
						}

						indices.push_back(currentIndex);
						currentIndex++;
					}
				}


				// If the file incorrectly or was missing normals, we compute them here.
				// Alternatively, we override those present in the file if this was mandated by the use.
			}
		}

        unsigned int faceCount = unsigned(indices.size()) / 3;

		float4* meshVertexBuffer = new float4[vertices.size()];
		std::copy(vertices.begin(), vertices.end(), meshVertexBuffer);

		float3* meshNormalBuffer = new float3[normals.size()];
		std::copy(normals.begin(), normals.end(), meshNormalBuffer);

		unsigned int* meshIndexBuffer = new unsigned int[3 * faceCount];
		std::copy(indices.begin(), indices.end(), meshIndexBuffer);

		objFile.close();

		Mesh mesh;

		mesh.vertices = meshVertexBuffer;
		mesh.normals = meshNormalBuffer;

		mesh.indices = meshIndexBuffer;

		mesh.vertexCount = 3 * faceCount;
		mesh.indexCount = 3 * faceCount;

		return mesh;
	}
	else {
		throw std::runtime_error("Reading OBJ file failed. This is usually because the operating system can't find it. Check if the relative path (to your terminal's working directory) is correct.");
	}
}