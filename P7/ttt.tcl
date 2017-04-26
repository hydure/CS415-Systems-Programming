package require Tk

canvas .c -width 300 -height 400 -bg white
pack .c
set callBell 0

# Create rectangles for the board
.c create rectangle   0   0 100 100 -width 5 -tags rec1 -fill {} -outline {}
.c create rectangle 100   0 200 100 -width 5 -tags rec2 -fill {} -outline {}
.c create rectangle 200   0 301 100 -width 5 -tags rec3 -fill {} -outline {}
.c create rectangle   0 100 100 200 -width 5 -tags rec4 -fill {} -outline {}
.c create rectangle 100 100 200 200 -width 5 -tags rec5 -fill {} -outline {}
.c create rectangle 200 100 301 200 -width 5 -tags rec6 -fill {} -outline {}
.c create rectangle   0 200 100 301 -width 5 -tags rec7 -fill {} -outline {}
.c create rectangle 100 200 200 301 -width 5 -tags rec8 -fill {} -outline {}
.c create rectangle 200 200 301 301 -width 5 -tags rec9 -fill {} -outline {}

# Put textboxes on top of each rectangle
.c create text  50  50 -font {Times -10 bold} -tags text1
.c create text 150  50 -font {Times -10 bold} -tags text2
.c create text 250  50 -font {Times -10 bold} -tags text3
.c create text  50 150 -font {Times -10 bold} -tags text4
.c create text 150 150 -font {Times -10 bold} -tags text5
.c create text 250 150 -font {Times -10 bold} -tags text6
.c create text  50 250 -font {Times -10 bold} -tags text7
.c create text 150 250 -font {Times -10 bold} -tags text8
.c create text 250 250 -font {Times -10 bold} -tags text9

# Bind X's and O's to tiles upon button click
.c bind rec1 <Button-1> { puts stdout "1"}
.c bind rec2 <Button-1> { puts stdout "2"}
.c bind rec3 <Button-1> { puts stdout "3"}
.c bind rec4 <Button-1> {puts stdout "4"}
.c bind rec5 <Button-1> { puts stdout "5"}
.c bind rec6 <Button-1> { puts stdout "6"}
.c bind rec7 <Button-1> { puts stdout "7"}
.c bind rec8 <Button-1> { puts stdout "8"}
.c bind rec9 <Button-1> { puts stdout "9"}

# Create board's lines
.c create line 100   0 100 300 -tag bar1 -width 5
.c create line 200   0 200 300 -tag bar1 -width 5
.c create line   0 100 300 100 -tag bar1 -width 5
.c create line   0 200 300 200 -tag bar1 -width 5

.c create rectangle 1 300 300 400 -fill grey -outline black
.c create text  10 330 -anchor w -text " " -font {Times 12 bold} -tags you
.c create text  10 370 -anchor w -text " " -font {Times 12 bold} -tags opp
.c create text 290 320 -anchor e -text " " -font {Times 12 bold} -tags status
        
# Frame for holding buttons
frame .bf
pack  .bf -expand 1 -fill x

# Exit button, cannot press button while game is active
button .bf.exit -text "Exit" -command  {exit}

# Silent button
button .bf.ssilent -text "Silent" -command {if {$callBell == 0} {.bf.ssilent configure -text "Sound"; \
        set callBell 1}  else {.bf.ssilent configure -text "Silent";  set callBell 0 }}

proc ring {} {global callBell; if {$callBell == 0} {bell}}
    
# Resign button
button  .bf.sresign -text "Resign" -command {puts stdout "11"}

frame .r -bd 2 -relief ridge
pack .r -side bottom 

# Pack buttons into frame
pack .bf.ssilent .bf.sresign .bf.exit -side left -expand 1 -fill x

# Make exit button unclickable, ttt.c makes it clickable
tk busy .bf.exit

