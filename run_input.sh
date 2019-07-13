#!/bin/bash
FILES=./input/s_uf50-218/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done