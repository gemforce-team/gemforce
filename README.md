gemforce
========

Gem combining program for GC2:CS - Written in C.


HOWTO use our programs:

compile:  
  gcc -O2 -lm file.c -o output  
run:  
  ./file -ptedi number  
where:  
  p - print best gem parens formula  
  e - print best gem equations formula  
  t - print best gem tree formula  
  c - print best gems table (for each value best gem stats)   
  d - debug + info  
  i - info  
  l (num) - limit hard gem computation to num (amps only)  
  s (num) - change chunk size (you should not need this)  
  number - value of gem to be computed  
example:  
  ./file -pet 32  


Results:  
All resulting schemes (recipes) are placed in "results" folder.  

Name convention:  
  First part of name describes what kind of recipe this is.  
  Second part is:  
    comb - it's combine which should be used for both gem and its amps  
    spec - it's spec optimised for using no amplifiers - you probably don't want
           to use it  
    amps - it's pair of main gem spec and amps spec in optimal proportions, it's
           recommended to use :)  
  Third part - number(s):  
    numbers in the last part specify how many base gems go into specific recipe.
    Lower numbers are faster to perform, but higher gain bigger growth. It's
    recommended to use highest spec available (you do that only once) and highest
    comb you've got patience to perform (have in mind that you have to perform
    combines tens of times each game)  


Roadmap:

Orange combine - Done  
Orange/black spec - Done  
Orange/black combine - Done  
Yellow/black spec - Done  
Yellow/black combine - Done  
Amplifier-oriented OB - Done  
Amplifier-oriented YB - Done  
OBR and YBR
