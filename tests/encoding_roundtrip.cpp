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
    return 0;
}
