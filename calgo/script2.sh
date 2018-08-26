#!/bin/bash

for n in `seq 1 100`
do
  ./toto < vs.txt 2> "log-SIMU-$n" 1> /dev/null
  echo -n -e "\x1b[1K\r$n "
  echo "set terminal postscript enhanced
set xlabel \"Iterations\"
set ylabel \"Potential\"
set term png
set output \"pictures/SIMU-$n.png\"
plot 'log-SIMU-$n' pt 7 ps 0.1 lc 'black' t 'Travelling time'
quit" | gnuplot
done;

echo ""

