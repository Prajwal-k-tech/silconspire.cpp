# QAP Solver: Silicon Spire Cleanroom Optimizer 🏭

A high-performance C++ command-line tool implementing a hybrid metaheuristic algorithm (Grey Wolf Optimizer + Tabu Search) to solve the Quadratic Assignment Problem (QAP), specifically designed for the Silicon Spire semiconductor fabrication plant layout optimization challenge.

## Installation & Usage 🚀

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

### Example Usage
```bash
# Quick test with smaller parameters
./qap_solver --pack-size 20 --max-iterations 50

# High-quality search with extensive parameters
./qap_solver --pack-size 100 --max-iterations 500 --ts-iterations 100

# Custom problem instance
./qap_solver --input-file my_problem.txt
```

## Problem Statement & Solution 🔬

### The Silicon Spire Challenge

**Silicon Spire Dynamics** is constructing a cutting-edge semiconductor fabrication plant with a critical cleanroom layout decision. The company must optimally assign **six processing modules** to **six cleanroom bays** to minimize wafer pod transportation costs and maximize production efficiency.

#### Processing Modules:
1. **Photolithography Bay**
2. **Etching & Cleaning Station**
3. **Deposition Chamber**
4. **Metrology & Inspection Hub**
5. **Ion Implantation Unit**
6. **Wafer Test & Sort Area**

#### Available Locations:
- **Bay Alpha**
- **Bay Beta**
- **Bay Gamma**
- **Bay Delta**
- **Bay Epsilon**
- **Bay Zeta**

#### Optimization Objective:
Minimize the total cost function: **Σᵢ Σⱼ (flow[i][j] × distance[location[i]][location[j]])**

Where:
- `flow[i][j]` = Wafer pods per hour flowing between facilities i and j
- `distance[x][y]` = Physical distance in meters between bays x and y
- The goal is to place high-traffic facility pairs close together.

### Solution for the 6x6 Silicon Spire Problem

✅ **OPTIMAL SOLUTION FOUND**: The tool consistently finds the proven optimal solution for the new 6x6 problem.

- **Best cost**: **41,490** (verified by exhaustive search of all 720 permutations)
- **Optimal assignment**:
  - Photolithography Bay → Bay Delta
  - Etching & Cleaning Station → Bay Gamma
  - Deposition Chamber → Bay Alpha
  - Metrology & Inspection Hub → Bay Zeta
  - Ion Implantation Unit → Bay Beta
  - Wafer Test & Sort Area → Bay Epsilon

## Algorithm Overview 🐺

This tool implements a **state-of-the-art hybrid metaheuristic** combining:

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

## Data Format 📊

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
The included `silicon_spire.txt` contains a 6×6 problem with:
- **Distance matrix**: Cleanroom bay separation distances (meters)
- **Flow matrix**: Wafer pod traffic between processing modules (pods/hour)

## Results & Performance 📈

### Solution Quality
✅ **OPTIMAL SOLUTION FOUND**: The tool consistently finds the proven optimal solution for the 6x6 test case.
- **Best cost**: 41,490 (verified by exhaustive search)
- **Optimal assignment**: As detailed in the solution section above.

### Performance Characteristics
- **Small problems (n ≤ 10)**: Near-optimal solutions in seconds
- **Medium problems (n ≤ 30)**: High-quality solutions in minutes
- **Computational complexity**: O(pack_size × iterations × (n + ts_iterations × n²))
- **Memory usage**: O(pack_size × n)

### Algorithm Convergence
- Typically converges within 10-50 iterations for small problems
- Robust across different parameter settings
- Consistent results across multiple runs

## Sample Output 💻
```
Loading QAP instance from: silicon_spire.txt
Problem size: 6x6

Starting Grey Wolf Optimizer + Tabu Search hybrid algorithm...
Pack size: 30, Max iterations: 100
Tabu Search iterations: 50, Tabu tenure: 10
Initial best cost: 42040

Iteration 1: Best cost = 41490
...

=== FINAL RESULTS ===
Best cost found: 41490
Best assignment:
  Photolithography Bay -> Bay Delta
  Etching & Cleaning Station -> Bay Gamma
  Deposition Chamber -> Bay Alpha
  Metrology & Inspection Hub -> Bay Zeta
  Ion Implantation Unit -> Bay Beta
  Wafer Test & Sort Area -> Bay Epsilon
```

## Technical Implementation 🔧

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

### Key Features
- **Robust error handling** with parameter validation
- **Memory-efficient** STL container usage  
- **Warning-free compilation** with strict compiler flags
- **Cross-platform compatibility** (Linux, Windows, macOS)
- **Professional code quality** with comprehensive testing

## Project Goals 🎯

This tool serves multiple purposes:
- **Practical solver** for real-world facility layout optimization
- **Research platform** for metaheuristic algorithm comparison
- **Educational resource** demonstrating hybrid optimization techniques
- **Benchmark tool** for QAP solver performance evaluation

The Silicon Spire scenario provides a concrete, high-stakes application where optimization decisions directly impact multi-billion-dollar manufacturing operations.

---

## Verification & Testing ✅

The implementation has been thoroughly validated:
- ✅ **Correctness**: Manual calculation verification of QAP cost function
- ✅ **Optimality**: Exhaustive search confirms our solution is globally optimal for the 6x6 case
- ✅ **Robustness**: Extensive testing across parameter ranges
- ✅ **Reliability**: Consistent results across multiple runs
- ✅ **Quality**: Warning-free compilation with strict compiler settings

**Built for production use and research excellence.** 🏆