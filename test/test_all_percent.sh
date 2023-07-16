#!/bin/bash

run_rome_graph(){
file_path="data/gd_challenge/rome"
out_path="data/res/rome/"$2/$3"/rome"
file_type=".json"
#echo "rome"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) --initial_placement "0.$3" > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
#echo "finished rome"$1" at "$current_time
}
run_all_rome_graphs(){
mkdir -p data/res/rome/$1/$2
rm -rf data/res/rome/$1/$2/*
start_time=$(date +”%H:%M:%S”)
#echo "started at "$start_time
for i in $(seq 1 16);
do
run_rome_graph $i $1 $2
done
}

run_random_graph(){
file_path="data/gd_challenge/rand"
out_path="data/res/random/"$2/$3"/rand"
file_type=".json"
#echo "rand"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) --initial_placement "0.$3" > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
#echo "finished rand"$1" at "$current_time
}
run_all_rand_graphs(){
mkdir -p data/res/random/$1/$2
rm -rf data/res/random/$1/$2/*
start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
run_random_graph "_20_1" $1 $2
run_random_graph "_20_01" $1 $2
run_random_graph "_20_05" $1 $2
run_random_graph "_50_1" $1 $2
run_random_graph "_50_01" $1 $2
run_random_graph "_50_05" $1 $2
run_random_graph "_50_08" $1 $2
run_random_graph "_100_01" $1 $2
run_random_graph "_100_03" $1 $2
run_random_graph "_1000_01" $1 $2
run_random_graph "_1000_001" $1 $2
run_random_graph "_1000_0001" $1 $2
run_random_graph "_1000_02" $1 $2
run_random_graph "_1000_0005" $1 $2
}

run_grid_graph(){
file_path="data/gd_challenge/grid"
out_path="data/res/grid/"$2/$3"/grid"
file_type=".json"
#echo "grid"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) --initial_placement "0.$3" > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
#echo "finished grid"$1" at "$current_time
}
run_all_grid_graphs(){
mkdir -p data/res/grid/$1/$2
rm -rf data/res/grid/$1/$2/*
#start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
run_grid_graph "10_1" $1 $2
run_grid_graph "10_05" $1 $2
run_grid_graph "20_02" $1 $2
run_grid_graph "20_05" $1 $2
run_grid_graph "20_08" $1 $2
run_grid_graph "50_1" $1 $2
run_grid_graph "50_02" $1 $2
run_grid_graph "50_05" $1 $2
run_grid_graph "50_08" $1 $2
}

run_all_rome_graphs $1 $2
run_all_rand_graphs $1 $2
run_all_grid_graphs $1 $2
