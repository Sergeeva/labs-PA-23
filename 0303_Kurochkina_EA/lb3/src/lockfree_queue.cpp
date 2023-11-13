#include <iostream>
#include <atomic>
#include <memory>
#include <utility>
#include <chrono>
#include "matrix.h"

using namespace std;

#include <atomic>
#include <memory>
#include<utility>
#include<chrono>
#include "matrix.h"
#define WORK_TIME_MS 9000
#define MATRIX_DIM 50
#define FILENAME "C:/Users/Katrin/Qt/pa_lab3/results3.txt"
#define CURRENT_TIME chrono::steady_clock::now()

using namespace std;

unsigned long get_time_diff(chrono::time_point<std::chrono::steady_clock> first, chrono::time_point<std::chrono::steady_clock> second) {
    return chrono::duration_cast<chrono::milliseconds>(second - first).count();
}

chrono::time_point<std::chrono::steady_clock> start;


template<typename T>
class LockFreeQueue {
    struct Node {
        T data;
        atomic<Node*> next;
    };

    atomic<Node*> head;
    atomic<Node*> tail;
public:
    atomic<int> inserted;
    atomic<int> popped;
    LockFreeQueue()
        : head(new Node()), tail(head.load()) {}

    void push(T val) {
        Node* new_node = new Node();
        new_node->data = move(val);
        while (true) {
            Node* last = tail.load();
            Node* next = last->next.load();
            if (last == tail.load()) {
                if (next == nullptr) {
                    if (last->next.compare_exchange_weak(next, new_node)) {
                        inserted++;
                        tail.compare_exchange_weak(last, new_node);
                        return;
                    }
                } else {
                    tail.compare_exchange_weak(last, next);
                }
            }

        }
    }

    bool pop(T& val) {
        while (true) {
            Node* first = head.load();
            Node* last = tail.load();
            Node* next = first->next.load();
            if (first == last) {
                if (next == nullptr) {
                    return false;
                }
                tail.compare_exchange_weak(last, next);
            } else {
                if (head.compare_exchange_weak(first, next)) {
                    popped++;
                    val = move(first->data);
                    // delete first;
                    return true;
                }
            }
        }

    }

    ~LockFreeQueue() {
        Node* cur = head.load();
        Node* tmp;
        while (cur) {
            tmp = cur;
            cur = cur->next;
            delete tmp;
        }

    }

};



LockFreeQueue<pair<Matrix, Matrix>> matrices_to_mult;
LockFreeQueue<Matrix> matrices_to_print;


void multMatrices() {
    pair<Matrix, Matrix> tmp(Matrix(0, 0), Matrix(0, 0));
    while (get_time_diff(start, CURRENT_TIME) < WORK_TIME_MS) {
        if (matrices_to_mult.pop(tmp)) {
            matrices_to_print.push(tmp.first * tmp.second);
        }
    }
}

void consumersMultMatrices(int n) {
    vector<thread> cons;
    for (int i = 0; i < n; i++) {
        cons.emplace_back(multMatrices);
    }
    for (auto &t: cons) {
        t.join();
    }
}

void generateMatrices() {
    while (get_time_diff(start, CURRENT_TIME) < WORK_TIME_MS) {
        matrices_to_mult.push(make_pair(Matrix(MATRIX_DIM, MATRIX_DIM), Matrix(MATRIX_DIM, MATRIX_DIM)));
    }

}

void producersGenMatrices(int n) {
    vector<thread> prods;
    for (int i = 0; i < n; i++) {
        prods.emplace_back(generateMatrices);
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
    int inserted = matrices_to_print.inserted;
    while (inserted > 0) {
        matrices_to_print.pop(tmp);
        file << tmp;
        inserted--;
    }
    file.close();
}



int main() {

    int producers_num, consumers_num;
    cout << "Producers num and consumers num: ";
    cin >> producers_num >> consumers_num;
    start = CURRENT_TIME;
    thread producer_thread(producersGenMatrices, producers_num);
    thread consumer_thread(consumersMultMatrices, consumers_num);
    producer_thread.join();
    consumer_thread.join();
    thread printer_t(printResults);

    cout << "generated pairs for mult: " << matrices_to_mult.inserted << '\n';
    cout << "results: " << matrices_to_mult.popped << '\n';
    printer_t.join();
    return 0;
}
