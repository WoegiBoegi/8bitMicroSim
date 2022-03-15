$x02A0
;declare word and give it a label
:string
w:"hello world!"

$x0180
;print string
LHL :string
MIL B,x00
LDM
:loop
INT 1
INC L
LDM
CMP B
JNE :loop
HLT