# SiliconSpire QAP Solver (C++)

A C++17 command-line heuristic for the Quadratic Assignment Problem (QAP). It combines Grey Wolf Optimization (GWO), a largest-value-priority permutation decoder, and swap-based Tabu Search.

This repository contains a small illustrative instance. The cleanroom names in the example describe the toy data; this is not a deployed semiconductor-layout system, and no industrial savings or performance benchmark is claimed.

## Build and run

```sh
g++ -std=c++17 -O2 -o qap_solver qap_solver.cpp
./qap_solver --input-file silicon_spire.txt
./qap_solver --input-file silicon_spire.txt --seed 42
./qap_solver --help
```

An instance file contains a positive integer `n`, then an `n × n` distance matrix, followed by an `n × n` flow matrix. The solver reports an error for invalid dimensions, missing or malformed matrix values, or unexpected trailing data.

Assignments must be permutations of `0..n-1`. The objective is accumulated in signed 64-bit arithmetic; invalid assignments and costs outside the supported range are reported as errors.

The objective for an assignment `p` is:

```text
sum(i = 0..n-1) sum(j = 0..n-1) flow[i][j] * distance[p[i]][p[j]]
```

## Search parameters

| Option | Default | Meaning |
| --- | ---: | --- |
| `--input-file FILE` | `silicon_spire.txt` | QAP instance path |
| `--pack-size N` | `30` | GWO population; at least three candidates are required |
| `--max-iterations N` | `100` | GWO iterations; must be positive |
| `--ts-iterations N` | `50` | Tabu iterations per invocation; zero disables Tabu Search |
| `--tabu-tenure N` | `10` | Tabu-list length |
| `--ts-every N` | `1` | Run Tabu Search on the current best every N GWO iterations |
| `--jitter VALUE` | `0.0` | Uniform perturbation before permutation decoding |
| `--seed N` | generated | Unsigned 64-bit random seed; the selected seed is printed for repeat runs |

GWO work is approximately `O(iterations × pack_size × n)`. Each Tabu iteration considers `O(n²)` swaps and recomputes each candidate objective in `O(n²)`, giving `O(Tabu iterations × n⁴)` per Tabu invocation. Runtime depends on the chosen parameters and instance.

## Small QAPLIB evaluation

The standard-library Python runner downloads three small instances from [QAPLIB](https://coral.ise.lehigh.edu/data-sets/qaplib/qaplib-problem-instances-and-solutions/) and verifies each file's SHA-256 before use. It runs seeds 1–30 with pack size 30, 100 GWO iterations, 50 Tabu iterations and tenure 10:

```sh
python3 benchmarks/run_qaplib.py --solver ./qap_solver
```

Observed with this C++ build:

| Instance | Published optimum | Exact runs | Best | Median | Mean gap |
| --- | ---: | ---: | ---: | ---: | ---: |
| Nug12 | 578 | 15/30 | 578 | 582 | 0.796% |
| Nug14 | 1,014 | 1/30 | 1,014 | 1,034 | 1.900% |
| Tai12a | 224,416 | 18/30 | 224,416 | 224,416 | 1.530% |

The per-seed CSV can be saved with `--csv results.csv`. These are small-instance checks, not evidence of performance on larger or industrial QAPs. The C++ and Rust programs use different random-number implementations, so equal seed values do not make their runs a paired algorithm comparison. Re-run the script for results from a different build or platform.

## Scope and limitations

- GWO and Tabu Search are randomized heuristics. They do not guarantee an optimal assignment.
- The best assignment is retained across generations. After Tabu Search changes it, the solver re-encodes its continuous position before the next GWO iteration.
- Pass `--seed N` to repeat a run with the same build and input on the same platform. Without it, the solver generates and prints a seed that can be reused.
- The bundled 4 × 4 instance is suitable for a small demonstration, not for drawing conclusions about larger QAPs.
- No industrial data or comparison against exact solvers on larger instances is included.

## Attribution

This is an educational optimization project. The included instance and cleanroom narrative are illustrative. Contributions and prior work remain attributed in the repository history.
