#!/bin/sh

cd "$(dirname "$0")/.." || exit 1

current_time=$(date "+%Y-%m-%d_%H-%M-%S")

mkdir -p "benchmark/perf-stat/$current_time"

perf stat -o "benchmark/perf-stat/$current_time/stat.txt" "$@" ./build/social-network
