#!/bin/bash
run_rome_graph(){
file_path="data/gd_challenge/rome"
out_path="data/res/rome/rome"
file_type=".json"
echo "rome"$1$file_type
../build/gd --file $file_path$1$file_type --out $out_path$1"_" --time $((1000*$2)) > $out_path$1"_output.txt"
current_time=$(date +”%H:%M:%S”)
echo "finished rome"$1" at "$current_time
}
rm -rf data/res/rome/*
start_time=$(date +”%H:%M:%S”)
echo "started at "$start_time
for i in $(seq 1 16);
do
run_rome_graph $i $1
done

