#!/usr/bin/gnuplot

set datafile separator whitespace
# set datafile separator ","

set title "STEPAN_3"
set term qt size 1700,1000
#set size ratio -1
set xtics 10
set ytics 5
set grid

set style data linespoints

plot \
     "z.log" using 10:2			title "spP"	pointinterval -12	pointsize 1	lw 2	pointtype "P",	\
     "z.log" using 10:4			title "spI"	pointinterval -12	pointsize 1	lw 2	pointtype "I",	\
     "z.log" using 10:($6/10)		title "coP/10"	pointinterval -12	pointsize 1	lw 2	pointtype "p",	\
     "z.log" using 10:($8/10)		title "coD/10"	pointinterval -12	pointsize 1	lw 2	pointtype "d",	\
     "z.log" using 10:($14/10)		title "SPD/10"	pointinterval -12	pointsize 1	lw 2	pointtype "S",	\
     "z.log" using 10:18		title "ANG"	pointinterval -12	pointsize 1	lw 2	pointtype "A",	\
     "z.log" using 10:20		title "ANGT"	pointinterval -6	pointsize 1	lw 2	pointtype "T",	\
     "z.log" using 10:($22/10)		title "COUT/10"	pointinterval -12	pointsize 1	lw 2	pointtype "C",	\
     "z.log" using 10:($16/10)		title "SPDt/10"	pointinterval -12	pointsize 1	lw 2	pointtype "N"

pause -1
