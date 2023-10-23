## Installing rust toolchain is easy!
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```


## To run different versions of this program:
- Conditional variable version:
```bash
cargo run --release --features condvar
```

- Full locking version:
```bash
cargo run --release --features fulllock
```

- Partial locking version:
```bash
cargo run --release --features partiallock
```


