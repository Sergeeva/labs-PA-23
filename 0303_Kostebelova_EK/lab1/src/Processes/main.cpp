#include <iostream>
#include <random>
#include <string>
#include <sys/wait.h>
#include <unistd.h>
#include <fstream>

const int MATRIX_SIZE = 4;

// Функция для генерации случайных входных матриц
void generateMatrices(const char* filename_matrix1, const char* filename_matrix2) {
    std::ofstream file1(filename_matrix1);
    if (!file1.is_open()) {
        std::cerr << "Error opening file " << filename_matrix1 << std::endl;
        return;
    }

    // Генерация случайных значений для матрицы A
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            int valueA = rand() % 10;    // Значение матрицы A от 0 до 9
            file1 << valueA <<  " ";
        }
        file1 << std::endl;
    }

    file1.close();

    std::ofstream file2(filename_matrix2);
    if (!file2.is_open()) {
        std::cerr << "Error opening file " << filename_matrix2 << std::endl;
        return;
    }

    // Генерация случайных значений для матрицы B
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            int valueB = rand() % 10;    // Значение матрицы B от 0 до 9
            file2 << valueB << " ";
        }
        file2 << std::endl;
    }

    file2.close();
    
    std::cout << "Input matrices are generated and written to a file" << std::endl;
}

// Функция для умножения матриц
void multiplyMatrices(const char* filename_matrix1, const char* filename_matrix2, const char* filename_result) {

    int matrixA[MATRIX_SIZE][MATRIX_SIZE];
    int matrixB[MATRIX_SIZE][MATRIX_SIZE];
    
    std::ifstream inputFile_matrix1(filename_matrix1, std::ios::in);
    if (!inputFile_matrix1.is_open()) {
        std::cerr << "Error opening file " << filename_matrix1 << std::endl;
        return;
    }
    std::ifstream inputFile_matrix2(filename_matrix2, std::ios::in);
    if (!inputFile_matrix2.is_open()) {
        std::cerr << "Error opening file " << filename_matrix2 << std::endl;
        return;
    }


    // Загрузка данных из файлов в матрицы A и B
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            inputFile_matrix1 >> matrixA[i][j];
            inputFile_matrix2 >> matrixB[i][j];
        }
    }

    inputFile_matrix1.close();
    inputFile_matrix2.close();

    int matrixC[MATRIX_SIZE][MATRIX_SIZE];

    // Умножение матриц A и B
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            matrixC[i][j] = 0;
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                matrixC[i][j] += matrixA[i][k] * matrixB[k][j];
            }
        }
    }

    std::ofstream outputFile(filename_result);
    if (!outputFile.is_open()) {
        std::cerr << "Error opening file " << filename_result << std::endl;
        return;
    }

    // Запись результирующей матрицы C в файл
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            outputFile << matrixC[i][j] << " ";
        }
        outputFile << std::endl;
    }

    outputFile.close();
    std::cout << "Matrices multiplied and written to file" << std::endl;
}

void writeToConsole(const char* file) {
    std::ifstream inputFile(file);
    if (!inputFile.is_open()) {
        std::cerr << "Error opening file " << file << std::endl;
    }
    else {
        std::string line;
    while (std::getline(inputFile, line)) {
        std::cout << line << std::endl;
    }

    inputFile.close();
    }
}

int main() {
    const char* filename_matrix1 = "matrix_A.txt";
    const char* filename_matrix2 = "matrix_B.txt";
    const char* filename_result = "result_matrix.txt";

    pid_t gen_pid = fork();
    switch(gen_pid){
        case 0:
            generateMatrices(filename_matrix1, filename_matrix2);
            exit(0);
        default:
            wait(&gen_pid); 
    }
    
    pid_t mult_pid = fork();
    switch(mult_pid){
        case 0:
            multiplyMatrices(filename_matrix1, filename_matrix2, filename_result);
            exit(0);
        default:
            wait(&mult_pid); 
    }

    pid_t write_pid = fork();
    switch(write_pid){
        case 0:
            writeToConsole(filename_result);
            exit(0);
        default:
            wait(&write_pid); 
    }

    return 0;
}