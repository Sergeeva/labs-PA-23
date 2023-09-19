use std::{thread, io::{stdin, stdout}, sync::{mpsc::channel, Arc}, ops::Range, cmp::min};

use workers::*;

use std::{time::Instant, sync::OnceLock};

static WRITE_INPUT_METRIC: OnceLock<Instant> = OnceLock::new();
static MULTIPLY_METRIC: OnceLock<Instant> = OnceLock::new();
static WRITE_OUTPUT: OnceLock<Instant> = OnceLock::new();


fn input_worker() {
    println!("\tINPUT WORKER START");
    let (mat1, mat2) = input_matrices();

    print!("Input multiplication workers count: ");
    std::io::Write::flush(&mut stdout()).unwrap();

    let mut buf = String::new();
    stdin().read_line(&mut buf).unwrap();
    let mut workers_cnt = buf.trim().parse().expect("Invalid workers count!");

    let mat_elem_cnt = mat1.len() * mat2[0].len();
    if workers_cnt > mat_elem_cnt {
        println!("Workers count is more than matrix elements count, setting workers count to {}", mat_elem_cnt);
        workers_cnt = mat_elem_cnt;
    }

    start_measure(&WRITE_INPUT_METRIC);

    mat1.save_to_file("in1.txt");
    mat2.save_to_file("in2.txt");

    let elapsed = stop_measure(&WRITE_INPUT_METRIC);
    println!(" > Write inputs to file: Elapsed time: {} ms", elapsed.as_millis());

    thread::spawn(move || {
        multiply_worker((mat1, mat2), workers_cnt);
    })
    .join()
    .unwrap();

    println!("\tINPUT WORKER END");
}

fn multiply_worker((mat1, mat2): (Mat, Mat), worker_cnt: usize) {
    println!("\tMULTIPLY WORKER START");
    //here we create MPSC channel and deligate work to P workers

    start_measure(&MULTIPLY_METRIC);
    let res_rows = mat1.len();
    let res_cols = mat2[0].len();
    let mut res = vec![vec![0; res_cols]; res_rows];

    let mat1 = Arc::new(mat1);
    let mat2 = Arc::new(mat2);

    let mat_elem_cnt = res_rows * res_cols;

    let (tx, rx) = channel::<(Range<usize>, Box<[i32]>)>(); // workers will calculate range of matrix and
                                                   // send it back to main thread

    let chunk_size = mat_elem_cnt / worker_cnt;

    let calculate_rng = move |rng: Range<usize>, mat1: Arc<Mat>, mat2: Arc<Mat>| -> Box<[i32]> {
        // here we calculate range of matrix and return it to main thread
        let mut chunk: Vec<i32> = Vec::with_capacity(rng.len());

        for cursor in rng.clone() {
            let row = cursor / res_cols;
            let col = cursor % res_cols;

            let mut sum = 0;
            for i in 0..mat2.len() {
                sum += mat1[row][i] * mat2[i][col];
            }

            chunk.push(sum);
        }

        chunk.into_boxed_slice()
    };

    let mut cursor = 0;
    while cursor < mat_elem_cnt {
        let tx_copy = tx.clone();

        let rng = cursor..min(cursor + chunk_size, mat_elem_cnt);
        cursor += chunk_size;

        println!("Sending range {}..{}", rng.start, rng.end);

        let mat1_clone = mat1.clone();
        let mat2_clone = mat2.clone();

        std::thread::spawn(move || {
            tx_copy.send((rng.clone(), calculate_rng(rng, mat1_clone, mat2_clone))).unwrap();
        });
    }

    //drop tx to close channel
    drop(tx);

    println!("Collecting results...");

    while let Ok((rng, chunk)) = rx.recv() {
        print!(".");
        std::io::Write::flush(&mut stdout()).unwrap();
        for cursor in rng.clone() {
            let row = cursor / res_cols;
            let col = cursor % res_cols;

            res[row][col] = chunk[cursor - rng.start];
        }
    }
    println!("Finished! Launching print worker...");

    let elapsed = stop_measure(&MULTIPLY_METRIC);
    println!(" > Multiply matrices: Elapsed time: {} ms", elapsed.as_millis());


    thread::spawn(move || {
        print_worker(res);
    })
    .join()
    .unwrap();

    println!("\tMULTIPLY WORKER END");
}

fn print_worker(mat: Mat) {
    println!("\tPRINT WORKER START");
    start_measure(&WRITE_OUTPUT);

    mat.save_to_file("res.txt");

    let elapsed = stop_measure(&WRITE_OUTPUT);
    println!(" > Write output to file: Elapsed time: {} ms", elapsed.as_millis());

    println!("Result matrix saved to res.txt");
    println!("\tPRINT WORKER END");
}


fn main() {
    println!("\n\t\t*** Matrix multiplication [threads optimized version] ***");
    input_worker();
}
