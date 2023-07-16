#!/bin/bash
for i in $(seq 1 10);
do
bash ./test_all_rome.sh $((60*$i))
bash ./test_all_random.sh $((60*$i))
#bash ./test_all_grid.sh $((60*$i))
done

