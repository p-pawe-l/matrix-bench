"""Plot matrix multiplication benchmark results with seaborn."""

import argparse
from pathlib import Path

import matplotlib.pyplot as plt
import pandas as pd
import seaborn as sns
from matplotlib.ticker import FuncFormatter

from parse_results import ALGORITHMS, DEFAULT_PATH, ROOT, add_speedup, load_results

# Fixed categorical order, validated for colorblind separation — color follows the algorithm
PALETTE = dict(zip(ALGORITHMS.values(), ["#2a78d6", "#eb6834", "#1baf7a", "#6250d6"]))
MARKERS = dict(zip(ALGORITHMS.values(), ["o", "s", "D", "^"]))

SURFACE = "#fcfcfb"
TEXT_PRIMARY = "#0b0b0b"
TEXT_SECONDARY = "#52514e"
GRID = "#e4e3df"


def apply_style() -> None:
    sns.set_theme(
        style="whitegrid",
        rc={
            "figure.facecolor": SURFACE,
            "axes.facecolor": SURFACE,
            "axes.edgecolor": GRID,
            "axes.labelcolor": TEXT_SECONDARY,
            "axes.titlecolor": TEXT_PRIMARY,
            "axes.titleweight": "bold",
            "axes.titlesize": 13,
            "axes.titlelocation": "left",
            "axes.spines.top": False,
            "axes.spines.right": False,
            "grid.color": GRID,
            "grid.linewidth": 0.8,
            "xtick.color": TEXT_SECONDARY,
            "ytick.color": TEXT_SECONDARY,
            "legend.frameon": False,
            "font.size": 10,
        },
    )


def _line_panel(ax: plt.Axes, df: pd.DataFrame, y: str, title: str, ylabel: str) -> None:
    sns.lineplot(
        data=df, x="n", y=y, hue="algorithm", style="algorithm",
        palette=PALETTE, markers=MARKERS, dashes=False,
        linewidth=2, markersize=8, markeredgecolor=SURFACE, markeredgewidth=1.5,
        ax=ax, legend=False,
    )
    ax.set_xscale("log", base=2)
    ax.set_yscale("log")
    ax.set_xticks(sorted(df["n"].unique()))
    ax.xaxis.set_major_formatter(FuncFormatter(lambda v, _: f"{int(v)}"))
    ax.xaxis.set_minor_locator(plt.NullLocator())
    ax.set_title(title)
    ax.set_xlabel("Matrix size n (n × n)")
    ax.set_ylabel(ylabel)

    ax.margins(x=0.12)


def _end_labels(ax: plt.Axes, df: pd.DataFrame, y: str, min_gap_pt: float = 12) -> None:
    """Direct label at the end of each line, in text ink, nudged apart so they never overlap."""
    last = df[df["n"] == df["n"].max()].sort_values(y)
    px_per_pt = ax.figure.dpi / 72
    min_gap = min_gap_pt * px_per_pt

    placed = []
    for _, row in last.iterrows():
        y_px = ax.transData.transform((row["n"], row[y]))[1]
        target = max(y_px, placed[-1][1] + min_gap) if placed else y_px
        placed.append((row, target, y_px))

    for row, target, y_px in placed:
        ax.annotate(
            f"{row[y]:.0f} ms" if y == "time_ms" else f"{row[y]:.1f}×",
            # offset in points so it survives savefig's different dpi
            (row["n"], row[y]), xytext=(8, (target - y_px) / px_per_pt), textcoords="offset points",
            va="center", fontsize=9, color=TEXT_SECONDARY,
        )


def plot_results(df: pd.DataFrame, out_path: Path, show: bool = False) -> None:
    apply_style()
    df = add_speedup(df)
    df_time = df[df["time_ms"] > 0]  # 0 ms can't be shown on a log axis

    fig, (ax_time, ax_speed) = plt.subplots(1, 2, figsize=(13, 5.5), layout="constrained")
    _line_panel(ax_time, df_time, "time_ms", "Run time", "Time (ms, log scale)")
    _line_panel(ax_speed, df, "speedup", "Speedup over row prior", "Speedup (×, log scale)")
    ax_speed.axhline(1, color=TEXT_SECONDARY, linewidth=1, linestyle=":", zorder=0)

    handles = [
        plt.Line2D([], [], color=PALETTE[a], marker=MARKERS[a], linewidth=2, markersize=8,
                   markeredgecolor=SURFACE, label=a)
        for a in ALGORITHMS.values()
    ]
    fig.legend(handles=handles, loc="outside lower center", ncols=len(handles), labelcolor=TEXT_PRIMARY)
    fig.suptitle("Matrix multiplication benchmark", fontsize=15, fontweight="bold", color=TEXT_PRIMARY)

    # Labels are placed in pixels, so finalize the layout first
    fig.canvas.draw()
    _end_labels(ax_time, df_time, "time_ms")
    _end_labels(ax_speed, df, "speedup")
    fig.set_layout_engine("none")

    out_path.parent.mkdir(parents=True, exist_ok=True)
    fig.savefig(out_path, dpi=200, facecolor=SURFACE)
    print(f"Saved plot to {out_path}")
    if show:
        plt.show()


def main() -> None:
    parser = argparse.ArgumentParser(description=__doc__)
    parser.add_argument("json", nargs="?", type=Path, default=DEFAULT_PATH, help="results JSON")
    parser.add_argument("-o", "--out", type=Path, default=ROOT / "assets" / "results.png")
    parser.add_argument("--show", action="store_true", help="also open an interactive window")
    args = parser.parse_args()
    plot_results(load_results(args.json), args.out, args.show)


if __name__ == "__main__":
    main()
