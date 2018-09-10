#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <sstream>
#include <limits>

#include "geom.hpp"

typedef struct Mesh {
	float4* vertices;
	float3* normals;

	unsigned int* indices;

	size_t vertexCount;
	size_t indexCount;

	Mesh() {
		vertices = nullptr;
		normals = nullptr;
		indices = nullptr;
		vertexCount = 0;
		indexCount = 0;
	}

	void deleteMesh() {
		delete[] vertices;
		delete[] normals;
		delete[] indices;
	}
} Mesh;

Mesh loadOBJ(std::string src);