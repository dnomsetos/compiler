# Compiler

A compiler implementation for a Rust-like language, built in C++. The language grammar is defined in [`grammar.txt`](grammar.txt).

## Pipeline

```
Source Code → Scanner → Parser → Semantic Analysis → Borrow Checker → IR Builder → Object Emission
```

Each stage can be invoked independently via CLI flags, making it easy to inspect intermediate representations.

## Building

From the project root:

```bash
# Debug build (for development and testing)
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_BUILD_TYPE=Debug
cmake --build build

# Release build
cmake -S . -B build -DCMAKE_CXX_COMPILER=g++-13 -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

## Usage

```bash
./build/compiler <mode> <file>
```

Run `./build/compiler --help` (without a file) to see all available options.

### Modes

| Flag                | Description                                              |
| ------------------- | -------------------------------------------------------- |
| `--print_ast`       | Parse the file and print the AST                         |
| `--semantic`        | Run semantic analysis (type checking, symbol resolution) |
| `--build_ir`        | Build LLVM IR                                            |
| `--build_custom_ir` | Build IR representation for borrow checker               |
| `--borrow_check`    | Run the borrow checker                                   |
| `--emit_object`     | Emit a native object file                                |

### Additional flags

| Flag | Description |
|------|-------------|
| `--skip_empty` | Skip empty nodes when printing the AST |
| `--file <path>` | Write output to a file instead of stdout |

## Running Tests

```bash
cmake --build build --target run_tests
```

Test cases live in `tests/test_files/`. Each subdirectory contains a `.rs` source file along with its expected AST, IR, and result output.

## Project Structure

```
compiler/
├── src/
│   ├── scanner/            # Tokenizer / lexer
│   ├── parser/             # Recursive descent parser, AST
│   ├── semantic_analysis/  # Type system, symbol table, semantic visitor
│   ├── ir_builder/         # LLVM IR and custom IR generation
│   ├── borrow_check/       # Borrow checker (CFG, places state)
│   ├── testing_utilities/  # AST printer, type checker helpers
│   └── main.cpp            # CLI entry point
├── include/                # Public headers (mirrors src/ layout)
├── tests/                  # Test runner and test cases
├── grammar.txt             # Formal language grammar
└── allocators.md           # Memory allocator benchmark results
```

## Language Features

The supported grammar includes:

- **Functions** — `fn name(arg: type) { ... }`
- **Variables** — `let` / `static`, with optional `mut` and type annotation
- **Expressions** — arithmetic, bitwise, logical, comparison, casting (`as`)
- **Control flow** — `if` / `else if` / `else`, `loop`, labeled `break` / `continue`
- **Block expressions** — blocks that return a value
- **References** — `&T` and `&mut T`
- **Primitive types** — `i8`–`i64`, `u8`–`u64`, `f32`, `f64`, `bool`, `char`

## Memory Allocator Notes

See [`allocators.md`](allocators.md) for a Valgrind-based comparison of three C++ allocator strategies (`default`, `unsynchronized_pool_resource`, `monotonic_buffer_resource`). Overall cache miss rates are similar across all three; differences are marginal and workload-dependent.

