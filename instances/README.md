This directory contains larger QAP test instances derived from the Silicon Spire scenario.

Files:
- `silicon_spire_8.txt`  — 8×8 instance (distance matrix then flow matrix)
- `silicon_spire_10.txt` — 10×10 instance
- `silicon_spire_12.txt` — 12×12 instance

Format for each file:
- First line: integer n
- Next n lines: distance matrix (n rows of n whitespace-separated integers)
- Next n lines: flow matrix (n rows of n whitespace-separated integers)

Notes:
- These are synthetic, varied-scale instances intended to stress the solver so default small-pack runs won't finish by chance.
- For n ≥ 10 exhaustive verification is infeasible by brute force; use the solver with larger packs/iterations to compare relative improvements.

Suggested quick test (compile then run):
```bash
g++ -std=c++17 -O2 -Wall qap_solver.cpp -o qap_solver
./qap_solver --input-file instances/silicon_spire_8.txt --pack-size 30 --max-iterations 200 --ts-iterations 500 --tabu-tenure 50
```

More examples and instance generation
```
# Compile with warnings enabled
g++ -std=c++17 -O2 -Wall qap_solver.cpp -o qap_solver

# Run the large synthetic 50x50 instance (example parameters used in experiments):
./qap_solver --input-file instances/meta_massive_50.txt --pack-size 300 --max-iterations 2000 --ts-iterations 200 --tabu-tenure 80 --ts-every 50 --jitter 0.02

# Generate custom synthetic instances using the helper script:
# scripts/generate_meta_instance.py --n N --clusters K --seed S --out instances/my_instance_N.txt
# Example: create a 50×50 clustered instance (seeded for reproducibility)
python3 scripts/generate_meta_instance.py --n 50 --clusters 5 --seed 123 --out instances/meta_massive_50.txt
```
