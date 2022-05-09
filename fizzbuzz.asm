$x0180

MIL B,d100
MIL A,d1
:MainLoop

MIL D,d0

MIL C,d3
SPR A
SPR B
MIL B,d0
CAL :div
SBR B
MIL C,d0 
CMP C
SBR A
LHL :StrFizz
CIE :printHLStr

MIL C,d5
SPR A
SPR B
MIL B,d0
CAL :div
SBR B
MIL C,d0 
CMP C
SBR A
LHL :StrBuzz
CIE :printHLStr

SPR A
MIL A,d0
CMP D
SBR A
CIE :printACCdec

LHL :StrNL
CAL :printHLStr

INC A
CMP B
JIE :end
JMP :MainLoop

:end
HLT

:StrFizz
w:"Fizz"
l:d0
:StrBuzz
w:"Buzz"
l:d0
:StrNL
l:x0a
l:d0

:div
CMP C
JGT :endDiv
SUB C
INC B
JMP :div
:endDiv
RET

:printACCdec
SPR C
SPR A
SPR B
MIL B,d0
MIL C,d10
CAL :div
MOV C,A
; B - res, C - rest

CAL :printNumRegB
MOV B,C
CAL :printNumRegB

SBR B
SBR A
SBR C

RET

:printNumRegB
MIL A,x30
ADD B
INT d1
RET

:printHLStr
INC D
SPR A
SPR B
MIL B,d0
CAL :printLoop
SBR B
SBR A
RET

:printLoop
LDM
CMP B
JIE :endPrintLoop
INT d1
INC L
JMP :printLoop

:endPrintLoop
RET