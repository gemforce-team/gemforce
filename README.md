gemforce
========

Gem combining program for GC2:CS - Written in C.


!!! ATTENTION !!!  
Due to a vicious bug killgem recipes are incorrect right now, they are being recomputed, please don't use them till you see this.  
!!! ATTENTION !!!

HOWTO use our programs:

compile:  
  gcc -O2 -lm file.c -o output  
run:  
  ./file -ptedi number  
where:  
  p - print best gem parens formula  
  t - print best gem tree formula  
  e - print best gems table (for each value best gem stats)  
  d - debug + info  
  i - info  
  l (num) - limit hard gem computation to num (amps only)  
  number - value of gem to be computed  
example:  
  ./file -pte 32  


Results:  
All resulting schemes (recipes) are placed in "results" folder.  

Name convention:  
  First part of name describes whether this is managem or killgem recipe.  
  Second part is:  
    comb - it's combine which should be used for both gem and its amps  
    spec - it's spec optimised for using no amplifiers - you probably don't want
           to use it  
    amps - it's pair of main gem spec and amps spec in optimal proportions, it's
           recommended to use :)  
  Third part - number(s):  
    numbers in the last part specify how many base gems go into specific recipe.
    Lower numbers are faster to perform, but higher gain bigger growth. It's
    recommended to use highest spec aviable (you do that only once) and highest
    comb you've got patience to perform (have in mind that you have to perform
    combines tens of times each game)  


Roadmap:

Orange combine - Done  
Orange/black spec - Done  
Orange/black combine - Done  
Yellow/black spec - Done  
Yellow/black combine - Done  
Amplifier-oriented OB - Done  
Amplifier-oriented YB - WIP  
OBR and YBR
