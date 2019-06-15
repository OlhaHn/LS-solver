#!/bin/bash
FILES=./input/us_pigeon-hole/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./a.out < $f
done