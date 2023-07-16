#!/bin/bash
run_grid_graph(){
file_path="data/gd_challenge/grid"
out_path="data/res/grid/"$2"/grid"
file_type=".json"
echo "grid"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
echo "finished grid"$1" at "$current_time
}
mkdir -p data/res/grid/$1
rm -rf data/res/grid/$1/*
start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
run_grid_graph "10_1" $1
run_grid_graph "10_05" $1
run_grid_graph "20_02" $1
run_grid_graph "20_05" $1
run_grid_graph "20_08" $1
run_grid_graph "50_1" $1
run_grid_graph "50_02" $1
run_grid_graph "50_05" $1
run_grid_graph "50_08" $1

