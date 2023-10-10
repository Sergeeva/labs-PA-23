#include <iostream>
#include <vector>
#include <thread>
#include <random>
#include <fstream>
#include <string>
#include <chrono>
//#define printToConsole
//const char* name = "C:/Users/liza/Desktop/PA_Lab1_Kostebelova_0303_P_Threads/result.txt";
const char* name = "result.txt";
const int MATRIX_SIZE = 1000;
const int THREADS_NUM = 1;

void printMatrix(std::vector<std::vector<int>>& matrix, int row, int col) {
    for (int i=0; i<row; i++) {
        for (int j=0; j<col; j++) {
            std::cout << matrix.at(i).at(j) << " ";
        }
        std::cout << std::endl;
    }
}

void generateMatrices(std::vector<std::vector<int>>& matrix1, std::vector<std::vector<int>>& matrix2)
{
    //fill vecM1
    std::vector<int> vec_for_matrix;
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            vec_for_matrix.push_back(1 + rand() % 10);
        }
        matrix1.push_back(vec_for_matrix);
        vec_for_matrix.clear();
    }
    //fill copy vecM2
    for (int i=0; i<MATRIX_SIZE; i++) {
        for (int j=0; j<MATRIX_SIZE; j++) {
            vec_for_matrix.push_back(1 + rand() % 10);
        }
        matrix2.push_back(vec_for_matrix);
        vec_for_matrix.clear();
    }
#ifdef printToConsole
    std::cout << "First Matrix" << std::endl;
    printMatrix(matrix1, MATRIX_SIZE, MATRIX_SIZE);
    std::cout << "Second Matrix" << std::endl;
    printMatrix(matrix2, MATRIX_SIZE, MATRIX_SIZE);
#endif

}


// Функция, которую будут выполнять потоки для вычисления умножения элементов матрицы
void multiplyMatrices(const std::vector<std::vector<int>>& matrix1, const std::vector<std::vector<int>>& matrix2,
                      std::vector<std::vector<int>>& result, int start, int end) {
    for (int i = start; i < end; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            int sum = 0;
            for (int k = 0; k < MATRIX_SIZE; ++k) {
                sum += matrix1[i][k] * matrix2[k][j];
            }
            result[i][j] = sum;
        }
    }
}

void writeToFile(std::vector<std::vector<int>>& matrix) {
  std::ofstream file(name);

  if (file.is_open()) {
    for (const auto& row : matrix) {
      for (const auto& element : row) {
        file << element << " ";
      }
      file << std::endl;
    }

    file.close();
    std::cout << "Done recording" << std::endl;
  } else {
    std::cout << "Error while recording" << std::endl;
  }
}

int main() {

    // Измерение времени начала выполнения программы
    auto start_time = std::chrono::high_resolution_clock::now();


    std::vector<std::vector<int>> matrix1;
    std::vector<std::vector<int>> matrix2;
    std::vector<std::vector<int>> result(MATRIX_SIZE, std::vector<int>(MATRIX_SIZE, 0));
    std::thread thread_fill_matrix(generateMatrices, std::ref(matrix1), std::ref(matrix2));

    thread_fill_matrix.join();

    std::vector<std::thread> threads;

    int step = MATRIX_SIZE / THREADS_NUM;
    int start = 0;
    int end = step;

    // Создаем и запускаем потоки для вычислений
    for (int i = 0; i < THREADS_NUM; ++i) {
        if (i == THREADS_NUM - 1) {
            // Последний поток берет оставшуюся часть
            end = MATRIX_SIZE;
        }
        threads.emplace_back(multiplyMatrices, std::ref(matrix1), std::ref(matrix2), std::ref(result), start, end);
        start = end;
        end += step;
    }

    // Дожидаемся окончания работы всех потоков
    for (auto& thread : threads) {
        thread.join();
    }

    // Выводим результат
#ifdef printToConsole
    std::cout << "Result Matrix" << std::endl;
    for (int i = 0; i < MATRIX_SIZE; ++i) {
        for (int j = 0; j < MATRIX_SIZE; ++j) {
            std::cout << result[i][j] << " ";
        }
        std::cout << std::endl;
    }
#endif

    writeToFile(result);

    // Измерение времени окончания выполнения программы
    auto end_time = std::chrono::high_resolution_clock::now();

    // Вычисление общего времени выполнения программы в миллисекундах
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end_time - start_time).count();

    std::cout << "Program duration " << duration << " ms" << std::endl;

    return 0;
}
