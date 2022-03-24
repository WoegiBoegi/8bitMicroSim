$x0180
;this is a comment :)
LHL :string
MIL B,x00
LDM
:startLoop
CMP B
JIE :endLoop
INT d1
INC L
LDM
JMP :startLoop
:endLoop
HLT
;this is another comment :)
:string
w:"hello world!"