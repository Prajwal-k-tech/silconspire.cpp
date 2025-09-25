#!/usr/bin/env python3
"""
Generate a reproducible clustered QAP instance to exercise metaheuristics.
Writes an instance file with: n, distance matrix (n x n), flow matrix (n x n).
Default: n=50, clusters=5, seed=42
"""
import argparse
import random

def generate(n=50, clusters=5, seed=42):
    random.seed(seed)
    # assign each location to a cluster
    cluster_of = [i * clusters // n for i in range(n)]
    # ensure roughly equal clusters
    # distances: within-cluster small; between-cluster larger (depending on cluster separation)
    D = [[0]*n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i == j:
                D[i][j] = 0
            else:
                if cluster_of[i] == cluster_of[j]:
                    # within cluster distances small (1..10)
                    D[i][j] = 1 + random.randint(0,9)
                else:
                    # between clusters: base depends on cluster distance
                    cd = abs(cluster_of[i] - cluster_of[j])
                    D[i][j] = 30 + cd*10 + random.randint(0,19)
    # flows: strong inside cluster, mixed across clusters
    F = [[0]*n for _ in range(n)]
    for i in range(n):
        for j in range(n):
            if i == j:
                F[i][j] = 0
            else:
                if cluster_of[i] == cluster_of[j]:
                    # strong intra-cluster flows (80..200)
                    F[i][j] = random.randint(80,200)
                else:
                    # inter-cluster flows: mostly moderate (5..80) but introduce some conflicting heavy flows
                    if random.random() < 0.05:
                        # rare heavy cross-cluster linkage to create conflicting objective
                        F[i][j] = random.randint(120,220)
                    else:
                        F[i][j] = random.randint(5,80)
    return D, F

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--n', type=int, default=50)
    parser.add_argument('--clusters', type=int, default=5)
    parser.add_argument('--seed', type=int, default=42)
    parser.add_argument('--out', type=str, default='instances/meta_massive_50.txt')
    args = parser.parse_args()

    D, F = generate(n=args.n, clusters=args.clusters, seed=args.seed)

    with open(args.out, 'w') as fh:
        fh.write(str(args.n) + "\n")
        for i in range(args.n):
            fh.write(' '.join(str(x) for x in D[i]) + '\n')
        for i in range(args.n):
            fh.write(' '.join(str(x) for x in F[i]) + '\n')

    print('Wrote', args.out, 'n=', args.n, 'clusters=', args.clusters, 'seed=', args.seed)
