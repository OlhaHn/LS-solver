#!/bin/bash
FILES=./input/s_SW100-8-5/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done