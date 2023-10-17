#include <iostream>
#include <queue>
#include <thread>
#include <mutex>
#include <condition_variable>

#include "../io/output.h"
#include "../io/input.h"
#include "../model/matrix.h"

// #define DEBUG

std::queue<Matrix> buffer;
std::mutex mutexProducer;
std::mutex mutexConsumer;

void producer(int countIterations) {
    for (int i = 1; i <= countIterations; i++) {
        std::lock_guard<std::mutex> lock(mutexProducer);
        #ifdef DEBUG
            std::cout << "Producer start, ";
        #endif
        buffer.push(generateMatrix());
        #ifdef DEBUG
            std::cout << "Producer leave" << std::endl;
        #endif
    }
}

void consumer(int countIterations) {
    for (int i = 1; i <= countIterations; i++) {
        std::lock_guard<std::mutex> lock(mutexConsumer);
        #ifdef DEBUG
            std::cout << "Consumer start, ";
        #endif
        while(buffer.empty()) {
            continue;
        }
        Matrix data = buffer.front();
        buffer.pop();
        writeMatrixToFile(squareMatrix(data));
        #ifdef DEBUG
            std::cout << "Consumer leave" << std::endl;
        #endif
    }
}

int main() {
    std::vector<int> inputValues = readInputFile();
    if(inputValues.size() != 3) {
        return -1;
    }

    openOutputFile();

    int countProducers = inputValues[0],
        countConsumers = inputValues[1], 
        countIterationsPerProducer = inputValues[2];
    int countIterationPerConsumer = countIterationsPerProducer*countProducers/countConsumers;

    std::vector<std::thread> threads;

    for(int i = 0; i < countConsumers; i++) {
        threads.push_back(std::thread(consumer, countIterationPerConsumer));
    }
    for(int i = 0; i < countProducers; i++) {
        threads.push_back(std::thread(producer, countIterationsPerProducer));
    }

    for(int i = 0; i < countProducers+countConsumers; i++) {
        threads[i].join();
    }

    closeOutputFile();
    return 0;
}
