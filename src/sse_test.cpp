#include "sse_test.hpp"
#include <stdlib.h>
#include <time.h>

float randFloat() {
    return (float) (rand()) / (float) (RAND_MAX);
}

float4 randFloat4() {
    float4 res;
    res.x = randFloat();
    res.y = randFloat();
    res.z = randFloat();
    res.w = randFloat();
    return res;
}

void sse_test(Mesh &mesh) {
    //Not allowed to change:
    unsigned int const loadFactor = 1000;
    //
    std::vector<float4> vertices;
    std::vector<float4> rand1;
    std::vector<float4> rand2;
    std::vector<float4> rand3;
    std::vector<float4> rand4;
    srand(time(NULL));

    vertices.resize(mesh.vertexCount);

    std::cout << "SSE_TEST: Initializing vectors... " << std::flush;
    for (unsigned int i=0; i < vertices.size(); i++) {
        vertices[i] = mesh.vertices[i];
        rand1.push_back(randFloat4());
        rand2.push_back(randFloat4());
        rand3.push_back(randFloat4());
        rand4.push_back(randFloat4());
    }
    std::cout << "finished!"  << std::endl;
    for (unsigned int j = 0; j < loadFactor; j++) {
        std::cout << "SSE_TEST: " << (j+1) << "/" << loadFactor << " Crunching numbers on " << vertices.size() << " vertices... " << "\r" << std::flush;
        for (unsigned int i=0; i < vertices.size(); i++) {
            vertices[i] = vertices[i] + rand1[i];
            vertices[i] = vertices[i] - rand2[i];
            vertices[i] = vertices[i] * rand3[i];
            if (rand4[i] != 0) {
                vertices[i] = vertices[i] / rand4[i];
            }
        }
    }
    std::cout << std::endl;
}