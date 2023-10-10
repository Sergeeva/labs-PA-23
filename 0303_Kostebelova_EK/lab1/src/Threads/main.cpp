#include <iostream>
#include <vector>
#include <thread>
#include <future>
#include <numeric>
#include <chrono>
#include <random>
#include <fstream>
#include <string>
const int MATRIX_SIZE = 10;
//const char* name = "C:/Users/liza/Desktop/PA_Lab1_Kostebelova_0303_Threads/result.txt";
const char* name = "result.txt";
//using namespace std;

void generateMatrices(std::promise<int> matrixA_promise[MATRIX_SIZE][MATRIX_SIZE], std::promise<int> matrixB_promise[MATRIX_SIZE][MATRIX_SIZE])
{
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            //random_number = firs_value + rand() % last_value;
            matrixA_promise[i][j].set_value(1 + rand() % 10);
        }
    }
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixB_promise[i][j].set_value(1 + rand() % 10);
        }
    }
}

void multiplyMatrices(int matrixA[MATRIX_SIZE][MATRIX_SIZE], int matrixB[MATRIX_SIZE][MATRIX_SIZE], std::promise<int> matrixC_promise[MATRIX_SIZE][MATRIX_SIZE])
{
    int count = 0;
    for (int i = 0; i < MATRIX_SIZE; i++) {
        for (int j = 0; j < MATRIX_SIZE; j++) {
            count = 0;
            for (int k = 0; k < MATRIX_SIZE; k++) {
                count += matrixA[i][k] * matrixB[k][j];
            }
            matrixC_promise[i][j].set_value(count);
        }
    }

}

void writeToFile(int res[MATRIX_SIZE][MATRIX_SIZE]) {
//    std::cout << "NOW" << std::endl;
    std::ofstream myfile;
    myfile.open(name);
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            myfile << res[i][j] << ' ';
        }
        myfile << std::endl;
    }
    myfile.close();
}

int main()
{
    int matrixA[MATRIX_SIZE][MATRIX_SIZE];
    int matrixB[MATRIX_SIZE][MATRIX_SIZE];
    int matrixC[MATRIX_SIZE][MATRIX_SIZE];

    std::promise<int> matrixA_promise[MATRIX_SIZE][MATRIX_SIZE];
    std::future<int> matrixA_future[MATRIX_SIZE][MATRIX_SIZE];
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixA_future[i][j] = matrixA_promise[i][j].get_future();
        }
    }

    std::promise<int> matrixB_promise[MATRIX_SIZE][MATRIX_SIZE];
    std::future<int> matrixB_future[MATRIX_SIZE][MATRIX_SIZE];
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixB_future[i][j] = matrixB_promise[i][j].get_future();
        }
    }

    std::thread work_thread(generateMatrices, std::move(matrixA_promise), std::move(matrixB_promise));

    std::cout << "First Matrix" << std::endl;
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixA[i][j] = matrixA_future[i][j].get();
            std::cout << matrixA[i][j] << ' ';
        }
        std::cout << std::endl;
    }
    std::cout << "Second Matrix" << std::endl;
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixB[i][j] = matrixB_future[i][j].get();
            std::cout << matrixB[i][j] << ' ';
        }
        std::cout << std::endl;
    }

    std::promise<int> matrixC_promise[MATRIX_SIZE][MATRIX_SIZE];
    std::future<int> matrixC_future[MATRIX_SIZE][MATRIX_SIZE];
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixC_future[i][j] = matrixC_promise[i][j].get_future();
        }
    }
    std::thread work_thread2(multiplyMatrices, matrixA, matrixB, std::move(matrixC_promise));

    std::cout << "RESULT Matrix in file result.txt" << std::endl;
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            matrixC[i][j] = matrixC_future[i][j].get();
            std::cout << matrixC[i][j] << ' ';
        }
        std::cout << std::endl;
    }


    std::thread work_thread3(writeToFile, matrixC);


    work_thread.join();
    work_thread2.join();
    work_thread3.join();
}
