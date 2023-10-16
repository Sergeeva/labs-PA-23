#include "./matrix.h"
#include <utility>
#include <chrono>
#include <algorithm>
#include <cmath>
#define PAIR_INT pair<int, int>
#define CURRENT_TIME chrono::steady_clock::now()


unsigned long get_time_diff(chrono::time_point<std::chrono::steady_clock> first, chrono::time_point<std::chrono::steady_clock> second) {
    return chrono::duration_cast<chrono::milliseconds>(second - first).count();
}


using namespace std;


bool is_power_of_2(int n) {
    return n > 0 && ((n & (n - 1)) == 0);
}

class MatrixMultiplier {
    
    void parallel_mult_helper(const Matrix& a, const Matrix& b, Matrix* res, int thread_count, int thread_idx) {
        int newRows = res->rows, newCols = res->cols;
        int i, j, tmp;
        for (int step = thread_idx; step < newRows * newCols; step += thread_count) {
            i = step / newCols;
            j = step % newCols;
            tmp = 0;
            for (int k = 0; k < newRows; k++) {
                tmp += (a.matrix[i][k] * b.matrix[k][j]);
            }
            res->matrix[i][j] = tmp;
        }
    }
    
    Matrix get_square_slice(const Matrix& m, int i, int j, int size) {
        Matrix tmp(size, size, false);
        for (int y = 0; y < size; y++) {
            for (int x = 0; x < size; x++) {
                tmp.matrix[y][x] = m.matrix[i+y][j+x];
            }
        }
        return tmp;
    }

    void write_ci(Matrix* res, const Matrix& ci, const PAIR_INT& lt, int size) {
        for(int i = 0; i < size; i++) {
            for (int j = 0; j < size; j++) {
                res->matrix[i + lt.first][j + lt.second] = ci.matrix[i][j];
            }
        }
    }

    void strassen_helper(const Matrix& a, const Matrix& b, Matrix* res, const PAIR_INT& lt, int size) {
        if(size <= 1) {
            return;
        }
        strassen_helper(a, b, res, make_pair(lt.first, lt.second), size/2); // left top
        strassen_helper(a, b, res, make_pair(lt.first, lt.second + size/2), size/2); // right top
        strassen_helper(a, b, res, make_pair(lt.first + size/2, lt.second), size/2); // left bot
        strassen_helper(a, b, res, make_pair(lt.first + size/2, lt.second + size/2), size/2); // right bot

        Matrix A11(get_square_slice(a, lt.first, lt.second, size/2)),
               A12(get_square_slice(a, lt.first, lt.second + size/2, size/2)),
               A21(get_square_slice(a, lt.first + size/2, lt.second, size/2)),
               A22(get_square_slice(a, lt.first + size/2, lt.second + size/2, size/2)),
               B11(get_square_slice(b, lt.first, lt.second, size/2)),
               B12(get_square_slice(b, lt.first, lt.second + size/2, size/2)),
               B21(get_square_slice(b, lt.first + size/2, lt.second, size/2)),
               B22(get_square_slice(b, lt.first + size/2, lt.second + size/2, size/2));
               
        Matrix D((A11 + A22)*(B11 + B22)),
              D1((A12 - A22)*(B21 + B22)),
              D2((A21 - A11)*(B11 + B12)),
              H1((A11 + A12)*B22),
              H2((A21 + A22)*B11),
              V1(A22*(B21-B11)),
              V2(A11*(B12 - B22));
        write_ci(res, move(D + D1 + V1 - H1), lt, size/2); // c11
        write_ci(res, move(V2 + H1), make_pair(lt.first, lt.second + size/2), size/2); // c12
        write_ci(res, move(V1 + H2), make_pair(lt.first + size/2, lt.second), size/2); // c21
        write_ci(res, move(D + D2 + V2 - H2), make_pair(lt.first + size/2, lt.second + size/2), size/2); // c22
    }

    void expand_matrix(Matrix& m, int dim) {
        for(int i = 0; i < m.rows; i++) {
            for(int j = m.cols; j < dim; j++) {
                m.matrix[i].push_back(0);
            }
        }
        for (int i = m.rows; i < dim; i++) {
            m.matrix.emplace_back(dim);
        }
        m.cols = m.rows = dim;
    }

    Matrix* prepare_matrices(Matrix& a, Matrix& b) {
        int max_size = max({a.rows, a.cols, b.rows, b.cols}, [](int x, int y){ return x < y;});
        int next_pow_2 = pow(2, ceil(log2(max_size)));
        expand_matrix(a, next_pow_2);
        expand_matrix(b, next_pow_2);
        return new Matrix(next_pow_2, next_pow_2, false);
    }

    public:
    MatrixMultiplier() = default;

    Matrix* no_parallel(const Matrix& a, const Matrix& b) {
        if (a.cols != b.rows) {
            return nullptr;
        }
        Matrix* result = new Matrix(a.rows, b.cols, false);
        int tmp = 0;
        for(int i = 0; i < a.rows; i++) {
            for(int j = 0; j < b.cols; j++) {
                tmp = 0;
                for (int k = 0; k < b.rows; k++) {
                    tmp += (a.matrix[i][k] * b.matrix[k][j]);
                }
                result->matrix[i][j] = tmp;
            }
        }
        return result;
    }
    
    Matrix* parallel(const Matrix& a, const Matrix& b, int thread_count) {
        if (a.cols != b.rows) {
            return nullptr;
        }
        vector<thread> my_threads;
        Matrix *result = new Matrix(a.rows, b.cols, false);

        for (int i = 0; i < thread_count; i++) {
            my_threads.emplace_back(&MatrixMultiplier::parallel_mult_helper, this, ref(a), ref(b), result, thread_count, i);
        }
        for (auto &t: my_threads) {
           t.join();
        }
        return result;
    }

    Matrix* strassen_alg(Matrix a, Matrix b) {
        if(a.cols != b.rows) { 
            cout << "Error while multiplying matrices with Strassen's algorithm: wrong dimensions\n";
            return nullptr;
        }
        int res_rows = a.rows, res_cols = b.cols;
        Matrix* result = prepare_matrices(a, b);
        strassen_helper(a, b, result, make_pair(0, 0), a.rows);
        for (int i = 0; i < res_rows; i++) {
            result->matrix[i].resize(res_cols);
        }
        result->matrix.resize(res_rows);
        result->rows = res_rows;
        result->cols = res_cols;      
        return result;
    }

};

int main() {
    std::cout << "Compare matrix multiplication algorithms.\n";
    int thread_count = 3;
    MatrixMultiplier matrix_multer;
    int tests_count = 5;
    int tests[tests_count] = {32, 256, 1024, 4096, 16384};
    int dim;
    chrono::time_point<std::chrono::steady_clock> start;
    vector<Matrix*> results;
    Matrix* cur_res;
    for (int i = 0; i < tests_count; i++) {
        cout << "Test #" << i+1 << ": matrix " << tests[i] << 'x' << tests[i] << '\n';
        dim = tests[i];
        Matrix a(dim, dim, true), b(dim, dim, true);
        
        start = CURRENT_TIME;
        cur_res = matrix_multer.strassen_alg(a, b);
        cout << "Strassen: " << get_time_diff(start, CURRENT_TIME) << "ms\n";
        results.push_back(cur_res);
        
        start = CURRENT_TIME;
        cur_res = matrix_multer.parallel(a, b, thread_count);
        cout << "Multithread: " << get_time_diff(start, CURRENT_TIME) << "ms\n";
        results.push_back(cur_res);
    }
    for(int i = 0; i < results.size(); i++) {
        delete results[i];
    }


    return 0;
}