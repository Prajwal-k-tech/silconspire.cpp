#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <sstream>
#include <algorithm>
#include <random>
#include <numeric>
#include <deque>
#include <climits>
#include <cmath>

struct Problem {
    int n;
    std::vector<std::vector<int>> distance;
    std::vector<std::vector<int>> flow;
    
    Problem(int size) : n(size) {
        distance.resize(n, std::vector<int>(n));
        flow.resize(n, std::vector<int>(n));
    }
};

struct Wolf {
    std::vector<double> position;
    std::vector<int> permutation;
    int fitness;
    
    Wolf(int size) : position(size), permutation(size), fitness(INT_MAX) {
        std::iota(permutation.begin(), permutation.end(), 0);
    }
};

// Command line arguments structure
struct Config {
    std::string input_file = "silicon_spire.txt";
    int pack_size = 30;
    int max_iterations = 100;
    int ts_iterations = 50;
    int tabu_tenure = 10;
};

// Function declarations
Problem load_problem(const std::string& filename);
int calculate_cost(const Problem& problem, const std::vector<int>& permutation);
std::vector<int> lvp_decode(const std::vector<double>& position);
void apply_tabu_search(const Problem& problem, Wolf& wolf, int ts_iterations, int tabu_tenure);
Config parse_arguments(int argc, char* argv[]);
void print_usage();

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        Config config = parse_arguments(argc, argv);
        
        // Load problem instance
        std::cout << "Loading QAP instance from: " << config.input_file << std::endl;
        Problem problem = load_problem(config.input_file);
        std::cout << "Problem size: " << problem.n << "x" << problem.n << std::endl;
        
        // Initialize random number generator
        std::random_device rd;
        std::mt19937 gen(rd());
        std::uniform_real_distribution<> dis(-1.0, 1.0);
        
        // Initialize wolf pack
        std::vector<Wolf> wolves(config.pack_size, Wolf(problem.n));
        Wolf alpha(problem.n), beta(problem.n), delta(problem.n);
        
        // Initialize wolves with random positions
        for (auto& wolf : wolves) {
            for (double& pos : wolf.position) {
                pos = dis(gen);
            }
            wolf.permutation = lvp_decode(wolf.position);
            wolf.fitness = calculate_cost(problem, wolf.permutation);
        }
        
        // Find initial alpha, beta, delta
        std::sort(wolves.begin(), wolves.end(), 
                 [](const Wolf& a, const Wolf& b) { return a.fitness < b.fitness; });
        alpha = wolves[0];
        beta = wolves[1];
        delta = wolves[2];
        
        std::cout << "\nStarting Grey Wolf Optimizer + Tabu Search hybrid algorithm..." << std::endl;
        std::cout << "Pack size: " << config.pack_size << ", Max iterations: " << config.max_iterations << std::endl;
        std::cout << "Tabu Search iterations: " << config.ts_iterations << ", Tabu tenure: " << config.tabu_tenure << std::endl;
        std::cout << "Initial best cost: " << alpha.fitness << std::endl << std::endl;
        
        // Main GWO loop
        for (int iteration = 0; iteration < config.max_iterations; iteration++) {
            double a = 2.0 - 2.0 * iteration / config.max_iterations; // Linearly decreasing from 2 to 0
            
            for (auto& wolf : wolves) {
                // Update position based on alpha, beta, delta
                for (int i = 0; i < problem.n; i++) {
                    // Alpha influence
                    double r1 = dis(gen), r2 = dis(gen);
                    double A1 = 2 * a * r1 - a;
                    double C1 = 2 * r2;
                    double D_alpha = std::abs(C1 * alpha.position[i] - wolf.position[i]);
                    double X1 = alpha.position[i] - A1 * D_alpha;
                    
                    // Beta influence
                    r1 = dis(gen); r2 = dis(gen);
                    double A2 = 2 * a * r1 - a;
                    double C2 = 2 * r2;
                    double D_beta = std::abs(C2 * beta.position[i] - wolf.position[i]);
                    double X2 = beta.position[i] - A2 * D_beta;
                    
                    // Delta influence
                    r1 = dis(gen); r2 = dis(gen);
                    double A3 = 2 * a * r1 - a;
                    double C3 = 2 * r2;
                    double D_delta = std::abs(C3 * delta.position[i] - wolf.position[i]);
                    double X3 = delta.position[i] - A3 * D_delta;
                    
                    // Update position
                    wolf.position[i] = (X1 + X2 + X3) / 3.0;
                    
                    // Clamp position to [-1, 1]
                    wolf.position[i] = std::max(-1.0, std::min(1.0, wolf.position[i]));
                }
                
                // Convert to permutation and calculate fitness
                wolf.permutation = lvp_decode(wolf.position);
                wolf.fitness = calculate_cost(problem, wolf.permutation);
            }
            
            // Sort wolves and update alpha, beta, delta
            std::sort(wolves.begin(), wolves.end(), 
                     [](const Wolf& a, const Wolf& b) { return a.fitness < b.fitness; });
            
            bool improved = false;
            if (wolves[0].fitness < alpha.fitness) {
                alpha = wolves[0];
                improved = true;
            }
            if (wolves[1].fitness < beta.fitness) {
                beta = wolves[1];
            }
            if (wolves[2].fitness < delta.fitness) {
                delta = wolves[2];
            }
            
            // Apply Tabu Search to alpha wolf (hybridization)
            apply_tabu_search(problem, alpha, config.ts_iterations, config.tabu_tenure);
            
            // Update wolves[0] with improved alpha
            wolves[0] = alpha;
            
            // Progress output
            if ((iteration + 1) % 10 == 0 || improved) {
                std::cout << "Iteration " << (iteration + 1) 
                         << ": Best cost = " << alpha.fitness << std::endl;
            }
        }
        
        // Final results
        std::cout << "\n=== FINAL RESULTS ===" << std::endl;
        std::cout << "Best cost found: " << alpha.fitness << std::endl;
        std::cout << "Best assignment:" << std::endl;
        
        // Map facility indices to names for Silicon Spire scenario
        std::vector<std::string> facilities = {
            "Photolithography Bay",
            "Etching & Cleaning Station", 
            "Deposition Chamber",
            "Metrology & Inspection Hub"
        };
        std::vector<std::string> bays = {"Alpha", "Beta", "Gamma", "Delta"};
        
        for (int i = 0; i < problem.n; i++) {
            if (i < static_cast<int>(facilities.size()) && alpha.permutation[i] < static_cast<int>(bays.size())) {
                std::cout << "  " << facilities[i] 
                         << " -> Bay " << bays[alpha.permutation[i]] << std::endl;
            } else {
                std::cout << "  Facility " << i 
                         << " -> Location " << alpha.permutation[i] << std::endl;
            }
        }
        
    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    
    return 0;
}

Problem load_problem(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        throw std::runtime_error("Cannot open file: " + filename);
    }
    
    int n;
    file >> n;
    
    Problem problem(n);
    
    // Read distance matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> problem.distance[i][j];
        }
    }
    
    // Read flow matrix
    for (int i = 0; i < n; i++) {
        for (int j = 0; j < n; j++) {
            file >> problem.flow[i][j];
        }
    }
    
    file.close();
    return problem;
}

int calculate_cost(const Problem& problem, const std::vector<int>& permutation) {
    int cost = 0;
    for (int i = 0; i < problem.n; i++) {
        for (int j = 0; j < problem.n; j++) {
            cost += problem.flow[i][j] * problem.distance[permutation[i]][permutation[j]];
        }
    }
    return cost;
}

std::vector<int> lvp_decode(const std::vector<double>& position) {
    int n = position.size();
    std::vector<std::pair<double, int>> sorted_positions;
    
    for (int i = 0; i < n; i++) {
        sorted_positions.push_back({position[i], i});
    }
    
    std::sort(sorted_positions.begin(), sorted_positions.end(), std::greater<>());
    
    std::vector<int> permutation(n);
    for (int i = 0; i < n; i++) {
        permutation[sorted_positions[i].second] = i;
    }
    
    return permutation;
}

void apply_tabu_search(const Problem& problem, Wolf& wolf, int ts_iterations, int tabu_tenure) {
    std::deque<std::pair<int, int>> tabu_list;
    std::vector<int> current_solution = wolf.permutation;
    std::vector<int> best_solution = current_solution;
    int current_cost = wolf.fitness;
    int best_cost = current_cost;
    static int global_best = INT_MAX;  // Track global best across all TS calls
    if (best_cost < global_best) {
        global_best = best_cost;
    }
    
    for (int iter = 0; iter < ts_iterations; iter++) {
        std::vector<int> best_neighbor = current_solution;
        int best_neighbor_cost = INT_MAX;
        int best_i = -1, best_j = -1;
        
        // Explore 2-opt neighborhood
        for (int i = 0; i < problem.n - 1; i++) {
            for (int j = i + 1; j < problem.n; j++) {
                // Create neighbor by swapping positions i and j
                std::vector<int> neighbor = current_solution;
                std::swap(neighbor[i], neighbor[j]);
                
                int neighbor_cost = calculate_cost(problem, neighbor);
                
                // Check if move is tabu
                bool is_tabu = false;
                for (const auto& tabu_move : tabu_list) {
                    if ((tabu_move.first == i && tabu_move.second == j) ||
                        (tabu_move.first == j && tabu_move.second == i)) {
                        is_tabu = true;
                        break;
                    }
                }
                
                // Accept move if not tabu or if it improves global best (aspiration criterion)
                if (!is_tabu || neighbor_cost < global_best) {
                    if (neighbor_cost < best_neighbor_cost) {
                        best_neighbor = neighbor;
                        best_neighbor_cost = neighbor_cost;
                        best_i = i;
                        best_j = j;
                    }
                }
            }
        }
        
        // If no improving move found, break
        if (best_i == -1) break;
        
        // Update current solution
        current_solution = best_neighbor;
        current_cost = best_neighbor_cost;
        
        // Update best solution
        if (current_cost < best_cost) {
            best_solution = current_solution;
            best_cost = current_cost;
            if (best_cost < global_best) {
                global_best = best_cost;
            }
        }
        
        // Add move to tabu list
        tabu_list.push_back({best_i, best_j});
        if (static_cast<int>(tabu_list.size()) > tabu_tenure) {
            tabu_list.pop_front();
        }
    }
    
    // Update wolf with best solution found
    wolf.permutation = best_solution;
    wolf.fitness = best_cost;
}

Config parse_arguments(int argc, char* argv[]) {
    Config config;
    
    for (int i = 1; i < argc; i++) {
        std::string arg = argv[i];
        
        if (arg == "--help" || arg == "-h") {
            print_usage();
            exit(0);
        } else if (arg == "--input-file" && i + 1 < argc) {
            config.input_file = argv[++i];
        } else if (arg == "--pack-size" && i + 1 < argc) {
            config.pack_size = std::stoi(argv[++i]);
            if (config.pack_size < 1) {
                throw std::invalid_argument("Pack size must be positive");
            }
        } else if (arg == "--max-iterations" && i + 1 < argc) {
            config.max_iterations = std::stoi(argv[++i]);
            if (config.max_iterations < 1) {
                throw std::invalid_argument("Max iterations must be positive");
            }
        } else if (arg == "--ts-iterations" && i + 1 < argc) {
            config.ts_iterations = std::stoi(argv[++i]);
            if (config.ts_iterations < 1) {
                throw std::invalid_argument("TS iterations must be positive");
            }
        } else if (arg == "--tabu-tenure" && i + 1 < argc) {
            config.tabu_tenure = std::stoi(argv[++i]);
            if (config.tabu_tenure < 1) {
                throw std::invalid_argument("Tabu tenure must be positive");
            }
        } else {
            std::cerr << "Unknown argument: " << arg << std::endl;
            print_usage();
            exit(1);
        }
    }
    
    return config;
}

void print_usage() {
    std::cout << "QAP Solver - Grey Wolf Optimizer with Tabu Search\n";
    std::cout << "Usage: ./qap_solver [options]\n\n";
    std::cout << "Options:\n";
    std::cout << "  --input-file FILE     Path to QAP instance file (default: silicon_spire.txt)\n";
    std::cout << "  --pack-size SIZE      Number of wolves (default: 30)\n";
    std::cout << "  --max-iterations N    Maximum GWO iterations (default: 100)\n";
    std::cout << "  --ts-iterations N     Tabu Search iterations (default: 50)\n";
    std::cout << "  --tabu-tenure N       Tabu list size (default: 10)\n";
    std::cout << "  --help, -h            Show this help message\n";
}