#!/bin/bash
for i in $(seq 1 10);
do
bash ./test_all_rome.sh $((60*$i))
done

