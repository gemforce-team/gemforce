; Shows Mouse-Coordinates upon pressing Alt+x
!x::
MouseGetPos, xpos, ypos
Msgbox, The cursor is at X%xpos% Y%ypos%.
return
