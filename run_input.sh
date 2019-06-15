#!/bin/bash
FILES=./input/s_uf20-91/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done