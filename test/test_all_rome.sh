#!/bin/bash
run_rome_graph(){
file_path="data/gd_challenge/rome"
out_path="data/res/rome/"$2"/rome"
file_type=".json"
echo "rome"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_"$((1000*$2)) --time $((1000*$2)) > $out_path$1"_"$((1000*$2))"_output.txt"
current_time=$(date +”%H:%M:%S”)
echo "finished rome"$1" at "$current_time
}
mkdir -p data/res/rome/$1
rm -rf data/res/rome/$1/*
start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
for i in $(seq 1 16);
do
run_rome_graph $i $1
done

