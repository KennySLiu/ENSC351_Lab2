#!/bin/bash
for i in `seq 1 3`;
do
    for j in `seq 1 15`;
    do
        ./a.out $i >> test_outputs/test_method${i}
    done
done


