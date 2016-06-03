# Datafile to show
# datafile='' # externally initialized

#Inizializzazione

reset
set output #chiude eventuali files aperti
set terminal x11 enhanced

#Definizione e formattazione del grafico

set title 'Pool 2D graph'
set xlabel 'Leech'	# {/Symbol roba_da_scrivere_in_greco_grazie_a_enhanced}
set ylabel 'Bbound'
set xrange [*:*]
set yrange [*:*]
unset key #toglie la legenda al grafico
#set samples 10000
#set logscale xy
#set xtics (80,90,100,200,300)
#set ytics (2000,4000,6000,8000,10000,20000)

#Plot (scegliere le barre d'errore volute)
plot datafile using 2:3:1 pointtype 13 pointsize 1 linetype palette
print 'Press any key to exit'
pause -1
