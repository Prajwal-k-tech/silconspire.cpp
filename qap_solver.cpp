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
using namespace std;

struct Problem {
    int n;
    vector<vector<int>> distance; //the distance matrix, distances between the factories 
    vector<vector<int>> flow; //the amount of flow between facilities 
    Problem(int size) : n(size) { //part after the colon is a member initializer, we initialize n to size 
        distance.resize(n, vector<int>(n)); //resize to size n*n
        flow.resize(n, vector<int>(n)); //same thing here 
    }
};

struct Wolf {
    vector<double> position; //position of the wolf in the search space
    vector<int> permutation; //the permutation, obtained by decoding using LVM
    long long fitness; //the fitness of the solution (use 64-bit to avoid overflow)
    Wolf(int size) : position(size), permutation(size), fitness(LLONG_MAX) {
        iota(permutation.begin(), permutation.end(), 0); //initialize with 0,1,2,3,...,n-1
    }
};

// Command line arguments structure
struct Config {
    string input_file = "silicon_spire.txt"; //default input file 
    int pack_size = 30;
    int max_iterations = 100;
    int ts_iterations = 50;
    int tabu_tenure = 10;
    // additional controls
    int ts_every = 1; // apply Tabu Search every K iterations (1 = every iteration)
    double jitter = 0.0; // add small uniform noise in [-jitter, jitter] before LVP decode
};

// Function declarations
Problem load_problem(const string& filename); //function to load the problem from a file
long long calculate_cost(const Problem& problem, const vector<int>& permutation); //function to calculate the cost of a given permutation
vector<int> lvp_decode(const vector<double>& position); //do the lvp decode, returns a permutation 
void apply_tabu_search(const Problem& problem, Wolf& wolf, int ts_iterations, int tabu_tenure); //apply tabu search to a wolf
Config parse_arguments(int argc, char* argv[]); //parse command line arguments
void print_usage();

int main(int argc, char* argv[]) {
    try {
        // Parse command line arguments
        Config config = parse_arguments(argc, argv);
    //Load problem instance
    cout << "Loading QAP instance from: " << config.input_file << endl;
    Problem problem = load_problem(config.input_file);
    cout << "Problem size: " << problem.n << "x" << problem.n << endl;
    // Initialize random number generator
    random_device rd;
    mt19937 gen(rd());
    uniform_real_distribution<> dis(-1.0, 1.0);
    // Initialize wolf pack
    vector<Wolf> wolves(config.pack_size, Wolf(problem.n)); //initalize pack of wolves
        Wolf alpha(problem.n), beta(problem.n), delta(problem.n); //initalize alpha, beta, delta wolves
         // --- IGNORE ---
        // Initialize wolves with random positions
        for (auto& wolf : wolves) {
            for (double& pos : wolf.position) {
                pos = dis(gen);
            }
            // optional initial jitter
            if (config.jitter > 0.0) {
                uniform_real_distribution<> jdis(-config.jitter, config.jitter);
                for (double& pos : wolf.position) pos += jdis(gen);
            }
            wolf.permutation = lvp_decode(wolf.position);
            wolf.fitness = calculate_cost(problem, wolf.permutation);
        }
    // Find initial alpha, beta, delta
    sort(wolves.begin(), wolves.end(), 
         [](const Wolf& a, const Wolf& b) { return a.fitness < b.fitness; });
        alpha = wolves[0];
        beta = wolves[1];
        delta = wolves[2];
    cout << "\nStarting Grey Wolf Optimizer + Tabu Search hybrid algorithm..." << endl;
    cout << "Pack size: " << config.pack_size << ", Max iterations: " << config.max_iterations << endl;
    cout << "Tabu Search iterations: " << config.ts_iterations << ", Tabu tenure: " << config.tabu_tenure << endl;
    cout << "Initial best cost: " << alpha.fitness << endl << endl;
        
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
                    double D_alpha = abs(C1 * alpha.position[i] - wolf.position[i]);
                    double X1 = alpha.position[i] - A1 * D_alpha;
                    
                    // Beta influence
                    r1 = dis(gen); r2 = dis(gen);
                    double A2 = 2 * a * r1 - a;
                    double C2 = 2 * r2;
                    double D_beta = abs(C2 * beta.position[i] - wolf.position[i]);
                    double X2 = beta.position[i] - A2 * D_beta;
                    
                    // Delta influence
                    r1 = dis(gen); r2 = dis(gen);
                    double A3 = 2 * a * r1 - a;
                    double C3 = 2 * r2;
                    double D_delta = abs(C3 * delta.position[i] - wolf.position[i]);
                    double X3 = delta.position[i] - A3 * D_delta;
                    
                    // Update position
                    wolf.position[i] = (X1 + X2 + X3) / 3.0;
                    
                    // Clamp position to [-1, 1]
                    wolf.position[i] = max(-1.0, min(1.0, wolf.position[i]));
                }
                
                // Optional jitter before decode to increase discrete diversity
                if (config.jitter > 0.0) {
                    uniform_real_distribution<> jdis(-config.jitter, config.jitter);
                    for (double& pos : wolf.position) {
                        pos += jdis(gen);
                        // Re-clamp after jitter to maintain bounds
                        pos = max(-1.0, min(1.0, pos));
                    }
                }
                // Convert to permutation and calculate fitness
                wolf.permutation = lvp_decode(wolf.position);
                wolf.fitness = calculate_cost(problem, wolf.permutation);
            }
            
            // Sort wolves and update alpha, beta, delta
            sort(wolves.begin(), wolves.end(), 
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
            
            // Apply Tabu Search to alpha wolf (hybridization) every ts_every iterations
            if (config.ts_iterations > 0 && config.ts_every > 0 && (iteration % config.ts_every == 0)) {
                apply_tabu_search(problem, alpha, config.ts_iterations, config.tabu_tenure);
            }
            // Update wolves[0] with improved alpha
            wolves[0] = alpha;
            // Progress output
            if ((iteration + 1) % 10 == 0 || improved) {
                cout << "Iteration " << (iteration + 1) 
                         << ": Best cost = " << alpha.fitness << endl;
            }
        }
        
    // Final results
    cout << "\n=== FINAL RESULTS ===" << endl;
    cout << "Best cost found: " << alpha.fitness << endl;
    cout << "Best assignment:" << endl;
        
        // Print final assignment using numeric facility indices starting from 0
        for (int i = 0; i < problem.n; i++) {
            cout << "  Facility " << i << " -> Location " << alpha.permutation[i] << endl;
        }
        
    } catch (const exception& e) {
        cerr << "Error: " << e.what() << endl;
        return 1;
    }
    
    return 0;
}

Problem load_problem(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        throw runtime_error("Cannot open file: " + filename);
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

long long calculate_cost(const Problem& problem, const vector<int>& permutation) {
    long long cost = 0;
    for (int i = 0; i < problem.n; i++) {
        for (int j = 0; j < problem.n; j++) {
            cost += static_cast<long long>(problem.flow[i][j]) * static_cast<long long>(problem.distance[permutation[i]][permutation[j]]);
        }
    }
    return cost;
}

vector<int> lvp_decode(const vector<double>& position) {
    int n = position.size();
    vector<pair<double, int>> sorted_positions;

    for (int i = 0; i < n; i++) {
        sorted_positions.push_back({position[i], i});
    }

    sort(sorted_positions.begin(), sorted_positions.end(), greater<>());

    vector<int> permutation(n);
    for (int i = 0; i < n; i++) {
        permutation[sorted_positions[i].second] = i;
    }

    return permutation;
}

void apply_tabu_search(const Problem& problem, Wolf& wolf, int ts_iterations, int tabu_tenure) {
    deque<pair<int, int>> tabu_list;
    vector<int> current_solution = wolf.permutation;
    vector<int> best_solution = current_solution;
    long long current_cost = wolf.fitness;
    long long best_cost = current_cost;
    static long long global_best = LLONG_MAX;  // Track global best across all TS calls
    if (best_cost < global_best) {
        global_best = best_cost;
    }
    
    for (int iter = 0; iter < ts_iterations; iter++) {
    vector<int> best_neighbor = current_solution;
        long long best_neighbor_cost = LLONG_MAX;
        int best_i = -1, best_j = -1;
        
        // Explore 2-opt neighborhood
        for (int i = 0; i < problem.n - 1; i++) {
            for (int j = i + 1; j < problem.n; j++) {
                // Create neighbor by swapping positions i and j
                vector<int> neighbor = current_solution;
                std::swap(neighbor[i], neighbor[j]);
                
                long long neighbor_cost = calculate_cost(problem, neighbor);
                
                // Check if move is tabu
                bool is_tabu = false;
                for (const auto& tabu_move : tabu_list) {
                    if ((tabu_move.first == i && tabu_move.second == j) ||
                        (tabu_move.first == j && tabu_move.second == i)) {
                        is_tabu = true;
                        break;
                    }
                }
                //Accept move if not tabu or if it improves global best (aspiration criterion)
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
        
        // If no valid move found (all moves are tabu and don't satisfy aspiration), break
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
        string arg = argv[i];

        if (arg == "--help" || arg == "-h") {
            print_usage();
            exit(0);
        } else if (arg == "--input-file" && i + 1 < argc) {
            config.input_file = argv[++i];
            } else if (arg == "--pack-size" && i + 1 < argc) {
                config.pack_size = stoi(argv[++i]);
                if (config.pack_size < 3) {
                    throw invalid_argument("Pack size must be at least 3 (needed for alpha/beta/delta)");
                }
        } else if (arg == "--max-iterations" && i + 1 < argc) {
            config.max_iterations = stoi(argv[++i]);
            if (config.max_iterations < 1) {
                throw invalid_argument("Max iterations must be positive");
            }
        } else if (arg == "--ts-iterations" && i + 1 < argc) {
            config.ts_iterations = stoi(argv[++i]);
            if (config.ts_iterations < 0) {
                throw invalid_argument("TS iterations must be >= 0 (use 0 to disable Tabu Search)");
            }
        } else if (arg == "--tabu-tenure" && i + 1 < argc) {
            config.tabu_tenure = stoi(argv[++i]);
            if (config.tabu_tenure < 1) {
                throw invalid_argument("Tabu tenure must be positive");
            }
        } else if (arg == "--ts-every" && i + 1 < argc) {
            config.ts_every = stoi(argv[++i]);
            if (config.ts_every < 1) {
                throw invalid_argument("ts-every must be >= 1");
            }
        } else if (arg == "--jitter" && i + 1 < argc) {
            config.jitter = stod(argv[++i]);
            if (config.jitter < 0.0) {
                throw invalid_argument("jitter must be >= 0");
            }
        } else {
            cerr << "Unknown argument: " << arg << endl;
            print_usage();
            exit(1);
        }
    }

    return config;
}

void print_usage() { //implementation of the function that prints usage options
    cout << "QAP Solver - Grey Wolf Optimizer with Tabu Search\n";
    cout << "Usage: ./qap_solver [options]\n\n";
    cout << "Options:\n";
    cout << "  --input-file FILE     Path to QAP instance file (default: silicon_spire.txt)\n";
    cout << "  --pack-size SIZE      Number of wolves (default: 30)\n";
    cout << "  --max-iterations N    Maximum GWO iterations (default: 100)\n";
    cout << "  --ts-iterations N     Tabu Search iterations (default: 50, 0 = disabled)\n";
    cout << "  --tabu-tenure N       Tabu list size (default: 10)\n";
    cout << "  --ts-every K          Apply Tabu Search every K iterations (default: 1)\n";
    cout << "  --jitter x            Add uniform jitter in [-x,x] before decoding (default: 0.0)\n";
    cout << "  --help, -h            Show this help message\n";
}