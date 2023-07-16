#!/bin/bash
run_random_graph(){
file_path="data/gd_challenge/rand"
out_path="data/res/random/"$2"/rand"
file_type=".json"
echo "rand"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
echo "finished rand"$1" at "$current_time
}
mkdir -p data/res/random/$1
rm -rf data/res/random/$1/*
start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
run_random_graph "_20_1" $1
run_random_graph "_20_01" $1
run_random_graph "_20_05" $1
run_random_graph "_50_1" $1
run_random_graph "_50_01" $1
run_random_graph "_50_05" $1
run_random_graph "_50_08" $1
run_random_graph "_100_01" $1
run_random_graph "_100_03" $1
run_random_graph "_1000_01" $1
run_random_graph "_1000_001" $1
run_random_graph "_1000_0001" $1
run_random_graph "_1000_02" $1
run_random_graph "_1000_0005" $1

