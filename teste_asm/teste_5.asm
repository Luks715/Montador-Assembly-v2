PROCESS_DATA: MACRO
LOAD OLD_DATA
DIV DOIS
STORE NEW_DATA
ENDMACRO

SECTION TEXT
L1: PROCESS_DATA
PROCESS_INPUT
PROCESS_DATA
PROCESS_DATA
DIS DOIS
JMP L2
STOP

SECTION DATA
OLD_DATA: SPACE 
NEW_DATA: SPACE
DOIS: CONST 1