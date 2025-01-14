section text
INPut N 


;comentário
Load n 
FAT: 
SUB oNE
jmpz fim

store aux
mult n
sTore N 
load aux 
jmp fat
FIM: output N 
stop

secTion data 
aux: space
n: space 
one: const 0xF