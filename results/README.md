## gemforce results

### Table of contents:

* [Results naming convention](#results-naming-convention)  
* [Parenthesis convention](#parenthesis-convention)  
* [Assumptions on skill levels](#assumptions-on-skill-levels)  
* **[What recipe should I use?](#what-recipe-should-i-use)**  
* [Performing recipes manually is boring!](#performing-recipes-manually-is-boring)  
* [Other stuff](#other-stuff) 

### Results naming convention

The results are divided in folders for each gem type (leech/managem/killgem) and kind (combine/amps/omnia).  
Leech gems are only used as amplifiers, the leech folder is needed only for testing purpose.  
Recipe names are composed by a gem type (as above), a recipe kind and number(s) for the recipe length.

The recipe kind can be:

* `combine` - a combine which should be used with an already built gem  
* `amps`    - a pair of main gem spec and amps spec in optimal proportions,
              though they don't take the combine into consideration  
* `omnia`   - a set of four recipes: a main gem spec and its amps spec in optimal proportions
              and a pair of optimal combines for both, it's the pinnacle of gem "rocket science"  

The number(s) in the specify how many base gems go into specific recipe:  

* combine:    `N` - the combine costs `N` base gems, as easy as it gets  
* amps:     `G-A` - the main gem costs `G` gems while its amps cost `A` each  
* omnia: `G-A--C` - the main gem costs `G` gems, the optimized amps cost `A` each
                    and both the combines require `C` copies of the main gem/amplifier

Lastly an `u` at the end of the name means that that recipe is the best up to the written number and it's likely shorter than indicated.

All the amped recipes here are built with 6 amps for managems and 8 amps for killgems.

I try to have here all the `2^n` recipes and the best between every `2^n`and `2^(n+1)`
for the `amps` and `comb` sections.  
There are then selected ones from the `omnia` kind, as having all the combinations would be impossible.


### Parenthesis convention

In most parenthesis schemes you'll find gems written as "3o" or "4k", etc.  
This simply mean that the gem is the basic gem upgraded the written number of times less one:
4o = 3o+3o = (2o+2o)+(2o+2o) = ((o+o)+(o+o))+((o+o)+(o+o)) which is the base gem upgraded 3 times.


### Assumptions on skill levels

gemforce needs minimal knowledge of the user skill levels.  
The programs that deal with a gem alone give valid output regardless of skill levels.  
When dealing with amps (`query-ngems`, `query-amps`, `query-setup`,`query-omnia`) two values are needed:

* Amplifiers: assumed at `45+15` - it matters quite a lot  
* True Colors: assumed at `120` - it matters nearly nothing

If you want more precise output those programs accept your True Colors level with the `-T` flag.


### What recipe should I use?

What to choose depends mostly on your grade of foolishness (and on your free time),
keep in mind a spec has to be done just one time, while a combine might need to be repeated tens of times.  
I'll outline 5 different choices here, from the easiest to the most time consuming:

**0. No supergemming**  
Well, why are you reading this then? Good luck with your crappy specces and 'U' upgrade

**1. Beginner**  
Combines give the most boost over time, more than specces, so the most effective way to boost your power
with a small time investment is to use a small combine instead of 'U' for every upgrade.  
Go to the managem and the killgem folders and find combines simple enough
(anything below `16c` should be good) to be doable fast.  
To choose the ratio of gems to put in the main gem and the amps stick to the thumb rule:  

* managems: amps 3 grades lower  
* killgems: amps 4 grades lower

**2. Intermediate**  
It's time to look into specces to match the combines.
Head to the managem and the killgem folders and choose a pair of small specces,
around `16s` or `32s`.  
Pair them with a combine from the same folder, to be used both with the main gem and its amps.
When using a single combine on both gem and its amps it can be proved that
the best choice is to use the one for the gem.
Using different combines on both yield too little benefit for now.  
The initial omnia recipes are good here, since they usually have the same combine for gem and amps.

**3. Advanced**  
Keep on with the intermediate way by improving the lengths of the spec/combine.
`32s` - `128s` and `64c`-`4096c` are the usual choices of people in this range.  
A little more power can be gained by "squeezing" the red out of the specces,
done by combining a specced gem with red with other copies without red on the first combine cycle.  
To get a red-less recipe from a red one change the red gem with a orange/yellow one.

**4. Ubermensch**  
Head to the high-level omnia recipes.  
While the initial omnia recipes have the same combine both for gems and amps,
the higher ones have vastly different recipes, which results in an extremely time consuming method.
In this range there are people using `2048s/16384c` or even more.
There are also like 5 people I know of in this range.  
Recommended for bots or people with extreme patience.


### Performing recipes manually is boring!
I agree with you.  
That's why we built a bot that does the ugly work for us.  
You can find it here: [wGemCombiner repo](https://github.com/gemforce-team/wGemCombiner)


### Other stuff

If you need a particular recipe you don't find here you can build it yourself using the programs.  
Check the [main README of the repository](https://github.com/gemforce-team/gemforce/#readme) for a comprehensive HOWTO.

If this project helped you and you wish to help by contributing, please contact us, leaving a
[new issue](https://github.com/gemforce-team/gemforce/issues/new) or opening a new pull request.
You can also help by donating some money for our time:  
[![PayPayl donate button](https://img.shields.io/badge/paypal-donate-yellow.svg)](https://www.paypal.com/cgi-bin/webscr?cmd=_s-xclick&hosted_button_id=LY6RG34S5UCTW "Donate to this project using Paypal")

For anything else, visit the [main README of the repository](https://github.com/gemforce-team/gemforce/#readme)
or leave a [Github issue](https://github.com/gemforce-team/gemforce/issues/new).
