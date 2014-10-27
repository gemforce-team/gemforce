; AutoHotkey Version: 1.1x
; Language:       English
; Author:         8thSeaOfRhye
;
;                     #####         Parent2Gem.ahk - Version 2       #####
; 
;                     ##### Description of Purpose and Functionality #####
;
; ###            General             ###
; This script translates a String containing the Parenthesis-( i.e. Bracket-)Version of 
; a speccing or combining scheme for gemcraft into actual commands for the gems creation.
; Works best in a STANDALONE FLASHPLAYER set to FULLSCREEN.
; It is hopefully easy to use and customize.
; The section which has to be changed is almost right at the top of the following code,
; namely under the command "!C::", and is commented to indicate the exact usage.
; 
; The STANDARD HOTKEY is  Alt + c (No shift!!). This corresponds to "!C". 
; Check ahkscript.org for Hotkey usage.
;
; CombiningMode is turned on by default. Speccing still works. Suggestion: Don't turn it off... 
;
;
; ###  Syntax for the Bracket-Formula ###
;
; Obviously: 
; 		r = Red / b = Black / o = orange / y = yellow / w = white
;
; Maybe a little less obviously:
;
;		g / k = copy of gem in the BOTTOM-LEFT! corner of the 12*3 craftingfield
;
; Furthermore:   ### New Features ###
; 		
;		the numerals 2 to 9 followed by any of the above = Corresponding color (or duplicate, respectively), upgraded 1 to 8 times 
;		
;		Examples: 9o / 2g = Orange upgraded 8 times (grade 9) / Base-gem (BottomLeft corner) upgraded once
;		(Needless to say, the upgrading happens via the "u"-button.) 
;
; And: 			
;
;		a / s / d  =  use (left / middle / right) gem in the top row of the 3*12 craftingfield
;		
;		(The order is inspired by the position of the keys on a standard keyboard.
;		 Attention: Those three can't be used in combination with numerals.)


#NoEnv  ; Recommended for performance and compatibility with future AutoHotkey releases.
SendMode Input  ; Recommended for new scripts due to its superior speed and reliability.
SetWorkingDir %A_ScriptDir%  ; Ensures a consistent starting directory.
;
; 
;	
;


!C::
;             I
; #####      V V       																					 #########
; #####       V        INSERT THE SCHEME TO BE COMBINED INBETWEEN THE QUTATION-MARKS IN THE LINE BELOW   #########
ParentStr := "(((((((g+g)+(g+g))+((g+g)+(g+g)))+(((g+g)+(g+g))+((g+g)+(g+g))))+((((g+g)+(g+g))+((g+g)+(g+g)))+(((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g))))))))+(((((g+g)+(g+g))+((g+g)+(g+g)))+(((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g)))))))+((((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g))))))+(((g+g)+(g+(g+(g+(g+g)))))+((g+(g+(g+g)))+((g+(g+g))+((g+g)+((g+g)+(g+(g+(g+(g+g))))))))))))+((((((g+g)+(g+g))+((g+g)+(g+g)))+(((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g)))))))+((((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g))))))+(((g+g)+(g+(g+(g+(g+g)))))+((g+(g+(g+g)))+((g+(g+g))+((g+g)+((g+g)+(g+(g+(g+(g+g)))))))))))+(((((g+g)+(g+g))+((g+g)+(g+(g+(g+(g+g))))))+(((g+g)+(g+(g+(g+(g+g)))))+((g+(g+(g+g)))+((g+(g+g))+((g+g)+((g+g)+(g+(g+(g+(g+g))))))))))+((((g+g)+(g+(g+(g+(g+g)))))+((g+(g+(g+g)))+((g+(g+g))+((g+g)+((g+g)+(g+(g+(g+(g+g)))))))))+(((g+(g+(g+g)))+((g+(g+g))+((g+g)+(g+(g+(g+(g+g)))))))+(((g+(g+g))+((g+g)+(g+(g+(g+(g+(g+g)))))))+(((g+g)+((g+g)+(g+(g+(g+(g+g))))))+((g+(g+(g+(g+(g+g)))))+((g+(g+(g+g)))+((g+(g+g))+((g+(g+g))+((g+g)+(g+(g+(g+(g+(g+g)))))))))))))))))"

; #####   						The default example is some random 1024 combine							 #########

CombineMode := True
LastGemPos := 1

TopLeftCornerX := 1127 				;  ####### ENTER THE COORDINATES OF THE 3*12 CRAFTINGFIELD ON YOUR SCREEN #########
TopLeftCornerY := 175 				;	###### If you don't know how to obtain those values, you'd better  ########
BottomRightCornerX := 1234 			;    ##### ask somebody. They don't have to be really precise.     #######
BottomRightCornerY := 602 			;	  #### AND THAT'S IT! The script should work for you now!  ######

FieldWidth := (BottomRightCornerX - TopLeftCornerX) / 3
FieldHeight := (BottomRightCornerY - TopLeftCornerY) / 12
CraftingField := [BottomRightCornerX -(FieldWidth / 2), BottomRightCornerY -(FieldHeight / 2)]

SetDefaultMouseSpeed, 0
i := 0
j := 0
StkCount := 1
initPos := [TopLeftCornerX + (FieldWidth / 2), BottomRightCornerY - (FieldHeight * 1.5)]
StackFields := []
While i < 12 
{
	while j < 3 
	{
		StackFields[StkCount, 1] := initPos[1] + (j * FieldWidth) 
		StackFields[StkCount, 2] := initPos[2] - (i * FieldHeight)
		StkCount := StkCount + 1
		j := j + 1
	}
	j := 0
	i := i + 1
}	


; For FieldNum, FieldValue in StackFields
;	MsgBox % " Coordinates of Stack-Field #" . FieldNum .  ": " . FieldValue[1] . FieldValue[2]

CurrentStk := 0
WGem() {
	global CraftingField
	Send {Numpad9}
	MouseClick, left, CraftingField[1], CraftingField[2]
}
OGem() {
	global CraftingField
	Send {Numpad7}
	MouseClick, left, CraftingField[1], CraftingField[2]
}
YGem() {
	global CraftingField
	Send {Numpad8}
	MouseClick, left, CraftingField[1], CraftingField[2]
}
BGem() {
	global CraftingField
	Send {Numpad1}
	MouseClick, left, CraftingField[1], CraftingField[2]
}
RGem() {
	global CraftingField
	Send {Numpad4}
	MouseClick, left, CraftingField[1], CraftingField[2]
}

GGem() {
	global CraftingField
	global FieldWidth
	MouseMove, CraftingField[1]-(2*FieldWidth), CraftingField[2]
	send d
}

AddtoStk() {
	global CraftingField
	global CurrentStk
	global StackFields
	if CurrentStk != 0 
		{	
		Send g
		MouseClickDrag, L, CraftingField[1], CraftingField[2], StackFields[CurrentStk][1], StackFields[CurrentStk][2]
		}
}

MovetoStk() { 
	global CraftingField
	global CurrentStk
	global StackFields
	CurrentStk := CurrentStk + 1
	MouseClickDrag, L, CraftingField[1], CraftingField[2], StackFields[CurrentStk][1], StackFields[CurrentStk][2]
}


PopStk() {
	global CraftingField
	global CurrentStk
	global StackFields
	if CurrentStk > 1	
		{
		Send g
		MouseClickDrag, L, StackFields[CurrentStk][1], StackFields[CurrentStk][2], StackFields[CurrentStk - 1][1], StackFields[CurrentStk - 1][2]
		CurrentStk := CurrentStk - 1
		}
}

if (CombineMode = True) {
	StrPosEnd := StrLen(ParentStr)
	ElementFound := False
	While ElementFound = False {
		if (SubStr(ParentStr, StrPosEnd, 1) != ")") {
			ElementFound := True
			LastGemPos := StrPosEnd
		}
		StrPosEnd := StrPosEnd - 1
	}	
}


StrPos := 1
Parents := []

While StrPos < StrLen(ParentStr) {
	if (SubStr(ParentStr, StrPos, 1) = "(") {
		if ((SubStr(ParentStr, StrPos - 1, 1) = "(") or (StrPos = 1)) 
			Parents.Insert(False)
		else
			Parents.Insert(True)
	}

	if (SubStr(ParentStr, StrPos, 1) = ")") {
		if (Parents[Parents.MaxIndex()] = True)
			PopStk()
		Parents.remove(Parents.MaxIndex())
	}
	
	if (SubStr(ParentStr, StrPos, 1) = "w") {
		WGem()
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()
	}	
	
	if (SubStr(ParentStr, StrPos, 1) = "o") {
		OGem()
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()
	}
	
	if (SubStr(ParentStr, StrPos, 1) = "r") {
		RGem()
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	}
	
	if (SubStr(ParentStr, StrPos, 1) = "b") {
		BGem()
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	}
	
	if (SubStr(ParentStr, StrPos, 1) = "y") {
		YGem()
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	}
	
	if ((SubStr(ParentStr, StrPos, 1) = "g") or (SubStr(ParentStr, StrPos, 1) = "k") or (SubStr(ParentStr, StrPos, 1) = "m")) {
		if (LastGemPos != StrPos)
			GGem()
		else 
			MouseClickDrag, L, CraftingField[1]-(2*FieldWidth), CraftingField[2], CraftingField[1], CraftingField[2]
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()				
	}
	
	if ((SubStr(ParentStr, StrPos, 1) < 10) and (SubStr(ParentStr, StrPos, 1) > 1)) {
		if (SubStr(ParentStr, StrPos + 1, 1) = "w")
			WGem()
		if (SubStr(ParentStr, StrPos + 1, 1) = "o")
			OGem()
		if (SubStr(ParentStr, StrPos + 1, 1) = "y")
			YGem()	
		if (SubStr(ParentStr, StrPos + 1, 1) = "b")
			BGem()	
		if (SubStr(ParentStr, StrPos + 1, 1) = "r")
			RGem()	
		if (SubStr(ParentStr, StrPos + 1, 1) = "g") {
			if (LastGemPos != StrPos + 1) 
				GGem()		
			else 
				MouseClickDrag, L, CraftingField[1]-(2*FieldWidth), CraftingField[2], CraftingField[1], CraftingField[2]
		}
		UCount := 1
		While UCount <= SubStr(ParentStr, StrPos + 1, 1) {
			MouseMove, CraftingField[1], CraftingField[2]
			send u
			UCount := UCount + 1
		}
		
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
		StrPos := StrPos + 1
	}
	
	if (SubStr(ParentStr, StrPos, 1) = "a") {
		MouseClickDrag, L, TopLeftCornerX + (FieldWidth / 2), TopLeftCornerY + (FieldHeight / 2), CraftingField[1], CraftingField[2]
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	
	}
	if (SubStr(ParentStr, StrPos, 1) = "s") {
		MouseClickDrag, L, TopLeftCornerX + (FieldWidth * 1.5), TopLeftCornerY + (FieldHeight / 2), CraftingField[1], CraftingField[2]
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	
	}
	if (SubStr(ParentStr, StrPos, 1) = "d") {
		MouseClickDrag, L, TopLeftCornerX + (FieldWidth * 2.5), TopLeftCornerY + (FieldHeight / 2), CraftingField[1], CraftingField[2]
		if (SubStr(ParentStr, StrPos - 1, 1) = "(")
			MovetoStk()
		else
			AddtoStk()		
	
	}
	StrPos := StrPos + 1	
}
Return
