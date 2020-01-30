#!/usr/bin/gnuplot

set datafile separator ","
set title "Raw magnetometer data (XYZ)"
set term qt size 1300,1000
set size ratio -1
set grid

plot "mag_raw.csv" using 1:2 title "XY" pointsize 2 pointtype 7, \
     "mag_raw.csv" using 1:3 title "XZ" pointsize 2 pointtype 7, \
     "mag_raw.csv" using 2:3 title "YZ" pointsize 2 pointtype 7

pause -1
