#ifndef MNEMONICOS_H
#define MNEMONICOS_H

#include <map>  
#include <string> 
#include <utility>

using namespace std;

// MAP CONTENDO TODAS AS DIRETIVAS VÁLIDAS DO PROGRAMA

// O PRIMEIRO VALOR DA TUPLA REPRESENTA SE ESSA DIRETIVA PRECISA DE UMA LABEL OU NÃO 
//EX: SECTION NÃO PRECISA DE LABEL, MAS BEGIN E EXTERN SIM

// O SEGUNDO VALOR DA TUPLA REPRESENTA A QUANTIDADE DE OPERANDOS QUE A DIRETIVA PRECISA
// EX: SPACE E CONST PRECISAM DO OPERANDO, MAS BEGIN E EXTERN NÃO
map<string, pair<bool, int>> diretivas = {
    {"SECTION"  ,{false, 1}},
    {"END"      ,{false, 0}},
    {"PUBLIC"   ,{false, 1}},
    {"EXTERN"   ,{true,  0}},
    {"SPACE"    ,{true,  1}},
    {"CONST"    ,{true,  1}},
    {"MACRO"    ,{true,  0}},
    {"ENDMACRO" ,{false, 0}}
};

// MAP CONTENDO TODAS AS INSTRUÇÕES VÁLIDAS DO PROGRAMA, SEUS OPCODES, E A QUANTIDADE DE ESPAÇO QUE OCUPAM NA MEMÓRIA
map<string, pair<string,int>> mnemonicos = {
    {"ADD"      ,{"1", 2}},
    {"SUB"      ,{"2", 2}},
    {"MULT"     ,{"3", 2}},
    {"DIV"      ,{"4", 2}},
    {"JMP"      ,{"5", 2}},
    {"JMPN"     ,{"6", 2}},
    {"JMPP"     ,{"7", 2}},
    {"JMPZ"     ,{"8", 2}},
    {"COPY"     ,{"9", 3}},
    {"LOAD"     ,{"10",2}},
    {"STORE"    ,{"11",2}},
    {"INPUT"    ,{"12",2}},
    {"OUTPUT"   ,{"13",2}},
    {"STOP"     ,{"14",1}}
};
#endif