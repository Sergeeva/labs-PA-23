__kernel void multiply(
    __global int* left,
    __global int* right,
    int size,
    __global int* result
) {
    int col = get_global_id(0);
    int row = get_global_id(1);
    int sum = 0;
    for (int i = 0; i < size; i++) {
        sum += (left[row * size + i] * right[i * size + col]);
    }
    result[row * size + col] = sum;
}