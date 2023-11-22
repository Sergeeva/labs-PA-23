
__kernel void matrix_multiply(__global int* result, __global int* first_matrix, __global int* second_matrix, int size) {

    // Getting position of current work-item
    int2 pos = (int2)(get_global_id(0), get_global_id(1));

    float value = 0;

    for (int i = 0; i < size; ++i)
    {
        int first = first_matrix[pos.y * size + i];
        int second = second_matrix[i * size + pos.x];
        value += first * second;
    }

    result[pos.y * size + pos.x] = value;
}
