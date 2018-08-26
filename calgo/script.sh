#!/bin/bash


for j in `seq 101 1000`
do
  echo -e -n "\x1b[1K\rSimu $j..."
  ./toto < vs.txt 2> log 1> /dev/null
  ./extractor 2 "#" "log1-$j" "log2-$j" < log
  echo "set terminal postscript enhanced
set xlabel \"Iterations\"
set ylabel \"Potential\"
set term png
set output \"pictures/Gamma-$j.png\"
plot  \"log1-$j\" pt 7 ps 0.3 lc 'black' t \"unrelevant\" , \"log2-$j\" pt 7 ps 0.2 lc 'red' t \"corrected\" 
quit" | gnuplot
done;
