#include <iostream>
#include <cstring>
#include "utilities/OBJLoader.hpp"
#include "rasteriser.hpp"
#include "sse_test.hpp"

int main(int argc, char **argv) {
	std::string input("../input/sphere.obj");
	std::string output("../output/sphere.png");
	unsigned int width = 1920;
	unsigned int height = 1080;
	bool sse = false;


	for (int i = 1; i < argc; i++) {
		if (i < argc -1) {
			if (std::strcmp("-i", argv[i]) == 0) {
				input = argv[i+1];
			} else if (std::strcmp("-o", argv[i]) == 0) {
				output = argv[i+1];
			} else if (std::strcmp("-w", argv[i]) == 0) {
				width = (unsigned int) std::stoul(argv[i+1]);
			} else if (std::strcmp("-h", argv[i]) == 0) {
				height = (unsigned int) std::stoul(argv[i+1]);
			}
		}
		if (std::strcmp("--sse", argv[i]) == 0) {
			sse = true;
		}
	}

	std::cout << "Loading '" << input << "' file... " ;
	Mesh mesh = loadOBJ(input);
	std::cout << "complete!" << std::endl;
	if (sse) {
		std::cout << "Running SSE test..." << std::endl;
		sse_test(mesh);
		std::cout << "SSE test finished!" << std::endl;
	} else {
		rasterise(mesh, output, width, height);
	}

	return 0;
}
