#include <iostream>
#include <fstream>
#include <chrono>

#include "engine/Open_cl_engine.h"
#include "png/svgpng.hpp"


auto main() -> int {

    const size_t size = 4096;

    Open_cl_engine engine{};

    // Getting device, creating context and command queue
    engine.init();

    // Reading .cl source code
    std::ifstream f("../matrix_multiply.cl");
    std::stringstream buf;
    buf << f.rdbuf();
    std::string source_code{buf.str()};

    // Building program and creating kernel for it
    engine.load_program(source_code, "matrix_multiply");

    auto start = std::chrono::high_resolution_clock::now();
    // Creating img argument for program and loading it into the kernel
    // Also enqueuing kernel work??
    engine.create_buffers(size);

    // Creating var for image read and creating read task in queue
    int *resultMatrix = new int[size * size];
    engine.read_buffer(resultMatrix, size);

    // Flushing queue and wait for completion
    engine.execute_and_wait();
    auto finish = std::chrono::high_resolution_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(finish - start);
    std::cout << duration.count() << '\n';

    /*for (int i = 0; i < size; ++i) {
        for (int j = 0; j < size; ++j) {
            std::cout << resultMatrix[i * size + j] << " ";
        }
        std::cout << "\n";
    }*/

    // Cleaning up (closing kernel, program, img_variable, queue and context)
    engine.clean_up();
    // Removing image byte array
    delete[] resultMatrix;
}
