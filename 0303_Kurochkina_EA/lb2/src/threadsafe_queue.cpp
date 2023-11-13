#include<utility>
#include<condition_variable>
#include<mutex>
#include<queue>
#include<chrono>
#include "matrix.h"
#define FILENAME "C:/Users/Katrin/Qt/pa_lab2/results1.txt"
#define MATRIX_DIM 50
#define WORK_TIME_MCS 3000000
#define CURRENT_TIME chrono::steady_clock::now()


template<typename T>
class Queue {
private:
    mutable mutex mut;
    queue<T> my_queue;
    condition_variable cv;
public:
    unsigned int inserted, popped;
    Queue() : inserted(0), popped(0) {}

    void push(T new_value) {
        lock_guard<mutex> lk(mut);
        my_queue.push(move(new_value));
        inserted++;
        cv.notify_one();
    }
    void waitPop(T& value) {
        unique_lock<mutex> lk(mut);
        cv.wait(lk, [this] {return !my_queue.empty();});
        value = move(my_queue.front());
        popped++;
        my_queue.pop();
    }

    bool empty() const {
        lock_guard<mutex> lk(mut);
        return my_queue.empty();
    }

    int size() {return my_queue.size();}
};


Queue<pair<Matrix, Matrix>> matrices_to_mult;
Queue<Matrix> matrices_to_print;


unsigned long timeDiff(chrono::time_point<std::chrono::steady_clock> first, chrono::time_point<std::chrono::steady_clock> second) {
    return chrono::duration_cast<chrono::microseconds>(second - first).count();
}

void multMatrices(chrono::time_point<std::chrono::steady_clock>& start_time) {
    pair<Matrix, Matrix> tmp(Matrix(0, 0), Matrix(0, 0));
    while (timeDiff(start_time, CURRENT_TIME) < WORK_TIME_MCS) {
        matrices_to_mult.waitPop(tmp);
        matrices_to_print.push(tmp.first * tmp.second);
    }
}

void consumersMultMatrices(int n, chrono::time_point<std::chrono::steady_clock>& start_time) {
    vector<thread> cons;
    for (int i = 0; i < n; i++) {
        cons.emplace_back(multMatrices, ref(start_time));
    }
    for (auto &t: cons) {
        t.join();
    }
}

void generateMatrices(chrono::time_point<std::chrono::steady_clock>& start_time) {
    while (timeDiff(start_time, CURRENT_TIME) < WORK_TIME_MCS) {
        matrices_to_mult.push(make_pair(Matrix(MATRIX_DIM, MATRIX_DIM), Matrix(MATRIX_DIM, MATRIX_DIM)));
    }

}

void producersGenMatrices(int n, chrono::time_point<std::chrono::steady_clock>& start_time) {
    vector<thread> prods;
    for (int i = 0; i < n; i++) {
        prods.emplace_back(generateMatrices, ref(start_time));
    }
    for (auto &t: prods) { t.join(); }

}

void printResults() {
    ofstream file;
    file.open(FILENAME);
    if (!file.is_open()) {
        file.close();
        cout << "Error\n";
        return;
    }
    Matrix tmp(0, 0);
    while (!matrices_to_print.empty()) {
        matrices_to_print.waitPop(tmp);
        file << tmp;
    }
    file.close();
}

int main() {

    int producers_num, consumers_num;
    cout << "Producers num and consumers num: ";
    cin >> producers_num >> consumers_num;
    auto start = CURRENT_TIME;
    thread producer_thread(producersGenMatrices, producers_num, ref(start));
    thread consumer_thread(consumersMultMatrices, consumers_num, ref(start));
    producer_thread.join();
    consumer_thread.join();
    thread printer_t(printResults);

    cout << "generated pairs for mult: " << matrices_to_mult.inserted << '\n';
    cout << "results: " << matrices_to_mult.popped << '\n';
    printer_t.join();
    return 0;
}

