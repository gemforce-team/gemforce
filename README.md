gemforce
========

Gem combining program for GC2:CS - Written in C.


### HOWTO use our programs:

compile:  
  gcc -O2 -lm file.c -o output

run:  
  ./file -[flags] number
  
where flags are:  
  p - print best gem parens formula  
  e - print best gem equations formula  
  t - print best gem tree formula  
  c - print best gems table (for each value best gem stats)   
  d - debug + info  
  i - info  
  f (filepath) - change table file (build/query only)  
  l (num) - limit hard gem computation to num (amps only)  
  s (num) - change chunk size (you should not need this)  
  number - value of gem to be computed
  
example:  
  ./file -pet 32  


### Results and tables:

All resulting schemes (recipes) are placed in "results" folder.  
Precomputed tables up to 1 MB are placed in "gem_tables" folder.  
More info on both can be found on the folder's README.

### Roadmap:

Orange combine - Done  
Orange/black spec - Done  
Orange/black combine - Done  
Yellow/black spec - Done  
Yellow/black combine - Done  
Amplifier-oriented OB - Done  
Amplifier-oriented YB - Done  
OBR and YBR
