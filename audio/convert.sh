#!/bin/bash

for file in input/*; do
  output="${file%.*}.ogg"
  output=$(basename $output)
  echo "converting $file into $output"
  ffmpeg -i "$file" -c:a libvorbis -q:a 4 -af "loudnorm=I=-21.15:TP=-1.5:LRA=11" output/"$output"
done
