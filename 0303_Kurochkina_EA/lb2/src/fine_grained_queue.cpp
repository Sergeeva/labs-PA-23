#include<utility>
#include<condition_variable>
#include<mutex>
#include<queue>
#include<chrono>
#include "matrix.h"
#define FILENAME "C:/Users/Katrin/Qt/pa_lab2/results2.txt"
#define WORK_TIME_MCS 3000000
#define MATRIX_DIM 50
#define CURRENT_TIME chrono::steady_clock::now()


template<typename T>
class FGQueue {
    struct Node {
        shared_ptr<T> data;
        unique_ptr<Node> next;
    };
    mutex head_mutex;
    unique_ptr<Node> head;
    mutex tail_mutex;
    Node* tail;
    condition_variable data_cond;

    Node* getTail() {
        lock_guard<mutex> tail_lock(tail_mutex);
        return tail;
    }

    unique_lock<mutex> waitData() {
        unique_lock<mutex> head_lock(head_mutex);
        data_cond.wait(head_lock,[&]{return head.get()!=getTail();});
        return move(head_lock);
    }

    unique_ptr<Node> popHead(T& value) {
        unique_lock<mutex> head_lock(waitData());
        popped++;
        value = move(*head->data);
        std::unique_ptr<Node> old_head = move(head);
        head = move(old_head->next);
        return old_head;
    }

public:
    unsigned int inserted, popped;
    FGQueue() : head(new Node()), tail(head.get()), inserted(0), popped(0) {}

    FGQueue(const FGQueue& other) = delete;
    FGQueue& operator= (const FGQueue& other) = delete;

    void push(T new_value) {
        shared_ptr<T> new_data(make_shared<T>(move(new_value)));
        unique_ptr<Node> p(new Node());
        {
            lock_guard<mutex> tail_lock(tail_mutex);
            tail->data = new_data;
            Node* const new_tail = p.get();
            tail->next = move(p);
            tail = new_tail;
            inserted++;
        }
        data_cond.notify_one();
    }

    void waitPopHead(T& value) {
        unique_ptr<Node> const old_head = popHead(value);
    }

    bool empty() {
        lock_guard<mutex> head_lock(head_mutex);
        return (head.get() == getTail());
    }

};


unsigned long get_time_diff(chrono::time_point<std::chrono::steady_clock> first, chrono::time_point<std::chrono::steady_clock> second) {
    return chrono::duration_cast<chrono::microseconds>(second - first).count();
}

FGQueue<pair<Matrix, Matrix>> matrices_to_mult;
FGQueue<Matrix> matrices_to_print;

void multMatrices(chrono::time_point<std::chrono::steady_clock>& start_time) {
    pair<Matrix, Matrix> tmp(Matrix(0, 0), Matrix(0, 0));
    while (get_time_diff(start_time, CURRENT_TIME) < WORK_TIME_MCS) {
        matrices_to_mult.waitPopHead(tmp);
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
    while (get_time_diff(start_time, CURRENT_TIME) < WORK_TIME_MCS) {
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
        matrices_to_print.waitPopHead(tmp);
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
