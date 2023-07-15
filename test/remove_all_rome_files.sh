#!/bin/bash
for i in $(seq 1 10);
do
rm -rf data/res/rome/$((60*$i))/*
done

