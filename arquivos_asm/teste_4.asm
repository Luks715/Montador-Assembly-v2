SECTION TEXT

BEGIN: MOD1

MOD2: EXTERN
Public L2
Public OLD_DATA
Public NEW_DATA
Public TMP_DATA
PROCESS_DATA
INPUT OLD_DATA
LOAD OLD_DATA
jmp mod2
L2: STOP

END 

SECTION DATA
DOIS: CONST 2
OLD_DATA: SPACE
NEW_DATA: SPACE
TMP_DATA: SPACE
