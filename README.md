gemforce
========

Gem combining program for GC2:CS - Written in C.

***

### Results and tables:

All resulting schemes (recipes) are placed in "results" folder.  
Precomputed tables up to 1 MB are placed in "gem_tables" folder.  
More info on both can be found on the respective folders' README.

***

### HOWTO use our programs:

**manual compilation:**  
  `gcc -O2 -lm -I "include" file.c -o output`
  
**automatic setup (*NIX shell required):**  
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
  q - quiet, print only final gem  
  u - upto, at the end consider the best gem up to N, not the Nth  
  r - adds red at the best gem, then consider the new gem
  f (filepath) - change table file (build/query only)  
  s (num) - change chunk size (you should not need this)  
  number - value of gem to be computed  
  numbers - spec value and comb value (omnia programs only)  
  N (num) - specify how many amps you use (omnia programs only)  
  T (num) - give your True Colors skill for better amps fitting  (omnia programs only)
  
to specify if you want a speccing or a combining recipe out of a general program
(e.g. managemquery) use (num)s or (num)c (default is speccing)  
global mode in (m/k)gaquery is toggled via (num)g (you should not need this)

**example:**  
  `./file -pet 32`  

### What does every program do?

Mana and kill suites are alike, so I'll just describe the mana one:

Included in auto setup package:

* leechquery    : gets leech combines from table
* managemquery  : gets managem combines and specces (alone) from tables
* mgaquery      : gets amped managem specces and their amps from tables
* mgomniaquery  : gets a full package of amped spec and combine for both from tables

Not included in auto setup package:

* leechbuild    : builds leech table
* leechcombine  : makes leech combines from scratch
* managem_alone : makes managem combines and specces (alone) from scatch
* managembuild  : builds managem tables
* mgfast1build  : build fast managem combine tables
* mgfast2build  : build fast managem combine tables
* mgsloppybuild : build fast managem combine tables
* wmga          : outdated program that makes white managem recipes from scratch



