parent2gem
========

Script to combine gems automatically - Written in AHK.

Features are documented below the wishlist, but documentation is currently out of date.

WISHLIST:

0) I still cannot get it to work consistently, maybe needs some tweaking.  
1) Speed toggle, so that it's possible to use it on slower setups.


Documentation
====

This script translates a String containing the Parenthesis-( i.e. Bracket-)Version of 
 a speccing or combining scheme for gemcraft into actual commands for the gems creation.
 Works best in a STANDALONE FLASHPLAYER set to FULLSCREEN.
 It is hopefully easy to use and customize.
 The section which has to be changed is almost right at the top of the following code,
 namely under the command "!C::", and is commented to indicate the exact usage.
 
 The STANDARD HOTKEY is  Alt + c (No shift!!). This corresponds to "!C". 
 Check ahkscript.org for Hotkey usage.

 CombiningMode is turned on by default. Speccing still works. Suggestion: Don't turn it off... 


 ###  Syntax for the Bracket-Formula ###

 Obviously: 
 		r = Red / b = Black / o = orange / y = yellow

 Maybe a little less obvious:

		g / k = copy of gem in the BOTTOM-LEFT! corner of the 12*3 craftingfield

 Furthermore:   ### New Features ###
 		
		the numerals 2 to 9 followed by any of the above = Corresponding color (or duplicate, respectively), upgraded 1 to 8 times 
		
		Examples: 9o / 2g = Orange upgraded 8 times (grade 9) / Base-gem (BottomLeft corner) upgraded once
		(Needless to say, the upgrading happens via the "u"-button.) 

 And: 			

		a / s / d  =  use (left / middle / right) gem in the top row of the 3*12 craftingfield
		
		(The order is inspired by the position of the keys on a standard keyboard.
		 Attention: Those three can't be used in combination with numerals.)

