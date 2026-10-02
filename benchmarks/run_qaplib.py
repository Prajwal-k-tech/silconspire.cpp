#!/usr/bin/env python3
"""Repeat the default QAP search on three checksum-pinned QAPLIB instances."""

import argparse
import csv
import hashlib
import re
import statistics
import subprocess
import sys
import tempfile
import time
import urllib.request
from pathlib import Path


QAPLIB_DATA = "https://coral.ise.lehigh.edu/wp-content/uploads/2014/07/data.d"
INSTANCES = {
    "nug12": (578, "15ad1047d3d39e2820466349dc75ce8cc560443e125c4f67ff06223d735f47b6"),
    "nug14": (1014, "ffa06e53e4ffde301651f2a3c627ff1350b9282d53fdd8b76f388c76d0ec0ed0"),
    "tai12a": (224416, "3a9fa8069e82f9fd7806c37175e9c568a66ffcf9798935bf1f7336df81938318"),
}
OBJECTIVE_PATTERNS = (
    re.compile(r"Best cost found:\s*(\d+)"),
    re.compile(r"Minimum Cost:\s*(\d+)"),
)


def fetch_instance(name: str, directory: Path) -> Path:
    expected_sha = INSTANCES[name][1]
    request = urllib.request.Request(
        f"{QAPLIB_DATA}/{name}.dat",
        headers={"User-Agent": "SiliconSpire-QAPLIB-benchmark/1.0"},
    )
    try:
        with urllib.request.urlopen(request, timeout=30) as response:
            content = response.read()
    except Exception as error:
        raise RuntimeError(f"could not download QAPLIB instance {name}: {error}") from error

    actual_sha = hashlib.sha256(content).hexdigest()
    if actual_sha != expected_sha:
        raise RuntimeError(
            f"QAPLIB {name} checksum mismatch: expected {expected_sha}, got {actual_sha}"
        )
    path = directory / f"{name}.dat"
    path.write_bytes(content)
    return path


def objective_from_output(output: str) -> int:
    for pattern in OBJECTIVE_PATTERNS:
        match = pattern.search(output)
        if match:
            return int(match.group(1))
    raise RuntimeError("solver output did not contain a recognized final objective")


def run_instance(solver: str, path: Path, name: str, runs: int, first_seed: int):
    optimum = INSTANCES[name][0]
    values = []
    rows = []
    for seed in range(first_seed, first_seed + runs):
        command = [
            solver,
            "--input-file",
            str(path),
            "--pack-size",
            "30",
            "--max-iterations",
            "100",
            "--ts-iterations",
            "50",
            "--tabu-tenure",
            "10",
            "--seed",
            str(seed),
        ]
        started = time.perf_counter()
        result = subprocess.run(command, check=True, capture_output=True, text=True)
        elapsed = time.perf_counter() - started
        value = objective_from_output(result.stdout)
        values.append(value)
        rows.append((name, seed, value, 100.0 * (value - optimum) / optimum, elapsed))
    return values, rows


def main() -> int:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("--solver", required=True, help="path to the built solver executable")
    parser.add_argument("--runs", type=int, default=30, help="runs per instance (default: 30)")
    parser.add_argument("--first-seed", type=int, default=1, help="first seed (default: 1)")
    parser.add_argument("--csv", type=Path, help="optional path for per-seed results")
    args = parser.parse_args()
    if args.runs < 1 or args.first_seed < 0:
        parser.error("--runs must be positive and --first-seed must be nonnegative")

    solver = str(Path(args.solver).expanduser().resolve())
    if not Path(solver).is_file():
        parser.error(f"solver executable not found: {solver}")

    summaries = []
    all_rows = []
    try:
        with tempfile.TemporaryDirectory(prefix="silconspire-qaplib-") as temporary:
            directory = Path(temporary)
            for name, (optimum, _) in INSTANCES.items():
                instance = fetch_instance(name, directory)
                values, rows = run_instance(
                    solver, instance, name, args.runs, args.first_seed
                )
                gaps = [100.0 * (value - optimum) / optimum for value in values]
                summaries.append(
                    (
                        name,
                        optimum,
                        args.runs,
                        sum(value == optimum for value in values),
                        min(values),
                        statistics.median(values),
                        statistics.mean(gaps),
                    )
                )
                all_rows.extend(rows)
    except (RuntimeError, subprocess.CalledProcessError) as error:
        print(f"benchmark failed: {error}", file=sys.stderr)
        return 1

    print("Instance | Optimum | Runs | Exact hits | Best | Median | Mean gap")
    print("--- | ---: | ---: | ---: | ---: | ---: | ---:")
    for name, optimum, runs, hits, best, median, mean_gap in summaries:
        print(
            f"{name} | {optimum} | {runs} | {hits}/{runs} | {best} | "
            f"{median:g} | {mean_gap:.3f}%"
        )

    if args.csv:
        with args.csv.open("w", newline="", encoding="utf-8") as output:
            writer = csv.writer(output)
            writer.writerow(("instance", "seed", "objective", "gap_percent", "elapsed_seconds"))
            writer.writerows(all_rows)
    return 0


if __name__ == "__main__":
    raise SystemExit(main())
