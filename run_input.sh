#!/bin/bash
FILES=./input/s_blocksworld/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done