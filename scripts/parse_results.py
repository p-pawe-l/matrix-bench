"""Load the benchmark JSON written by matrix_comp into a tidy DataFrame."""

import json
import sys
from pathlib import Path

import pandas as pd

ROOT = Path(__file__).resolve().parent.parent
DEFAULT_PATH = ROOT / "results" / "results.json"

# JSON key -> human-readable algorithm name (also fixes the plotting order)
ALGORITHMS = {
    "row_prior_mul": "Row prior",
    "col_prior_mul": "Col prior",
    "mul": "Row-broadcast (mul)",
    "multithread_mul": "Multithreaded",
}


def load_results(path: Path = DEFAULT_PATH) -> pd.DataFrame:
    """Return one row per (n, algorithm) with columns: n, algorithm, time_ms."""
    with open(path) as f:
        data = json.load(f)

    wide = pd.DataFrame(data["results"])
    long = wide.melt(id_vars="n", value_vars=list(ALGORITHMS), var_name="algorithm", value_name="time_ms")
    long["algorithm"] = pd.Categorical(
        long["algorithm"].map(ALGORITHMS), categories=list(ALGORITHMS.values()), ordered=True
    )
    return long.sort_values(["algorithm", "n"]).reset_index(drop=True)


def add_speedup(df: pd.DataFrame, baseline: str = "Row prior") -> pd.DataFrame:
    """Add a speedup column: baseline time / algorithm time at the same n."""
    base = df[df["algorithm"] == baseline].set_index("n")["time_ms"]
    out = df.copy()
    # Clamp 0 ms runs to 1 ms so small sizes don't divide by zero
    out["speedup"] = out["n"].map(base).clip(lower=1) / out["time_ms"].clip(lower=1)
    return out


if __name__ == "__main__":
    path = Path(sys.argv[1]) if len(sys.argv) > 1 else DEFAULT_PATH
    print(load_results(path).pivot(index="n", columns="algorithm", values="time_ms"))
