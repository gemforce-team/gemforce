
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
; 		r = Red / b = Black / o = orange / y = yellow
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
InitialMode := False ; Initialisation
;CoordMode, Mouse, Screen
CurrentCustom := "(g+g)+(g+g)"
LastDDL := 5
;             I
; #####      V V       																					 #########
; #####       V        INSERT THE SCHEME TO BE COMBINED INBETWEEN THE QUTATION-MARKS IN THE LINE BELOW   #########
ParentStr := "(g+g)+(g+g)"

; #####   						The default example is some random 1024 combine							 #########


!V:: 
	OptionsMenu()
return


OptionsMenu() {
	Global ParentStr
	Global ChosenCombine
	Global ParentNewStr
	Global ChosenIniMode
	Global IsIniChecked
	Global InitialMode
	Global CustomFormula
	Global PGScript
	Global CustomTemp
	Global LastDDL
	Global CurrentCustom
	Global LastDDLTemp
	Global TempReadOnly
	
	Gui, New,, Options
	GuiHWND := WinExist()

; ################# DDL ######################	
	TempReadOnly := ""
	If LastDDL != 5
		TempReadOnly := "readonly"
	Gui, Add, Text,, Choose your desired combine
	Gui, Add, DropDownList, w200 Choose%LastDDL% vChosenCombine gShowCustomField, killgem_amps64-5-gem|killgem_amps64-5-amps|managem_amps256-30_ieee-gem|managem_amps256-30_ieee-amps|universal16c|Custom
	Gui, Add, Text,, Custom Bracket Formula:        Attention! Syntaxerrors may lead to chaos!
	Gui, Add, Edit, w400 r3 vCustomFormula %TempReadOnly%, %CurrentCustom% 


; ################# INITMODE ################	

	Gui, Add, Text,, `n`n In order to reset the coordinates check the following option`: `n
	If (InitialMode = False)
		IsIniChecked := ""
	If (InitialMode = True)
		IsIniChecked := "Checked"
	Gui, Add, Checkbox, vChosenIniMode %IsIniChecked%, Initialisation-Mode 	
	Gui, Add, Text,, If checked, you will be asked for coordinates the next time the CombiningHotkey is pressed.`n CombiningHotkey is Alt C by default. 

;###############Wait For Input ##############

	Gui, Add, Button, default, Ok 
	Gui, Show
	WinWaitClose, ahk_id %GuiHWND%
	return                

;############### SUBS / Labels #############	

	ShowCustomField: 
		GuiControlGet, StatusOfDll, , ChosenCombine
		if (StatusOfDll = "custom")
			GuiControl, -readonly, CustomFormula
		else
			GuiControl, +readonly, CustomFormula
	return
	
	ButtonOk:
		Gui, Submit
		if (ChosenCombine = "killgem_amps64-5-gem") {
			ParentNewStr := "((((b+((b+y)+(y+y)))+((b+(b+y))+(b+(b+(b+b)))))+(((b+b)+(b+(y+y)))+(((b+y)+(y+y))+((y+y)+(y+(y+(y+(y+y))))))))+((((b+(b+y))+(b+(y+y)))+((b+(b+y))+(b+(b+(b+(b+b))))))+(((b+(b+y))+(b+(b+(b+b))))+((b+(b+(b+b)))+((b+(b+b))+((b+(b+b))+(b+(b+(b+(b+b))))))))))"
			LastDDL := 1
		}	
		if (ChosenCombine = "killgem_amps64-5-amps") {
			ParentNewStr := "((y+y)+(y+(y+y)))"
			LastDDL := 2
		}	
			
		if (ChosenCombine = "managem_amps256-30_ieee-gem") {
			ParentNewStr := "(((((((b+o)+(b+b))+(b+((o+o)+(b+o))))+(((b+o)+(b+b))+((b+b)+(b+(b+(b+(b+b)))))))+((((b+o)+(b+b))+(b+((o+o)+(o+(o+o)))))+((b+((o+o)+(b+o)))+(((o+o)+(b+o))+((o+o)+(o+(o+(o+(o+(o+(o+o)))))))))))+(((((b+o)+(b+b))+(b+((o+o)+(o+(o+o)))))+(((b+o)+(b+b))+((b+b)+(b+(b+(b+(b+b)))))))+((((b+o)+(b+b))+((b+b)+(b+(b+(b+(b+b))))))+(((b+b)+(b+(b+(b+(b+b)))))+((b+(b+(b+(b+b))))+((b+(b+(b+b)))+((b+(b+b))+((b+b)+(b+(b+(b+(b+(b+b)))))))))))))+((((((b+o)+(b+b))+(b+((b+o)+(o+(o+o)))))+(((b+o)+(b+b))+((b+b)+(b+(b+(b+(b+b)))))))+((((b+o)+(b+b))+(b+((b+o)+(o+(o+o)))))+((b+((o+o)+(b+o)))+(((o+o)+(b+o))+((o+(o+o))+(o+(o+(o+(o+(o+(o+o)))))))))))+(((((b+o)+(b+b))+(b+((b+o)+(o+(o+o)))))+((b+((o+o)+(b+o)))+(((b+o)+(o+(o+o)))+((o+(o+o))+(o+(o+(o+(o+(o+(o+o))))))))))+(((b+((o+o)+(b+o)))+((b+(o+o))+((o+o)+(o+(o+(o+(o+(o+o))))))))+((((o+o)+(b+o))+((o+(o+o))+(o+(o+(o+(o+(o+(o+o))))))))+(((o+o)+(o+(o+(o+(o+(o+o))))))+(((o+o)+(o+(o+(o+(o+(o+o))))))+((o+(o+(o+(o+o))))+((o+(o+(o+o)))+((o+(o+o))+((o+(o+o))+(o+(o+(o+(o+(o+(o+(o+o))))))))))))))))))"
			LastDDL := 3
		}		
	
		if (ChosenCombine = "managem_amps256-30_ieee-amps") {
			ParentNewStr := "((((o+o)+(o+o))+((o+o)+(o+(o+(o+o)))))+(((o+o)+(o+(o+(o+o))))+((o+(o+o))+((o+(o+o))+(o+(o+(o+(o+(o+(o+(o+o)))))))))))"			
			LastDDL := 4
		}	

		if (ChosenCombine = "universal16c") {
			ParentNewStr := "(((((((((g+g)+g)+g)+g)+g)+g)+(g+g))+(g+g))+(((g+g)+g)+(g+g)))"
			LastDDL := 5
		}	

		if (ChosenCombine = "Custom") {
			ParentNewStr := CustomFormula			
			LastDDL := 6
		}
			
;		if (ChosenCombine = "") {
;			ParentNewStr := ""
;			LastDDL := 1
;		}	
			
		if (ChosenCombine = "")
			ParentNewStr := ParentStr		
	
		FileRead, PGScript, Parent2Gem.ahk
		FileDelete, Parent2Gem.ahk
		ParentNewStr = ParentStr := "%ParentNewStr%" 
		PGScript := RegExReplace(PGScript, "ParentStr := ""[A-Za-z0-9()+]*""", ParentNewStr, , 1)  
		
		CustomTemp = CurrentCustom := "%CustomFormula%"
		if (ChosenCombine = "Custom") 
			PGScript := RegExReplace(PGScript, "CurrentCustom := "".*""", CustomTemp, , 1)		
		
		LastDDLTemp = LastDDL := %LastDDL%
		PGScript := RegExReplace(PGScript, "LastDDL := [0-9]*", LastDDLTemp, , 1)	
		
		if ((ChosenIniMode = 0) and (InitialMode = True))
			PGScript := RegExReplace(PGScript, "InitialMode := True  `; Init", "InitialMode := False `; Init", , 1)
		if ((ChosenIniMode = 1) and (InitialMode = False))
			PGScript := RegExReplace(PGScript, "InitialMode := False `; Init", "InitialMode := True  `; Init", , 1)	
	
		FileAppend, %PGScript%, Parent2Gem.ahk		
		Gui, Destroy
	Reload
}
!C::


CombineMode := True
LastGemPos := 1

TopLeftCornerX := 1590 				;  ####### ENTER THE COORDINATES OF THE 3*12 CRAFTINGFIELD ON YOUR SCREEN #########
TopLeftCornerY := 338				;	###### If you don't know how to obtain those values, you'd better  ########
BottomRightCornerX := 1436			;    ##### ask somebody. They don't have to be really precise.     #######
BottomRightCornerY := 686			;	  #### AND THAT'S IT! The script should work for you now!  ######

if (InitialMode = True) {
	FileRead, PGScript, Parent2Gem.ahk
	FileDelete, Parent2Gem.ahk
	MsgBox Place your mouse over the top-left corner of the 12*3 craftingfield and press ENTER 
		MouseGetPos, TLCX, TLCY
		PGScript := RegExReplace(PGScript, "TopLeftCornerX := [0-9]+", "TopLeftCornerX := " TLCX, , 1)  
		PGScript := RegExReplace(PGScript, "TopLeftCornerY := [0-9]+", "TopLeftCornerY := " TLCY, , 1)  	
	MsgBox Place your mouse over the bottom-right corner of the 12*3 craftingfield and press ENTER
		MouseGetPos, BRCX, BRCY
		PGScript := RegExReplace(PGScript, "BottomRightCornerX := [0-9]+", "BottomRightCornerX := "BRCX, , 1)  
		PGScript := RegExReplace(PGScript, "BottomRightCornerY := [0-9]+", "BottomRightCornerY := "BRCY, , 1)
		PGScript := RegExReplace(PGScript, "InitialMode := True  `; Init", "InitialMode := False `; Init", , 1)
	FileAppend, %PGScript%, Parent2Gem.ahk
	
	MsgBox The Script has been initialised and will be reloaded!
	
	Reload
}
	else {
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
	WGem() {
		global CraftingField
		Send {Numpad9}
		MouseClick, left, CraftingField[1], CraftingField[2]	
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
		if (SubStr(ParentStr, StrPos, 1) = "w") {
			WGem()
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
			if (SubStr(ParentStr, StrPos + 1, 1) = "o")
				OGem()
			if (SubStr(ParentStr, StrPos + 1, 1) = "y")
				YGem()	
			if (SubStr(ParentStr, StrPos + 1, 1) = "b")
				BGem()	
			if (SubStr(ParentStr, StrPos + 1, 1) = "r")
				RGem()	
			if (SubStr(ParentStr, StrPos + 1, 1) = "r")
				WGem()				
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
	while CurrentStk > 1 {
		PopStk()
	}
}


Return
