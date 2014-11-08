gemforce
========

Gem combining program for GC2:CS - Written in C.

***

### HOWTO use our programs:

**manual compilation:**  
  `gcc -O2 -lm -I "include" file.c -o output`
  
**automatic setup:**  
  in the repo root run `./auto_setup.sh`  
  everything you typically need will be placed in `bin/`

**run:**  
  `./file -[flags] number(s)`
  
where flags are:  
  p - print best gem parens formula  
  t - print best gem tree formula  
  e - print best gem equations formula  
  c - print best gems table (for each value best gem stats)   
  i - info  
  f (filepath) - change table file (build/query only)  
  l (num) - limit hard gem computation to num (amps only)  
  s (num) - change chunk size (you should not need this)  
  number - value of gem to be computed  
  numbers - spec value and comb value (omnia programs only)
  
to specify if you want a speccing or a combining recipe out of a general program
(e.g. managemquery) use (num)s or (num)c (default is speccing)  
global mode in (m/k)gaquery is toggled via (num)t (you should not need this)

**example:**  
  `./file -pet 32`  

***

### Results and tables:

All resulting schemes (recipes) are placed in "results" folder.  
Precomputed tables up to 1 MB are placed in "gem_tables" folder.  
More info on both can be found on the respective folders' README.

***

### Roadmap:

Orange combine - Done  
Orange/black spec - Done  
Orange/black combine - Done  
Yellow/black spec - Done  
Yellow/black combine - Done  
Amplifier-oriented OB - Done  
Amplifier-oriented YB - Done  
OBR and YBR
