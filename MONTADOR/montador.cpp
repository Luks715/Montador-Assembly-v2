#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <cstdlib>
#include "pre_processamento.cpp"
#include "montagem.cpp"

using namespace std;

// FUNÇÃO MAIN DO MONTADOR, RECEBE O NOME DO ARQUIVO COMO ARGUMENTO
// SE FOR UM ARQUIVO .ASM CHAMA A FUNÇÃO PRE_PROCESSAMENTO em src/MONTADOR/pre_processamento.cpp
// SE FOR UM ARQUIVO .PRE CHAMA A FUNÇÃO MONTAGEM src/MONTADOR/montagem.cpp
int main(int argc, char *argv[]) {
    if (argc != 2) {
        cerr << "Uso: ./montador <arquivo.asm ou arquivo.pre>" << endl;
        return EXIT_FAILURE;
    }

    string arquivo_nome = argv[1];

    if (regex_match(arquivo_nome, regex(".*\\.asm$"))) {
        pre_processamento(arquivo_nome);
    } else if (regex_match(arquivo_nome, regex(".*\\.pre$"))) {
        montagem(arquivo_nome);
    } else {
        cerr << "Erro: Extensão do arquivo inválida. Use um arquivo .asm ou .pre" << endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}