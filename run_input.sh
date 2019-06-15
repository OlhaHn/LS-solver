#!/bin/bash
FILES=./input/us_UUF125.538.100/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done