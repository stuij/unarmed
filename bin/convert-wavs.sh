#!/bin/bash

for file in $1/*.wav; do
    brr_encoder -l "$file" "${file%.wav}.brr"
done
