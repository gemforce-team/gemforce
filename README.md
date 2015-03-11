gemforce
========

Gem combining program for [Gemcraft 2: Chasing Shadows](http://gameinabottle.com/).


### Table of contents:

* **[If you just want recipes click here](results/#readme)**
* [Overview](#overview)  
* [Prebuilt recipes](#prebuilt-recipes)  
* [Get gemforce](#get-gemforce)  
* [Precomputed gem tables](#precomputed-gem-tables)  
* [HOWTO use our programs](#howto-use-our-programs)  
* [Name conventions](#name-conventions)  
* [What does each program do?](#what-does-each-program-do)  
* [Miscellaneous](#miscellaneous)  


### Overview:

gemforce project's goal is to find the optimal recipes for gems used in endgame play of GC2.  
To do so we developed a series of programs, each one computes some specific recipe type.

For an explanation of some of the name conventions used see [Name conventions](#name-conventions) section below.  
The full programs list can be found in the [What does each program do?](#what-does-each-program-do) section below.


### Prebuilt recipes:

If you are not interested in running the programs yourself, but you just want to know what the best recipes are
you can find lots of precomputed recipes in the [results folder](results).

More info on them can be found in the [results folder's README](results/#readme).


### Get gemforce:

If you need a recipe you can't find between the prebuilt ones or you are interested on running the programs yourself
for some other reason you need to get a working version of gemforce.

How to do this greatly depends on the operative system you are using:

* **Linux (and Mac OS, I suppose):**

  Download the source code (either by cloning the repository or using the ZIP download function)
  and then use one of the options below:

  * **automatic setup (*NIX shell required):**  
    In the repo root run `./auto_setup.sh`  
    everything you typically need will be placed in `bin/`
    
  * **manual compilation:**  
    `gcc -O2 -lm -I "include" file.c -o output`  
    You'll then need to copy the gem tables to the directory you built the executables in.

* **Windows (tested and working from XP to 8.1):**
  
  * **Binary download**  
  Every major change to gemforce a new release version is drafted and a set of binaries for Windows is rebuilt,
  which contains the same programs the automatic setup has.  
  You can find them in the [Release page](https://github.com/gemforce-team/gemforce/releases)
  
  * **manual compilation:**  
  Works on Windows too, but it's a bit more difficult to get hold of a compiler there.  
  Any modern C compiler is ok, remember to include the `include` directory, as the example below:  
  `gcc -O2 -lm -I "include" file.c -o output`


### Precomputed gem tables:
The tables allow you to retrieve results already computed without having the programs recompute them every time you need them.  
They are a necessity for the *query programs and must be placed in the same directory as them.

Precomputed tables up to 1 MB are placed in the [gem_tables folder](gem_tables).  
Those tables are big enough to have the recipes up to decent length
while allowing the whole repository to stay under a reasonable size.

For more information read the [gem_tables folder's README](gem_tables/#readme).


### HOWTO use our programs:

The programs need to be run from a terminal emulator, such as:

* `gnome-terminal` or `konsole` on most Linux distributions
* `cmd` or `powershell` on Windows

just to cite some of the most common.

Change the directory to the one you have the executables in and then run the programs with:    
`./program_name -[flags] number`

where the flags are:

* `p` - print best gem parenthesis formula  
* `t` - print best gem tree formula  
* `e` - print best gem equations formula  
* `c` - print best gems table (for each value best gem power) **do not use with -r or -u**  
* `i` - info  
* `q` - quiet, print only final gem  
* `u` - upto, at the end consider the best gem up to N, not the Nth  
* `r` - adds red to the best gem, then consider the new gem  
* `f "path/to/file"` - change table file (build/query only)

`number` - value of the gem to be computed (mandatory)  

To specify if you want a speccing or a combining recipe out of a general program (e.g. `managemquery`):  
if `number` is `100`, then:
* `./managemquery -[flags] 100s` or `./managemquery -[flags] 100` will give a speccing recipe
* `./managemquery -[flags] 100c` will give a combining recipe.

In addition the `managem_omnia` and `killgem_omnia` programs support some additional flags and a second number:
`./somegem_omnia -[flags] number1 number2`

* `f "path_spec,path_comb,path_amps"` - change table files, if any is empty it'll use the default path
* `N number` - specify how many amps you want to use `[0-8]`  
* `T number` - give your True Colors skill for better amps fitting `[0-999]`  

`number1` - speccing value (mandatory)  
`number2` - combine value (can be omitted, default is `16`)

Debug flags/options (you should not need these):

* `s number` - change chunk size (build/force only) 
* global mode in `(m/k)gaquery` is toggled via `(num)g`  

**examples:**  
`./file -pet 32`  
`./filequery -ipf "path/to/file" 128c`  
`./file_omnia -pq 32 16`  


### Name conventions:

* Variables:
  * Value : Mana cost of a recipe, expressed in base gems needed
  * Mana power : Displayed leech, that is leech*bloodbound
  * Kill power : Displayed damage * displayed crit, that is damage\*bloodbound\*crit\*bloodbound
  * Growth : log(Power)/log(recipe_length), an important quantity in recipe ranking

* Gems:
  * Managem : Orange-Black-Red gem, optimized for mana gain
  * Killgem : Yellow-Black-Red gem, optimized for kill power
  * Mana setup : Orange-Black-Red gem and 6x Orange amps, optimized for mana gain
  * Kill setup : Yellow-Black-Red gem and 6-8x Yellow amps, optimized for kill power
  
* Recipes:
  * Spec: Initial recipe performed on the base pure gems to make a killgem or managem
  * Combine: Recipe used to upgrade an already built gem
  
* Colors:
  * `o` : Orange
  * `y` : Yellow
  * `b` : Black
  * `r` : Red
  * `m` : Managem
  * `k` : Killgem  
  ` `
  * `w` : White
  * `g` : Generic gem
  * `s` : Cyan (Suppressive)
  * `h` : Cyan/Black


### What does each program do?

Mana and kill suites are alike, so I'll just describe the mana one:

Included in auto setup package:

* leechquery    : gets leech combines from table
* managemquery  : gets managem combines and specces (alone) from tables
* mgaquery      : gets amped managem specces and their amps from tables
* mgomniaquery  : gets a full package of amped spec and combine for both from tables

Not included in auto setup package:

* leechbuild    : builds leech table
* leechcombine  : makes leech combines from scratch
* managem_alone : makes managem combines and specces (alone) from scratch
* managembuild  : builds managem tables
* mgfast1build  : builds a fast managem combine table
* mgfast2build  : builds a fast managem combine table
* mgsloppybuild : builds a fast managem combine table


### Miscellaneous:

gemforce authors are:
* Andrea Stacchiotti aka AG user '12345ieee' 
* Wojciech Jabłoński aka AG user 'psorek'

Gemcraft 2: Chasing Shadows was developed by [gameinabottle](http://gameinabottle.com/)

