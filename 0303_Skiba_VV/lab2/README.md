## Installing rust toolchain is easy!
```bash
curl --proto '=https' --tlsv1.2 -sSf https://sh.rustup.rs | sh
```


## To run different versions of this program:
- Full locking:
```bash
cargo run --release --features fulllock
```

- Fine-grained locking:
```bash
cargo run --release --features partiallock
```
