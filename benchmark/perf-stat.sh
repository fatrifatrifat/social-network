#!/bin/sh

cd "$(dirname "$0")/.." || exit 1

current_time=$(date "+%Y-%m-%d_%H-%M-%S")

mkdir -p "benchmark/perf-stat/$current_time"

perf stat -o "benchmark/perf-stat/$current_time/edges_per_id.txt" "$@" ./build-per-id/social-network
perf stat -o "benchmark/perf-stat/$current_time/raw_edges.txt" "$@" ./build-raw-edges/social-network
