#include <condition_variable>
#include <mutex>
#include <queue>
#include <chrono>
#include "matrix.h"
#define MATRIX_SIZE 50
#define OPERATION_DURATION 3000000
#define CURRENT_TIME chrono::steady_clock::now()

// Шаблон для потокобезопасной очереди
template<typename T>
class ThinBlockQueue {
    struct QueueNode {
        shared_ptr<T> data;
        unique_ptr<QueueNode> next;
    };

    mutex head_mutex;
    unique_ptr<QueueNode> head;
    mutex tail_mutex;
    QueueNode* tail;
    condition_variable data_cond;

    QueueNode* getTail() {
        lock_guard<mutex> tail_lock(tail_mutex);
        return tail;
    }

    unique_lock<mutex> waitForData() {
        unique_lock<mutex> head_lock(head_mutex);
        data_cond.wait(head_lock, [&]{ return head.get() != getTail(); });
        return move(head_lock);
    }

    unique_ptr<QueueNode> popHead(T& value) {
        unique_lock<mutex> head_lock(waitForData());
        num_popped++;
        value = move(*head->data);
        unique_ptr<QueueNode> old_head = move(head);
        head = move(old_head->next);
        return old_head;
    }

public:
    unsigned int num_inserted, num_popped;

    ThinBlockQueue() : head(new QueueNode()), tail(head.get()), num_inserted(0), num_popped(0) {}

    ThinBlockQueue(const ThinBlockQueue& other) = delete;
    ThinBlockQueue& operator=(const ThinBlockQueue& other) = delete;

    void push(T newValue) {
        shared_ptr<T> newData(make_shared<T>(move(newValue)));
        unique_ptr<QueueNode> newNode(new QueueNode());
        {
            lock_guard<mutex> tail_lock(tail_mutex);
            tail->data = newData;
            QueueNode* const newTail = newNode.get();
            tail->next = move(newNode);
            tail = newTail;
            num_inserted++;
        }
        data_cond.notify_one();
    }

    void waitAndPop(T& value) {
        unique_ptr<QueueNode> const old_head = popHead(value);
    }

    bool isEmpty() {
        lock_guard<mutex> head_lock(head_mutex);
        return (head.get() == getTail());
    }
};

ThinBlockQueue<pair<Matrix, Matrix>> multiplicationQueue;
ThinBlockQueue<Matrix> printQueue;

unsigned long getTimeDifference(chrono::time_point<chrono::steady_clock> start, chrono::time_point<chrono::steady_clock> end) {
    return chrono::duration_cast<chrono::microseconds>(end - start).count();
}

void processMultiplicationQueue(chrono::time_point<chrono::steady_clock>& startTime) {
    pair<Matrix, Matrix> matricesPair(Matrix(0, 0), Matrix(0, 0));
    while (getTimeDifference(startTime, CURRENT_TIME) < OPERATION_DURATION) {
        multiplicationQueue.waitAndPop(matricesPair);
        printQueue.push(matricesPair.first * matricesPair.second);  
    }    
}

void startMatrixConsumers(int numConsumers, chrono::time_point<chrono::steady_clock>& startTime) {
    vector<thread> consumers;
    for (int i = 0; i < numConsumers; i++) {
        consumers.emplace_back(processMultiplicationQueue, ref(startTime));
    }
    for (auto &t : consumers) {
        t.join();
    }    
}

void fillMultiplicationQueue(chrono::time_point<chrono::steady_clock>& startTime) {
    while (getTimeDifference(startTime, CURRENT_TIME) < OPERATION_DURATION) {
        multiplicationQueue.push(make_pair(Matrix(MATRIX_SIZE, MATRIX_SIZE), Matrix(MATRIX_SIZE, MATRIX_SIZE)));
    }
}

void startMatrixProducers(int numProducers, chrono::time_point<chrono::steady_clock>& startTime) {
    vector<thread> producers;
    for (int i = 0; i < numProducers; i++) { 
        producers.emplace_back(fillMultiplicationQueue, ref(startTime));
    }
    for (auto &t : producers) { t.join(); }
}

void outputResults() {
    ofstream resultFile;
    resultFile.open("result_output.txt");
    Matrix resultMatrix(0, 0);
    while (!printQueue.isEmpty()) {
        printQueue.waitAndPop(resultMatrix);
        resultFile << resultMatrix;
    }
    resultFile.close();
}

int main() {
    int numProducers, numConsumers;
    cout << "Matrix Multiplication Simulation\n";
    cout << "Enter number of matrix producers and consumers: ";
    cin >> numProducers >> numConsumers;

    auto startTime = CURRENT_TIME; 

    thread producerThread(startMatrixProducers, numProducers, ref(startTime));
    thread consumerThread(startMatrixConsumers, numConsumers, ref(startTime));

    producerThread.join();
    consumerThread.join();
    thread resultThread(outputResults);

    cout << "Matrix pairs produced: " << multiplicationQueue.num_inserted << '\n';
    cout << "Matrix pairs consumed: " << multiplicationQueue.num_popped << '\n';

    resultThread.join();
    return 0;
}