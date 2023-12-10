#include "matrix.h"


//constructor
Matrix::Matrix(int r, int c, bool fill_with_vals) : rows(r), cols(c), matrix(r * c) {
    if (fill_with_vals) {
        for (int i = 0; i < rows; i++) {
            for (int j = 0; j < cols; j++) {
                matrix[i * cols + j] = rand() % 10 - 5;
            }
        }
    }
}

Matrix::Matrix(const Matrix& other) {
    this->rows = other.rows;
    this->cols = other.cols;
    this->matrix = other.matrix;
}

Matrix::Matrix(const Matrix&& other) {
    this->rows = other.rows;
    this->cols = other.cols;
    this->matrix = move(other.matrix);
}

Matrix Matrix::operator* (const Matrix& other) const {

    Matrix result(this->rows, other.cols, false);
    if (this->cols == other.rows) {
        int tmp;
        for (int i = 0; i < this->rows; i++) {
            for (int j = 0; j < other.cols; j++) {
                tmp = 0;
                for (int k = 0; k < other.rows; k++) {
                    tmp += (this->matrix[i * this->cols + k] * other.matrix[k * other.cols + j]);
                }
                result.matrix[i * other.cols + j] = tmp;
            }
        }
    }
    else {
        cout << "wrong dimensions for multiplications\n";
    }
    return result;
}

void Matrix::operator= (const Matrix&& other) noexcept {
    this->rows = other.rows;
    this->cols = other.cols;
    this->matrix = move(other.matrix);
}


//int* Matrix::to_array(bool row_first) {
//    cl_int* res = new cl_int[cl_int(this->rows) * this->cols];
//    if (row_first) {
//        for (int r = 0; r < this->rows; r++) {
//            for (int c = 0; c < this->cols; c++) {
//                res[r * this->cols + c] = this->matrix[r][c];
//            }
//        }
//    }
//    else {
//        for (int r = 0; r < this->rows; r++) {
//            for (int c = 0; c < this->cols; c++) {
//                res[c * this->rows + r] = this->matrix[r][c];
//            }
//        }
//    }
//
//    return res;
//}


std::ostream& operator<< (std::ostream& os, const Matrix& m) {
    os << m.rows << ' ' << m.cols << '\n';
    for (int i = 0; i < m.rows; i++) {
        for (int j = 0; j < m.cols; j++) {
            os << m.matrix[i * m.cols + j];
            if (j + 1 < m.cols) os << ' ';
        }
        os << '\n';
    }
    return os;
}

