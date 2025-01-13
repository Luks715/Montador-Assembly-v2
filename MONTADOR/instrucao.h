#ifndef INSTRUCAO_H
#define INSTRUCAO_H

#include <string>
#include <regex>
#include "mnemonicos.h"
using namespace std;

// ESTE ARQUIVO CONTÉM A STRUCT INSTRUÇÃO E ALGUMAS FUNÇÕES USADAS PARA CRIAR, MANIPULAR E VALIDAR INSTRUÇÕES

struct Instrucao {
    string rotulo;
    string operacao;
    string operando1;
    string operando2;
};

// RETORNA A LABEL PARA O CASO DE UM OPERANDO USAR ARITMÉTICA DE PONTEIROS
// EX: LOAD NUM+2, RETORNA NUM
string retorna_operando_label(string operando) {
    size_t pos = operando.find('+');
    
    // Se '+' foi encontrado, retorna a parte antes dele
    if (pos != std::string::npos) {
        return operando.substr(0, pos); // Retorna substring até o '+'
    }
    
    // Se não há '+', retorna a string original
    return operando;
}

// RETORNA O NÚMERO PARA O CASO DE UM OPERANDO USAR ARITMÉTICA DE PONTEIROS
// EX: LOAD NUM+2 RETORNA 2
string retorna_operando_offset(string operando) {
    size_t pos = operando.find('+');

    if (pos != std::string::npos) {
        return (operando.substr(pos));
    }

    return "0";
}

// FUNÇÃO QUE RETORNA QUANTOS OPERANDOS UMA INSTRUÇÃO RECEBEU
// EX: ADD NUM NUM_DOIS RETORNA 2, COPY NUM RETORNA 1, SUB RETORNA 0
int verifica_operandos(Instrucao instrucao) {
    if (!instrucao.operando1.empty() && !instrucao.operando2.empty()) {
        return 2; // Ambos os operandos existem
    } else if (!instrucao.operando1.empty()) {
        return 1; // Apenas operando1 existe
    } else {
        return 0; // Nenhum dos operandos existe
    }
}

// NOME AUTO EXPLCIATIVO
bool rotulo_valido(string rotulo) {
    regex label_regex("^[A-Z_][A-Z0-9_]*$");
    return regex_match(rotulo, label_regex);
}

// TRABALHA EM CONJUNTO COM A FUNÇÃO DIRETIVA_VALIDA PARA AVALIAR SE A DIRETIVA RECEBIDA É VÁLIDA
bool processa_diretiva(Instrucao instrucao) {
    int operandos_recebidos = verifica_operandos(instrucao);
    int operandos_esperados = diretivas[instrucao.operacao].second;

    if (operandos_recebidos == operandos_esperados) {
        if (instrucao.operacao == "SECTION") {
            if (instrucao.operando1 == "TEXT" || instrucao.operando1 == "DATA") {
                return true;
            } else {
                string linha = instrucao.operacao + " " + instrucao.operando1;
                cerr << "ERRO SINTÁTICO: SECTION INVÁLIDA: " << instrucao.operando1 << " NA LINHA " << linha << endl;
                return false;
            }
        } else if (instrucao.operacao == "SPACE") {
            for (char c : instrucao.operando1) {
                if (!std::isdigit(c)) {
                    string linha = instrucao.operacao + " " + instrucao.operando1;

                    cerr << linha << endl;
                    cerr << "ERRO SEMÂNTICO: O TIPO ESPERADO PARA O ARGUMENTO ERA NUMÉRICO: " << linha << endl;
                    return false;
                }
            }
            return true;
        } else if (instrucao.operacao == "CONST"){
            regex operando_const_regex("^(0X[0-9A-F]+|-?[0-9]+)$");

            if(regex_match(instrucao.operando1, operando_const_regex)) {
                return true;
            } else {
                string linha = instrucao.operacao + " " + instrucao.operando1;

                cerr << linha << endl;
                cerr << "ERRO SEMÂNTICO: O TIPO ESPERADO PARA O ARGUMENTO ERA NUMÉRICO EM DECIMAL OU HEXADECIMAL EM COMPLEMENTO DE 2: " << linha << endl;
                return false;
            }
        } else {
            return true;
        }
    } else {
        if (instrucao.operacao == "SPACE") {
            if (operandos_recebidos < operandos_esperados) {
            return true;
            } 
        } 

        string linha = instrucao.operacao + " " + instrucao.operando1 + " " + instrucao.operando2;
        cerr << "ERRO SINTÁTICO: NÚMERO DE OPERANDOS ERRADOS PARA UMA DIRETIVA NA LINHA: " << linha  << endl;
        cerr << "OPERANDOS ESPERADOS: " << operandos_esperados << ", OPERANDOS RECEBIDOS: " << operandos_recebidos << endl;
        return false;
    }   
}

// AVALIA SE A DIREITVA RECEBIDA É VÁLIDA
bool diretiva_valida(Instrucao instrucao){
    if (!instrucao.rotulo.empty()){
        if (diretivas[instrucao.operacao].first) {
            return processa_diretiva(instrucao);
        } else {
            cerr << "ERRO SINTÁTICO: ESSA DIRETIVA NÃO PODE ESTAR EM UM RÓTULO" << endl;
            return false;
        }
    } else {
        if (diretivas[instrucao.operacao].first) {
            cerr << "ERRO SINTÁTICO: ESSA DIRETIVA PRECISA ESTAR EM UM RÓTULO" << endl;
            return false;
        } else {
            return processa_diretiva(instrucao);
        }
    }
}

// AVALIA SE A INSTRUÇÃO RECEBIDA É VÁLIDA
bool instrucao_valida(Instrucao instrucao){
    int operandos_recebidos = verifica_operandos(instrucao);
    int operandos_esperados = mnemonicos[instrucao.operacao].second - 1;

    if (operandos_recebidos == operandos_esperados){
        if (instrucao.rotulo.empty()){
            return true;
        } else {
            if (!instrucao.operando1.empty()){
                if (instrucao.rotulo == retorna_operando_label(instrucao.operando1)){
                    cerr << "ERRO SEMÂNTICO: RÓTULO " << instrucao.rotulo << "DOBRADO NA MESMA LINHA" << endl;
                    return false;
                } else {
                    if (!instrucao.operando2.empty()){
                        if (instrucao.rotulo == retorna_operando_label(instrucao.operando2)) {
                            cerr << "ERRO SEMÂNTICO: RÓTULO " << instrucao.rotulo << "DOBRADO NA MESMA LINHA" << endl;
                            return false;
                        } else {
                            return true;
                        }
                    }
                    return true;
                }
            }
            return true;
        }
    } else {
        string linha = instrucao.operacao + " " + instrucao.operando1 + " " + instrucao.operando2;
        cerr << "ERRO SINTÁTICO: NÚMERO DE OPERANDOS ERRADOS PARA UMA INSTRUÇÃO NA LINHA: " << linha  << endl;
        cerr << "OPERANDOS ESPERADOS: " << operandos_esperados << ", OPERANDOS RECEBIDOS: " << operandos_recebidos << endl;
        return false;
    }
}

// AVALIA SE A LINHA RECEBIDA É UMA INSTRUÇÃO, UMA DIRETIVA OU NADA
string analise_linha(Instrucao instrucao){
    if (mnemonicos.find(instrucao.operacao) != mnemonicos.end()){
        return "INSTRUCAO";
    } else if (diretivas.find(instrucao.operacao) != diretivas.end()) {
        return "DIRETIVA";
    } else {
        return "INCONCLUSIVO";
    }
}

// TRABALHA EM CONJUNTO COM A FUNÇÃO CRIAR_INSTRUÇÃO, SEPARA A LINHA RECEBIDA POR ESPAÇOS E RETORNA UM VETOR COM CADA PARTE
vector<string> separa_string(string linha) {
    istringstream iss(linha);
    vector<string> tokens;
    string token;

    while (iss >> token) {
        tokens.push_back(token);
    }
    return tokens;
}

// RECEBE UMA LINHA E RETORNA UMA INSTRUÇÃO
Instrucao criar_instrucao(string linha) {
    Instrucao instrucao;
    vector<string> tokens = separa_string(linha);

    if (!tokens.empty() && tokens[0].back() == ':') { 
        instrucao.rotulo = tokens[0].substr(0, tokens[0].size() - 1); 
        tokens.erase(tokens.begin()); 
    }

    // Verifica se ainda há tokens para definir como operação
    if (!tokens.empty()) {
        instrucao.operacao = tokens[0]; 
        tokens.erase(tokens.begin()); 
    }

    // Verifica se ainda há tokens para definir como operando1
    if (!tokens.empty()) {
        instrucao.operando1 = tokens[0]; 
        tokens.erase(tokens.begin()); 
    }

    // Verifica se ainda há tokens para definir como operando2
    if (!tokens.empty()) {
        instrucao.operando2 = tokens[0]; 
        tokens.erase(tokens.begin()); 
    }

    // Caso ainda existam elementos no vetor (ex: LOAD NUM + 1) adiciona eles no operando 2
    if (!tokens.empty()) {
        for (const auto &token : tokens) {
            if (!instrucao.operando2.empty()) {
                instrucao.operando2 += " "; // Adiciona espaço entre os elementos
            }
            instrucao.operando2 += token; // Concatena o restante dos tokens no operando2
        }
    }

    return instrucao;
}
#endif