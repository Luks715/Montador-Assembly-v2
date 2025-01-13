#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_set>
#include "mnemonicos.h"
#include "instrucao.h"

using namespace std;

string secao_atual = "";
map<string, pair<std::vector<int>, string>> tabela_simbolos; 
map<string, int> tabela_definicoes;
map<string, std::vector<int>> tabela_usos;
bool tem_modulo = false;

std::string retorna_decimal(const std::string& num) {
    // REGEX QUE VERIFICA SE UM NÚMERO É HEXADECIMAL
    std::regex hex_regex("^(0X[0-9A-F]+)$"); 
    std::smatch match;

    if (std::regex_match(num, match, hex_regex)) {
        // Remove o prefixo '0X'
        std::string hex_value = num.substr(2); // Pega apenas o número hexadecimal
        
        // Converte hexadecimal para inteiro sem sinal (usando std::stoul)
        unsigned int value = std::stoul(hex_value, nullptr, 16);

        // Define o número de bits pelo comprimento do hexadecimal
        size_t num_bits = hex_value.length() * 4; // Cada dígito hexadecimal representa 4 bits

        // Se o número estiver no intervalo negativo em complemento de 2
        if (value >= (1U << (num_bits - 1))) {
            int signed_value = static_cast<int>(value - (1U << num_bits));
            return std::to_string(signed_value);
        } else {
            return std::to_string(static_cast<int>(value));
        }
    } else {
        return num; // Retorna o número original se não for hexadecimal
    }
}

// FUNÇÃO AUXILIAR QUE VERIFICA SE O OPERANDO PASSADO PARA A INSTRUÇÃO É VÁLIDO
// PROCURA O OPERANDO EM TODAS AS TABELAS
pair<string, string> operando_valido(string operando_recebido, string linha) {
    string operando = retorna_operando_label(operando_recebido);
    string offset = retorna_operando_offset(operando_recebido);

    // Verifica se offset é um número
    bool is_offset_number = !offset.empty() && std::all_of(offset.begin(), offset.end(), ::isdigit);

    if(tabela_simbolos.find(retorna_operando_label(operando)) != tabela_simbolos.end()){
        if (offset != "0") {
            if (is_offset_number){
                int tamanho = tabela_simbolos[operando].first.size();
                int indice = std::stoi(offset);

                if (tamanho > indice){
                    int pos = tabela_simbolos[operando].first[0] + indice;
                    return make_pair("simbolos", std::to_string(pos));
                } else {
                    cout << linha << endl;
                    cerr << "ERRO SINTÁTICO: ÍNDICE " << indice << " NÃO EXISTE NO VETOR " << operando  << endl;
                    return make_pair("false", " ");
                }
            } else {
                cout << linha << endl;
                cerr << "ERRO SINTÁTICO: TIPO INCORRETO PARA O SEGUNDO TERMO DA SOMA, DEVERIA SER INTEIRO, MAS FOI: " << offset << endl;
                return make_pair("false", " ");
            }
        } else {
            int pos = tabela_simbolos[operando].first[0];
            return make_pair("simbolos", std::to_string(pos));
        }
    } else if (tabela_usos.find(retorna_operando_label(operando)) != tabela_usos.end()) {
        if (offset != "0") {
            if (is_offset_number){
                return make_pair("usos", offset);
            } else {
                cout << linha << endl;
                cerr << "ERRO SINTÁTICO: TIPO INCORRETO PARA O SEGUNDO TERMO DA SOMA, DEVERIA SER INTEIRO, MAS FOI: " << offset << endl;
                return make_pair("false", " ");
            }
        } else {
            return make_pair("usos", "0");
        }
    } else {
        cout << linha << endl;
        cerr << "ERRO SEMÂNTICO: LABEL " << operando << " NÃO DEFINIDA, RÓTULO AUSENTE" << endl;
        return make_pair("false", " ");        
    }
}

// FUNÇÃO PRINCIPAL QUE LÊ TODAS AS LINHAS DE UM ARQUIVO .PRE, REALIZA O ALGORITMO DE DUAS PASSAGENS E RETORNA O ARQUIVO .OBJ
void montagem(string arquivo_pre_nome) {
    string caminho_arquivo_pre = "../teste_pre/" + arquivo_pre_nome;

    ifstream inputFile(caminho_arquivo_pre); 
    if (!inputFile.is_open()) {
        cerr << "O arquivo: " << arquivo_pre_nome << " Não existe" << endl;
        return;
    }

    string arquivo_obj_nome = regex_replace(arquivo_pre_nome, regex("\\.pre$"), ".obj");
    string caminho_arquivo_obj = "../teste_obj/" + arquivo_obj_nome;
    
    // VERIFICA SE O ARQUIVO DE SAÍDA FOI CRIADO
    ofstream outputFile(caminho_arquivo_obj); 
    if (!outputFile.is_open()) {
        cerr << "Erro: Não foi possível criar o arquivo de saída " << arquivo_pre_nome << endl;
        return;
    }

    // VARIÁVEIS PARA A PRIMEIRA PASSAGEM DO MONTADOR
    int pos_memoria = 0;

    // PARA IMPEDIR QUE O USUÁRIO SOBRESCREVA O CÓDIGO NA MEMÓRIA,
    // EX: STORE L1 + 1 NO CASO EM QUE L1 É UMA LABEL DE SECTION TEXT,
    // ESTOU USANDO ESSA IMPLEMENTAÇÃO PARA SALVAR OS LABELS DE CADA SECTION SEPARADAMENTE
    string secao_atual = "";

    // PRIMEIRA PASSAGEM
    string linha;
    cout << "primeira passagem\n";

    while (getline(inputFile, linha)) {
        Instrucao instrucao = criar_instrucao(linha);

        if (!instrucao.rotulo.empty()) {
            if ((tabela_simbolos.find(instrucao.rotulo) == tabela_simbolos.end()) && (tabela_usos.find(instrucao.rotulo) == tabela_usos.end())) {
                if (instrucao.operacao == "BEGIN") {
                    tem_modulo = true;
                    tabela_simbolos[instrucao.rotulo].first.push_back(0); 
                    tabela_simbolos[instrucao.rotulo].second = "MOD";

                } else if (instrucao.operacao == "EXTERN") {
                    tabela_usos.insert({instrucao.rotulo, {}});

                } else if (instrucao.operacao == "SPACE"){
                    // EX:
                    // PUBLIC NUMS
                    // PUBLIC DOIS    tabela_simbolos = {NUM: [0, 1, 2], DOIS: [3], NUM2: [4, 5]}
                    // NUM: SPACE 3   tabela_definicoes = {NUM: [0, 1, 2], DOIS: [3]}
                    // DOIS: CONST 2
                    // NUM2: SPACE 2

                    tabela_simbolos.insert({instrucao.rotulo, {{}, secao_atual}});

                    if(instrucao.operando1.empty()){
                        tabela_simbolos[instrucao.rotulo].first.push_back(pos_memoria);

                        if(tabela_definicoes.find(instrucao.rotulo) != tabela_definicoes.end()){
                            tabela_definicoes[instrucao.rotulo] = pos_memoria;
                        }

                        pos_memoria += 1;
                    } else {
                        int space_op1 = std::stoi(instrucao.operando1);

                        for (int i = 0; i < space_op1; ++i) {
                            tabela_simbolos[instrucao.rotulo].first.push_back(pos_memoria + i);
                        }  

                        if(tabela_definicoes.find(instrucao.rotulo) != tabela_definicoes.end()){
                            tabela_definicoes[instrucao.rotulo] = pos_memoria;
                        }

                        pos_memoria += std::stoi(instrucao.operando1);
                    }
                
                } else if (instrucao.operacao == "CONST") {
                    tabela_simbolos.insert({instrucao.rotulo, {{}, secao_atual}});
                    tabela_simbolos[instrucao.rotulo].first.push_back(pos_memoria);
                    
                    if (tabela_definicoes.find(instrucao.rotulo) != tabela_definicoes.end()){
                        tabela_definicoes[instrucao.rotulo] = pos_memoria;
                    }
                    pos_memoria += 1;
                
                } else {
                    tabela_simbolos.insert({instrucao.rotulo, {{}, secao_atual}});
                    tabela_simbolos[instrucao.rotulo].first.push_back(pos_memoria);

                    if (tabela_definicoes.find(instrucao.rotulo) != tabela_definicoes.end()){
                        tabela_definicoes[instrucao.rotulo] = pos_memoria;
                    }

                    // EX: 
                    // R: EXTERN
                    // S: EXTERN
                    // OUTPUT R           tabela_usos = {R: [1, 3, 7], S: [4]}
                    // COPY R, S
                    // COPY NUM, R
                    // NUM: SPACE
                    if (!instrucao.operacao.empty()){
                        if (!instrucao.operando1.empty()){
                            string op1 = retorna_operando_label(instrucao.operando1);

                            if (tabela_usos.find(op1) != tabela_usos.end()){
                                tabela_usos[retorna_operando_label(instrucao.operando1)].push_back(pos_memoria + 1);
                            }

                            if (!instrucao.operando2.empty()) {
                                string op2 = retorna_operando_label(instrucao.operando2);

                                if (tabela_usos.find(op1) != tabela_usos.end()){
                                    tabela_usos[retorna_operando_label(instrucao.operando1)].push_back(pos_memoria + 2);
                                }
                            }
                        }
                        pos_memoria += mnemonicos[instrucao.operacao].second;
                    }
                }
            } else {
                cout << linha << endl;
                cerr << "ERRO SINTÁTICO: RÓTULO " << instrucao.rotulo << " JÁ FOI DEFINIDO" << endl;
            }
        } else {
            if (instrucao.operacao == "SECTION"){
                secao_atual = instrucao.operando1;
            } else if (instrucao.operacao == "PUBLIC"){
                tabela_definicoes[instrucao.operando1] = 0;
            } else if (instrucao.operacao == "END") {
                break;
            } else {
                if (!instrucao.operando1.empty()){
                    string op1 = retorna_operando_label(instrucao.operando1);

                    if (tabela_usos.find(op1) != tabela_usos.end()){
                        tabela_usos[retorna_operando_label(instrucao.operando1)].push_back(pos_memoria + 1);
                    }

                    if (!instrucao.operando2.empty()) {
                        string op2 = retorna_operando_label(instrucao.operando2);

                        if (tabela_usos.find(op1) != tabela_usos.end()){
                            tabela_usos[retorna_operando_label(instrucao.operando1)].push_back(pos_memoria + 2);
                        }
                    }
                }
                pos_memoria += mnemonicos[instrucao.operacao].second;
            }
        }
    }

    inputFile.close();
    inputFile.open(caminho_arquivo_pre);
    // FIM DA PRIMEIRA PASSAGEM, AO FIM DESSE WHILE, TODAS AS LABELS, DEFINIÇÕES E USOS ESTÃO NAS TABELAS

    std::cout << "\nMap 'tabela_simbolos':\n";
    for (const auto& pair : tabela_simbolos) {
        std::cout << pair.first << " -> ";
        for (const auto& string : pair.second.first) {
            std::cout << string << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\nMap 'tabela_usos':\n";
    for (const auto& pair : tabela_usos) {
        std::cout << pair.first << " -> ";
        for (const auto& num : pair.second) {
            std::cout << num << " ";
        }
        std::cout << std::endl;
    }

    std::cout << "\nMap 'tabela_definicoes':\n";
    for (const auto& pair : tabela_definicoes) {
        std::cout << pair.first << " -> " << pair.second;
        std::cout << std::endl;
    }

    // VARIÁVEIS PARA A SEGUNDA PASSAGEM DO MONTADOR
    string arquivo_obj;
    string relativos;
    while(getline(inputFile, linha)){
        Instrucao instrucao = criar_instrucao(linha);
        string instrucao_montada;

        if (!instrucao.operacao.empty()){
            if (instrucao.operacao == "CONST") {
                arquivo_obj += retorna_decimal(instrucao.operando1) + " ";
                relativos += "0 ";

            } else if (instrucao.operacao == "SPACE") {
                if(!instrucao.operando1.empty()){
                    for (int i = 0; i < std::stoi(instrucao.operando1); ++i) {
                        arquivo_obj += "0 ";
                        relativos += "0 ";
                    }
                } else {
                    arquivo_obj += "0 ";
                    relativos += "0 ";
                } 

            } else if (mnemonicos.find(instrucao.operacao) != mnemonicos.end()){
                instrucao_montada += mnemonicos[instrucao.operacao].first + " ";
                relativos += "0 ";

                if (!instrucao.operando1.empty()) {
                    pair<string, string> op1_valido = operando_valido(instrucao.operando1, linha);

                    if (op1_valido.first != "false"){
                        instrucao_montada += op1_valido.second + " ";
                        relativos += "1 ";

                        if(!instrucao.operando2.empty()){
                            pair<string, string> op2_valido = operando_valido(instrucao.operando2, linha);

                            if(op2_valido.first != "false") {
                                instrucao_montada += op2_valido.second + " ";
                                arquivo_obj += instrucao_montada;
                                instrucao_montada = "";
                                relativos += "1 ";
                            }
                        } else {
                            arquivo_obj += instrucao_montada;
                            instrucao_montada = "";
                        }
                    }
                } else {
                    arquivo_obj += instrucao_montada;
                    instrucao_montada = "";
                }
            }
        }
    }

    if(tem_modulo){
        for (const auto& pair : tabela_definicoes) {
            outputFile << "D, " << pair.first << " " << pair.second << endl;
        }

        for (const auto& pair : tabela_usos) { 
            for (int num : pair.second) {  
                outputFile << "U, " << pair.first << " " << num << endl;  
            }  
        }

        outputFile << "R, " << relativos << endl;
    } 

    outputFile << arquivo_obj;
    inputFile.close();
    outputFile.close();

    cout << "Arquivo obj gerado com sucesso: " << arquivo_obj_nome << endl;
}