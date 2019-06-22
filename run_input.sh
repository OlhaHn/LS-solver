#!/bin/bash
FILES=./input/us_pigeon-hole/*
for f in $FILES
do
  echo "$f"
  #sed -i 's/  */ /g' $f
  ./b.out < $f
done