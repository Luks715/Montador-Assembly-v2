#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <map>
#include <vector>
#include "instrucao.h"
#include "mnemonicos.h"

using namespace std;

string section_text;
string section_data;
string nome_modulo;

string linha;
bool section_text_atual = false;
bool ja_existe_section_text = false;

bool section_data_atual = false;
bool ja_existe_section_data = false;

bool tem_modulo_aberto = false;
bool modulo_finalizado = true;
bool ja_existe_modulo = false;

bool ja_existe_macro = false;
bool tem_macro_aberta = false;
bool macro_finalizada = true;

map<string, std::vector<string>> MDT;

// FUNÇÃO QUE FORMATA A LINHA, REMOVE COMENTÁRIOS E ESPAÇOS
string formata_linha(string linha) {
    // Remove comentários (tudo após o ";")
    string linha_formatada = regex_replace(linha, regex(";.*"), "");

    // Remove espaços desnecessários no início e no final da linha
    linha_formatada = regex_replace(linha_formatada, regex("^\\s+|\\s+$"), "");

    // Substitui múltiplos espaços por um único espaço
    linha_formatada = regex_replace(linha_formatada, regex("\\s+"), " ");

    transform(linha_formatada.begin(), linha_formatada.end(), linha_formatada.begin(), [](unsigned char c) {
        return toupper(c);
    });

    return linha_formatada;
}

// ADICIONA A LINHA NA SEÇÃO CORRETA
void adiciona_linha(string linha){
    if (section_text_atual) {
        section_text += linha + "\n";
    } else if (section_data_atual) {
        section_data += linha + "\n";
    } else {
        cerr << "ADICIONAR LINHA FORA DE UMA SECTION" << endl;
    }
}

void processa_linha(string linha, Instrucao instrucao, ifstream& inputFile, ofstream& outputFile) {
    if (!instrucao.rotulo.empty()) {
        if (rotulo_valido(instrucao.rotulo)) {
            if (instrucao.rotulo == "BEGIN") {
                if (ja_existe_modulo) {
                    cout << linha << endl;
                    cerr << "ERRO SINTÁTICO: INICIAR MAIS DE UM MÓDULO NO MESMO ARQUIVO" << linha << endl;
                
                } else if (ja_existe_macro) {
                    cout << linha << endl;
                    cerr << "ERRO SINTÁTICO: INICIAR UMA MACRO FORA DO MÓDULO" << linha << endl;

                } else {
                    if (instrucao.operacao.empty()) {
                        cout << linha << endl;
                        cerr << "ERRO SINTÁTICO: INICIAR UM MÓDULO SEM UMA LABEL" << linha << endl;

                    } else {
                        if (rotulo_valido(instrucao.operacao)) {
                            if (!instrucao.operando1.empty()) {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: MAIS DE UM OPERANDO RECEBIDO" << linha << endl;

                            } else {
                                if(section_data_atual){
                                    cout << linha << endl;
                                    cerr << "ERRO SINTÁTICO: ABRIR UM MÓDULO DENTRO DA SECTION DATA" << linha << endl;
                                
                                // NÃO SEI SE ISSO DEVE SER UM ERRO OU NÃO
                                } else if (!section_text_atual){
                                    cout << linha << endl;
                                    cerr << "ERRO SINTÁTICO: ABRIR UM MÓDULO FORA DA SECTION TEXT" << linha << endl;

                                } else {
                                    tem_modulo_aberto = true;
                                    modulo_finalizado = false;
                                    ja_existe_modulo = true;
                                    outputFile << instrucao.operacao + ": BEGIN\n";
                                }
                            }  
                        } else {
                            cout << linha << endl;
                            cerr << "ERRO: A LABEL " << instrucao.operacao << " É INVÁLIDA" << endl;
                        }
                    }
                }
            }
            else if (instrucao.operacao.empty()) {
                adiciona_linha(linha);

            } else {
                string resultado = analise_linha(instrucao);

                if (resultado == "DIRETIVA"){
                    if (diretiva_valida(instrucao)) {
                        if (instrucao.operacao == "EXTERN") {
                            if(tem_modulo_aberto){
                                if(section_text_atual){
                                    section_text += linha + "\n";
                                } else {
                                    cout << linha << endl;
                                    cerr << "ERRO SEMÂNTICO: DECLARAÇÃO DE EXTERN FORA DA SECTION TEXT" << endl;
                                }
                            } else {
                                cout << linha << endl;
                                cerr << "ERRO SEMÂNTICO: DEFINIÇÃO DE EXTERN FORA DE UM MÓDULO" << endl;
                            }

                        } else if (instrucao.operacao == "CONST" || instrucao.operacao == "SPACE"){
                            if(section_text_atual){
                                cout << linha << endl;
                                cerr << "ERRO: DECLARAÇÃO DE CONSTS E SPACES NA SECTION TEXT" << endl;
                            } else if (section_data_atual) {
                                section_data += linha + "\n";
                            } else {
                                cout << linha << endl;
                                cerr << "ERRO: DECLARAÇÃO DE CONSTS E SPACES FORA DA SECTION DATA" << endl;
                            }

                        } else if (instrucao.operacao == "MACRO") {
                            if (section_text_atual) {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE DECLARAR UMA MACRO DENTRO DA SECTION TEXT" << endl;
                            
                            } else if (section_data_atual) {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE DECLARAR UMA MACRO DENTRO DA SECTION DATA" << endl;
                            } else {
                                if (MDT.size() < 2) {
                                    if (MDT.find(instrucao.rotulo) == MDT.end()) {
                                        ja_existe_macro = true;
                                        tem_macro_aberta = true;
                                        macro_finalizada = false;
                                        cerr << "Uma macro está sendo adicionada com o rótulo " << instrucao.rotulo << endl;
                                        MDT[instrucao.rotulo] = {};

                                        while (getline(inputFile, linha)) {
                                            if (!linha.empty()) {
                                                string linha_formatada_macro = formata_linha(linha);
                                                Instrucao instrucao_macro = criar_instrucao(linha_formatada_macro);

                                                string resultado = analise_linha(instrucao_macro);
                                                if (resultado == "DIRETIVA") {
                                                    if (diretiva_valida(instrucao_macro)) {
                                                        if (instrucao_macro.operacao == "ENDMACRO") {
                                                            tem_macro_aberta = false;
                                                            macro_finalizada = true;
                                                            break;
                                                        } else if (instrucao_macro.operacao == "MACRO") {
                                                            cout << linha << endl;
                                                            cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE DECLARAR UMA MACRO DENTRO DE OUTRA MACRO." << endl;
                                                        } else {
                                                            cout << linha << endl;
                                                            cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE USAR ESSA DIRETIVA EM UMA MACRO." << endl;
                                                        }
                                                    } 
                                                } else if (resultado == "INSTRUCAO") {
                                                    if (instrucao_valida(instrucao_macro)) {
                                                        MDT[instrucao.rotulo].push_back(linha_formatada_macro);
                                                    }
                                                } else {
                                                    if (MDT.find(instrucao_macro.operacao) != MDT.end()) {
                                                        for (string linha_macro_aninhada : MDT[instrucao_macro.operacao]) {
                                                            MDT[instrucao.rotulo].push_back(linha_macro_aninhada);
                                                        }
                                                    } else {
                                                        cout << linha << endl;
                                                        cerr << "ERRO LÉXICO: INSTRUÇÃO OU DIRETIVA INVÁLIDA" << endl;
                                                    }
                                                }

                                            }
                                        } 
                                    } else {
                                        cout << linha << endl;
                                        cerr << "ERRO SINTÁTICO: REDEFINIÇÃO DA MACRO: " << instrucao.rotulo << endl;
                                    }
                                } else {
                                    cout << linha << endl;
                                    cerr << "ERRO SINTÁTICO: APENAS 2 MACROS SUPORTADAS." << endl;
                                }
                            }
                        } else {
                            adiciona_linha(linha);
                        }
                    } 
                } else if (resultado == "INSTRUCAO") {
                    if (instrucao_valida(instrucao)) {
                        if (section_text_atual) {
                            section_text += linha + "\n";
                        } else {
                            cout << linha << endl;
                            cerr << "ERRO: DECLARAÇÃO DE COMANDOS FORA DA SECTION TEXT" << endl;
                        }
                    } 
                } else {
                    if(MDT.find(instrucao.operacao) != MDT.end()) {
                        if(!instrucao.operando1.empty()){
                            cout << linha << endl;
                            cerr << "ERRO: ESSA DIRETIVA NÃO ACEITA OPERANDOS" << endl;
                        } else {
                            if (section_text_atual) {
                                section_text += instrucao.rotulo + ": " + MDT[instrucao.operacao][0] + "\n";
                                for (int i = 1; i < MDT[instrucao.operacao].size(); i++){
                                    section_text += MDT[instrucao.operacao][i] + "\n";
                                } 
                            } else {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE USAR MACROS NA SECTION DATA";
                            }
                        }
                    } else {
                        cout << linha << endl;
                        cerr << "ERRO LÉXICO: INSTRUÇÃO OU DIRETIVA INVÁLIDA" << endl;
                    }
                }
            }
        } else {
            cout << linha << endl;
            cerr << "ERRO LÉXICO: A LABEL " << instrucao.rotulo << " É INVÁLIDA" << endl;
        }
    } else {
        string resultado = analise_linha(instrucao);
        
        if (resultado == "DIRETIVA"){
            if (diretiva_valida(instrucao)) {
                if (instrucao.operacao == "SECTION") {
                    if (tem_modulo_aberto) {
                        cout << linha << endl;
                        cerr << "ERRO SINTÁTICO: ABRIR UMA SECTION DENTRO DO MÓDULO " << linha << endl;
                    } else {
                        if (instrucao.operando1 == "TEXT") {
                            if(ja_existe_section_text) {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: NÃO É PERMITIDO MAIS DE UMA SECTION TEXT" << endl;
                            } else {
                                section_text_atual = true;
                                section_data_atual = false;
                                ja_existe_section_text = true;
                            }

                        } else if (instrucao.operando1 == "DATA"){
                            if(ja_existe_section_data) {
                                cout << linha << endl;
                                cerr << "ERRO SINTÁTICO: NÃO É PERMITIDO MAIS DE UMA SECTION DATA" << endl;
                            } else {
                                section_text_atual = false;
                                section_data_atual = true;
                                ja_existe_section_data = true;
                            }
                        }
                    }
                } else if (instrucao.operacao == "END") {
                    if (tem_modulo_aberto){
                        tem_modulo_aberto = false;
                        modulo_finalizado = true;
                        section_text_atual = false;
                    } else {
                        cout << linha << endl;
                        cerr << "ERRO SINTÁTICO: FECHAR UM MODULO SEM ABRIR OUTRO " << linha << endl;
                    }
                } else if (instrucao.operacao == "PUBLIC") {
                    if(tem_modulo_aberto){
                        if(section_text_atual){
                            section_text += linha + "\n";
                        } else {
                            cout << linha << endl;
                            cerr << "ERRO SEMÂNTICO: DECLARAÇÃO DE PUBLIC FORA DA SECTION TEXT" << endl;
                        }
                    } else {
                        cout << linha << endl;
                        cerr << "ERRO SEMÂNTICO: DEFINIÇÃO DE PUBLIC FORA DE UM MÓDULO" << endl;
                    }

                } else if (instrucao.operacao == "ENDMACRO"){
                    if (!tem_macro_aberta){
                        cout << linha << endl;
                        cerr << "ERRO: TENTOU FECHAR UMA MACRO SEM ABRIR UMA NA LINHA" << endl;
                    }

                } else {
                    adiciona_linha(linha);
                }
            }
        } else if (resultado == "INSTRUCAO") {
            if (instrucao_valida(instrucao)) {
                if (section_text_atual) {
                    section_text += linha + "\n";
                } else {
                    cout << linha << endl;
                    cerr << "ERRO: DECLARAÇÃO DE COMANDOS FORA DA SECTION TEXT" << endl;
                }
            } 
        } else {
            if(MDT.find(instrucao.operacao) != MDT.end()) {
                if(!instrucao.operando1.empty()){
                    cout << linha << endl;
                    cerr << "ESSA DIRETIVA NÃO ACEITA OPERANDOS" << endl;
                } else {
                    if (section_text_atual) {
                        for (const string& linha_macro : MDT[instrucao.operacao]) {
                            section_text += linha_macro + "\n";
                        }
                    } else {
                        cout << linha << endl;
                        cerr << "ERRO SINTÁTICO: VOCÊ NÃO PODE USAR MACROS NA SECTION DATA";
                    }
                }
            } else {
                cout << linha << endl;
                cerr << "ERRO LÉXICO: INSTRUÇÃO OU DIRETIVA INVÁLIDA" << endl;
            }
        }
    }
}

// FUNÇÃO PRINCIPAL QUE LÊ TODO O ARQUIVO .ASM E RETORNA O ARQUIVO .PRE
void pre_processamento_macros(string arquivo_asm_nome) {
    string caminho_arquivo_asm = "../arquivos_asm/" + arquivo_asm_nome;  

    // Verifica se o arquivo .asm existe
    ifstream inputFile(caminho_arquivo_asm); 
    if (!inputFile.is_open()) {
        cerr << "O arquivo: " << arquivo_asm_nome << " Não existe" << endl;
        return;
    }

    string arquivo_pre_nome = regex_replace(arquivo_asm_nome, regex("\\.asm$"), ".pre");
    string caminho_arquivo_pre = "../arquivos_pre/" + arquivo_pre_nome;
    
    // Verifica se o arquivo de saída foi criado
    ofstream outputFile(caminho_arquivo_pre); 
    if (!outputFile.is_open()) {
        cerr << "Erro: Não foi possível criar o arquivo de saída " << arquivo_pre_nome << endl;
        return;
    }

    // Processa o arquivo linha por linha
    while (getline(inputFile, linha)) {
        if (!linha.empty()){
            string linha_formatada = formata_linha(linha);

            if (linha_formatada.find("COPY") != std::string::npos){
                Instrucao instrucao = criar_instrucao(linha_formatada);

                if (instrucao.operando1.find(',') != std::string::npos) {
                    for (size_t i = 0; i < instrucao.operando1.length(); ++i) {
                        if (instrucao.operando1[i] == ',') {
                            instrucao.operando1[i] = ' ';
                        }
                    }
                    if (instrucao.rotulo.empty()){
                        linha_formatada = instrucao.operacao + " " + instrucao.operando1 + " " + instrucao.operando2;
                    } else {
                        linha_formatada = instrucao.rotulo + ": " + instrucao.operacao + " " + instrucao.operando1 + " " + instrucao.operando2;
                    }

                } else {
                    cout << linha_formatada << endl;
                    cerr << "ERRO SINTÁTICO: A FORMA CORRETA É COPY OPERANDO1,OPERANDO2" << endl;
                }
            }

            Instrucao instrucao = criar_instrucao(linha_formatada);
            processa_linha(linha_formatada, instrucao, inputFile, outputFile);
        }
    }

    if (section_text.empty()) {
        cerr << "ERRO SINTÁTICO: SECTION TEXT VAZIA" << endl;
    } else {
        outputFile << "SECTION TEXT\n" << section_text;
    }

    if (!section_data.empty()) {
        outputFile << "SECTION DATA\n" << section_data;
    }

    if(ja_existe_modulo){
        if(modulo_finalizado){
            outputFile << "END";
        } else {
            cerr << "MÓDULO NÃO FOI FINALIZADO" << endl;
        }
    }


    inputFile.close();
    outputFile.close();

    cout << "Arquivo pré-processado gerado com sucesso: " << arquivo_pre_nome << endl;
}