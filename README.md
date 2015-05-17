gemforce
========

Gem combining program for [Gemcraft 2: Chasing Shadows](http://gameinabottle.com/).


### Table of contents:

* **[If you just want recipes click here](results/#readme)**
* **[The combiner bot (wGemCombiner) has moved here](https://github.com/gemforce-team/wGemCombiner/#readme)**
* [Overview](#overview)  
* [Prebuilt recipes](#prebuilt-recipes)  
* [Get gemforce](#get-gemforce)  
* [Precomputed gem tables](#precomputed-gem-tables)  
* [HOWTO use our programs](#howto-use-our-programs)  
* [Name conventions](#name-conventions)  
* [Why are those weird recipes better than 'U'?](#why-are-those-weird-recipes-better-than-u)  
* [What does each program do?](#what-does-each-program-do)  
* [Report bugs](#report-bugs)
* [Credits](#credits)  


### Overview:

gemforce project's goal is to find the optimal recipes for gems used in endgame play of GC2.  
To do so we developed a series of programs, each one computes some specific recipe type.

For an explanation of some of the name conventions used see [Name conventions](#name-conventions) section below.  
For a brief introduction of non-standard combining see
[Why are those weird recipes better than 'U'?](#why-are-those-weird-recipes-better-than-u) section below  
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
    `gcc -O3 -lm -I "include" file.c -o output`  
    You'll then need to copy the gem tables to the directory you built the executables in.

* **Windows (tested and working from XP to 8.1):**
  
  * **Binary download**  
  Every major change to gemforce a new release version is drafted and a set of binaries for Windows is rebuilt,
  which contains the same programs the automatic setup has.  
  You can find them in the [Release page](https://github.com/gemforce-team/gemforce/releases)
  
  * **manual compilation:**  
  Works on Windows too, but it's a bit more difficult to get hold of a compiler there.  
  Any modern C compiler is ok, remember to include the `include` directory, as the example below:  
  `somecompiler -O3 -lm -I "include" file.c -o output`


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

* `h` - print an help text  
* `p` - print best gem parenthesis formula  
* `t` - print best gem tree formula  
* `e` - print best gem equations formula  
* `c` - print best gems table (for each value best gem power)  
* `i` - info  
* `q` - quiet, print only final gem  
* `u` - upto, at the end consider the best gem up to N, not the Nth  
* `r` - adds red to the best gem, then consider the new gem  
* `f "path/to/file"` - change table file (build/query only)

`number` - value of the gem to be computed (mandatory)  

To specify if you want a speccing or a combining recipe out of a `*query-alone` program:  
if `number` is `100`, then:
* `./mgquery-alone -[flags] 100s` or `./mgquery-alone -[flags] 100` will give a speccing recipe
* `./mgquery-alone -[flags] 100c` will give a combining recipe.

The `query-ngems` and `query-amps` programs support some additional flags and an extended `-f` pattern:

* `f "path_spec,path_amps"` - change table files, if any is empty it'll use the default path
* `N number` - specify how many amps you want to use (default: 6/8) `[0-8]`  
* `T number` - give your True Colors skill for better amps fitting `(default: 120) [0-999]`

The `query-omnia` and `query-setup` programs support an extra flag and a second number:  
`./*query-omnia -[flags] number1 number2`

* `f "path_spec,path_comb,path_amps"` - change table files, if any is empty it'll use the default path
* `G number` - get the final power at a given equivalent grade (default: `30`) `[1-999]`  

`number1` - speccing value (mandatory)  
`number2` - combine value (can be omitted, default: `16`)

Debug flags/options (you should not need these):

* `d` - prints debug text, depending on program  
* `g number` - for `query-amps` give custom growth value (default: `16c` for that gem)  
* `l number` - for `query-ngems` give custom hard gem limit (default: minimum between len and table len)

**examples:**  
`./file -pet 32`  
`./filequery-alone -ipf "path/to/table" 128c`  
`./filequery-omnia -pq 32 16`  

When in doubt about which flags are supported check with `-h`.


### Name conventions:

* Variables:
  * Value : Mana cost of a recipe, expressed in base gems needed
  * Mana power : Displayed leech, that is leech*bloodbound
  * Kill power : Displayed damage * displayed crit, that is damage\*bloodbound\*crit\*bloodbound
  * Growth : log(Power)/log(recipe_length), an important quantity in recipe ranking
  * Spec coeff : N_setup^(growth_spec-growth_comb), an important quantity in (amped) spec ranking

* Gems:
  * Managem : Orange-Black-Red gem, optimized for mana gain
  * Killgem : Yellow-Black-Red gem, optimized for kill power
  * Mana setup : Orange-Black-Red gem and 6x Orange amps, optimized for mana gain
  * Kill setup : Yellow-Black-Red gem and 6-8x Yellow amps, optimized for kill power
  
* Recipes:
  * Spec: Initial recipe performed on the base pure gems to make a killgem or managem
  * Combine: Recipe used to upgrade an already built gem
  * Weaving: Outdated method from GCL, abandoned in favor of spec/combine
  * Supergemming: Blanket term, used to indicate all of non-standard combing
  
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

### Why are those weird recipes better than 'U'?

There is nothing profound about those recipes (at least up to my understanding), they just happen to be better than the others.

The reason we can do better recipes that 'U' is in the way the game computes stats for a combined gems:
when two gems are combined the resulting gem stats are always in this form:  
`Combined.Stat = c1 * max(Gem1.Stat, Gem2.Stat) + c2 * min(Gem1.Stat, Gem2.Stat)`  
where `c1` and `c2` are coefficients that change between colors and with the difference in grade between the two combined gems.
The complete list can be found in the [GC2_formulae gist](https://gist.github.com/12345ieee/4a81c78d4426a99c4bc3)  

With this list it is possible to try all the combinations that give a, say, leech gem of value 8
and find the one that has the best leech (an useful gem for amplifier).  
Because of how these combining coefficients were chosen (by the game author) the best gem happens to be `(((((2o+o)+o)+o)+o)+2o)`
instead of the standard `4o`.

This reasoning can be extended to longer recipes and to ones with 2-3 different base g1.
It's possible to find recipes that outperform standard 'U' upgrading by tens of times in the end.  
This is usually done at the expense of other stats that we are not interested in:
leech combines are usually terrible for firerate and range, but neither matters for a gem at the speed cap in an amplifier,
which is the typical use case after a certain level.

**For the math enthusiasts (yes, you may want to skip the rest):**  
At the core everything is based on the fact that the growth of `2g+g` is usually higher than the one of `2g`.
There are exceptions, like cyan, for which a better upgrade than 'U' has yet to be discovered, and likely does not exist.

Another important point is the multiplicative property of gem combination, which assures us a given combine method
always multiply the stats by some number dependent only on the method and not on the base stats, which allows us
to find a method once and to apply it successfully lots of time on the same gem with the same increase.

A last point is that the quantity we optimize for are usually products of native color powers (like kill_power=bb\*bb\*dmg\*crit),
that coupled with the property above allows our combine method to remain optimal independently from the stats of the gem it's applied to.

When this 2nd property fails (white, blue and red power aren't directly related to the base color power
but there are logarithms in the mix) the problem becomes nearly impossible to solve with a certain generality,
which is why I offer no white kill/managems combiner (aside the unstable one on the `white` branch).


### What does each program do?

Mana and kill suites are alike, so I'll just describe the mana one:

Included in auto setup package:

* leechquery    : gets leech combines from table
* mgquery-alone : gets managem combines and specces (alone) from tables
* mgquery-amps  : gets amped managem specces and their amps from tables - fixed spec number version
* mgquery-setup : basically `alone (combines) + amps`,
offers more info on the whole spec-combine than the two used separately
* mgquery-omnia : gets amped managem specces, their amps and combines for both from tables

Not included in auto setup package:

* leechbuild      : builds leech table
* leechcombine    : makes leech combines from scratch
* mgcombine-alone : makes managem combines and specces (alone) from scratch
* mgbuild-appr    : builds managem tables
* mgbuild-exact   : builds the exact managem tables
* mgbuild-[...]   : builds a fast managem combine table in various versions
* mgquery-ngems   : gets amped managem specces and their amps from tables - fixed total number version


### Report bugs:

To report a bug with gemforce look at the
[Issues page on Github](https://github.com/gemforce-team/gemforce/issues) if it was already reported.  
If your bug isn't there open a [new issue](https://github.com/gemforce-team/gemforce/issues/new).
Be sure to include all the relevant information (OS, gemforce version, etc...) and give a reliable way to reproduce the bug.


### Credits:

gemforce authors are:

* Andrea Stacchiotti aka Steam user '12345ieee' aka AG user '12345ieee'  
* Wojciech Jabłoński aka Steam user 'psorek139' aka AG user 'psorek'

Current maintainer:

* Andrea Stacchiotti aka Steam user '12345ieee' aka AG user '12345ieee' 

If this project helped you and you wish to help by contributing, please contact us, leaving a
[new issue](https://github.com/gemforce-team/gemforce/issues/new) or opening a new pull request.
You can also help by donating some money for our time:  
[![PayPayl donate button](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LY6RG34S5UCTW "Donate to this project using Paypal")

Gemcraft 2: Chasing Shadows was developed by [gameinabottle](http://gameinabottle.com/)

