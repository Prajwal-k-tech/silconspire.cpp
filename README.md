# QAP Solver: Silicon Spire Cleanroom Optimizer 

A high-performance C++ command-line tool implementing a hybrid metaheuristic algorithm (Grey Wolf Optimizer + Tabu Search) to solve the Quadratic Assignment Problem (QAP), specifically designed for the Silicon Spire semiconductor fabrication plant layout optimization challenge.

## Installation & Usage 

### Prerequisites
- C++17 compatible compiler (g++, clang++)
- Standard library support

### Quick Start
```bash
# Compile the solver
g++ -std=c++17 -O2 -o qap_solver qap_solver.cpp

# Run with default settings on Silicon Spire data
./qap_solver

# View all options
./qap_solver --help
```

### Command Line Options
```
Usage: ./qap_solver [options]

Options:
  --input-file FILE     Path to QAP instance file (default: silicon_spire.txt)
  --pack-size SIZE      Number of wolves in population (default: 30)
  --max-iterations N    Maximum GWO iterations (default: 100)
  --ts-iterations N     Tabu Search iterations per cycle (default: 50)
  --tabu-tenure N       Tabu list memory size (default: 10)
  --help, -h            Show this help message
```

Additional runtime flags (useful for experiments):

```
  --ts-every N          Apply Tabu Search every N iterations (default: 1)
  --jitter D            Add small uniform noise (±D) to wolf positions before decoding (default: 0.02)
```

### Example Usage
```bash
# Quick test with smaller parameters
./qap_solver --pack-size 20 --max-iterations 50

# High-quality search with extensive parameters
./qap_solver --pack-size 100 --max-iterations 500 --ts-iterations 100

# Custom problem instance
./qap_solver --input-file my_problem.txt
```

Advanced run examples (diagnostics / experiments):

```
# 1) Disable Tabu Search (pure GWO exploration):
./qap_solver --input-file instances/meta_massive_50.txt --pack-size 300 --max-iterations 200 --ts-iterations 0 --jitter 0.02

# 2) Delay Tabu Search so GWO has time to explore:
./qap_solver --input-file instances/meta_massive_50.txt --pack-size 300 --max-iterations 2000 --ts-iterations 200 --ts-every 1000000 --jitter 0.02

# 3) Increase jitter (more discrete diversity before decoding):
./qap_solver --input-file instances/meta_massive_50.txt --pack-size 300 --max-iterations 2000 --ts-iterations 200 --tabu-tenure 80 --ts-every 50 --jitter 0.08

# 4) Smaller population to slow early convergence:
./qap_solver --input-file instances/meta_massive_50.txt --pack-size 80 --max-iterations 2000 --ts-iterations 200 --tabu-tenure 80 --ts-every 50 --jitter 0.02
```

## Problem Statement & Solution 🔬

### The Silicon Spire Challenge

**Silicon Spire Dynamics** is designing a semiconductor fabrication cleanroom layout. The company must optimally assign **four processing modules** to **four cleanroom bays** to minimize wafer pod transportation costs and maximize production efficiency.

#### Processing Modules:
1. **Photolithography Bay**
2. **Etching & Cleaning Station**
3. **Deposition Chamber**
4. **Metrology & Inspection Hub**

#### Available Locations:
- **Bay Alpha**
- **Bay Beta**
- **Bay Gamma**
- **Bay Delta**

#### Optimization Objective:
Minimize the total cost function: **Σᵢ Σⱼ (flow[i][j] × distance[location[i]][location[j]])**

Where:
- `flow[i][j]` = Wafer pods per hour flowing between facilities i and j
- `distance[x][y]` = Physical distance in meters between bays x and y

### Default 4×4 Test Case (silicon_spire.txt)

The repository's default test case (`silicon_spire.txt`) now contains a 4×4 QAP instance with the following matrices:

Distance matrix (meters):
```
0 10 15 20
10 0 35 25
15 35 0 30
20 25 30 0
```

Flow matrix (pods/hour):
```
0 90 120 80
90 0 40 50
120 40 0 70
80 50 70 0
```

### Verified Optimal Solution for the 4×4 Case

for 4x4 case there are 24 permuations and the best one is the following 

- **Optimal cost:** **17,600**
- **Optimal permutation (facility -> bay indices):** `(0, 2, 1, 3)`
  - Photolithography Bay → Bay Alpha
  - Etching & Cleaning Station → Bay Gamma
  - Deposition Chamber → Bay Beta
  - Metrology & Inspection Hub → Bay Delta

This result was verified by an exhaustive search of all 24 permutations and is included in the README as the canonical default test-case result.

## Algorithm Overview 

The tool implements a **state-of-the-art hybrid metaheuristic** combining:

### Grey Wolf Optimizer (GWO)
- **Nature-inspired** global search algorithm simulating wolf pack hunting behavior
- Maintains population of candidate solutions guided by three leaders (Alpha, Beta, Delta)
- Uses **Largest Value Priority (LVP)** encoding for discrete permutation problems

### Tabu Search (TS) 
- **Local search intensification** with intelligent memory structures
- **2-opt neighborhood** exploration with swap-based moves
- **Tabu list** prevents cycling, **aspiration criterion** allows promising forbidden moves

### Hybridization Strategy
- **Best-of-both-worlds approach**: GWO explores globally, TS exploits locally
- After each GWO iteration, the best solution (Alpha wolf) is refined using Tabu Search
- Achieves superior performance compared to either algorithm alone

## Data Format:

QAP instance files use the following format:
```
n

d₁₁ d₁₂ ... d₁ₙ
d₂₁ d₂₂ ... d₂ₙ
...
dₙ₁ dₙ₂ ... dₙₙ

f₁₁ f₁₂ ... f₁ₙ
f₂₁ f₂₂ ... f₂ₙ
...
fₙ₁ fₙ₂ ... fₙₙ
```

Where:
- `n` = Problem size
- First matrix = **Distance matrix** (distances between locations)
- Second matrix = **Flow matrix** (traffic between facilities)

### Silicon Spire Instance
The included `silicon_spire.txt` contains the canonical 4×4 Silicon Spire test instance (distance and flow matrices shown above). The matrices model distances between four cleanroom bays and wafer-pod traffic between four processing modules.

## Results & Performance 📈

### Solution Quality (default 4×4 case)
 **OPTIMAL SOLUTION VERIFIED**: The repository's default 4×4 test case has been exhaustively verified.
- **Best cost:** 17,600 (exhaustive search over 24 permutations)
- **Optimal permutation (facility -> bay indices):** `(0, 2, 1, 3)` — documented in the solution section above.

### Performance Characteristics
- **Small problems (n ≤ 10)**: Near-optimal solutions in seconds
- **Medium problems (n ≤ 30)**: High-quality solutions in minutes
- **Computational complexity**: O(pack_size × iterations × (n + ts_iterations × n²))
- **Memory usage**: O(pack_size × n)

### Algorithm Convergence
- Typically converges within 10-50 iterations for small problems
- Robust across different parameter settings
- Consistent results across multiple runs

### Sample Output 
```
Loading QAP instance from: silicon_spire.txt
Problem size: 4x4

Starting Grey Wolf Optimizer + Tabu Search hybrid algorithm...
Pack size: 30, Max iterations: 100
Tabu Search iterations: 50, Tabu tenure: 10
Initial best cost: 17600

Iteration 1: Best cost = 17600
...

=== FINAL RESULTS ===
Best cost found: 17600
Best assignment:
  Photolithography Bay -> Bay Alpha
  Etching & Cleaning Station -> Bay Gamma
  Deposition Chamber -> Bay Beta
  Metrology & Inspection Hub -> Bay Delta
```

## Technical Implementation 

### Core Data Structures
```cpp
struct Problem {
    int n;                                    // Problem size
    std::vector<std::vector<int>> distance;   // Distance matrix
    std::vector<std::vector<int>> flow;       // Flow matrix
};

struct Wolf {
    std::vector<double> position;    // Continuous GWO positions
    std::vector<int> permutation;    // Discrete QAP solution
    int fitness;                     // Objective function value
};
```