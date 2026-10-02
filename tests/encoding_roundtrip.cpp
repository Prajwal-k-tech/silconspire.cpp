#define SILICON_SPIRE_NO_MAIN
#include "../qap_solver.cpp"

int main() {
    const std::vector<std::vector<int>> examples = {
        {0},
        {0, 1},
        {2, 0, 3, 1},
        {4, 2, 0, 3, 1},
    };

    for (const auto& permutation : examples) {
        if (lvp_decode(lvp_encode(permutation)) != permutation) {
            std::cerr << "LVP encode/decode round trip failed\n";
            return 1;
        }
    }

    Problem problem(2);
    problem.distance = {{1, 2}, {3, 4}};
    problem.flow = {{5, 6}, {7, 8}};
    if (calculate_cost(problem, {1, 0}) != 60) {
        std::cerr << "QAP objective calculation returned the wrong value\n";
        return 1;
    }

    bool rejected_invalid_assignment = false;
    try {
        (void)calculate_cost(problem, {0, 0});
    } catch (const std::invalid_argument&) {
        rejected_invalid_assignment = true;
    }
    if (!rejected_invalid_assignment) {
        std::cerr << "Duplicate assignment was accepted\n";
        return 1;
    }

    Problem overflowing_problem(2);
    overflowing_problem.distance = {{INT_MAX, INT_MAX}, {INT_MAX, INT_MAX}};
    overflowing_problem.flow = {{INT_MAX, INT_MAX}, {INT_MAX, INT_MAX}};
    bool rejected_overflow = false;
    try {
        (void)calculate_cost(overflowing_problem, {0, 1});
    } catch (const std::overflow_error&) {
        rejected_overflow = true;
    }
    if (!rejected_overflow) {
        std::cerr << "Overflowing QAP objective was accepted\n";
        return 1;
    }

    return 0;
}
