$x02A0
:cool
INC B
MOV C,B
RET

$x0180
;this is a comment
LDL x80
STA x01A0
MOV B,A
SPR B
CAL :cool
SBR B
HLT