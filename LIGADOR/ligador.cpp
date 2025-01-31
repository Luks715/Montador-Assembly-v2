#include <iostream>
#include <string>
#include "ligar.cpp"

// FUNÇÃO MAIN DO LIGADOR, RECEBE OS NOMES DOS ARQUIVOS E CHAMA A FUNÇÃO LIGAMENTO EM src/LIGADOR/ligamento.cpp
int main(int argc, char* argv[]) {
    if (argc != 3) {
        std::cerr << "Erro: número incorreto de argumentos." << std::endl;
        std::cerr << "Uso correto: ./ligador prog1.obj prog2.obj" << std::endl;
        return EXIT_FAILURE;
    } else {
        std::string arquivo1 = argv[1];
        std::string arquivo2 = argv[2];

        regex arquivo_obj(".*\\.obj$");

        if (regex_match(arquivo1, regex(".*\\.obj$"))) {
            if (regex_match(arquivo2, regex(".*\\.obj$"))) {
                // CRIA UM VETOR DE STRINGS COM OS NOMES DOS ARQUIVOS PASSADOS E CHAMA A FUNÇÃO LIGAMENTO
                std::vector<std::string> args(argv, argv + argc);
                args.erase(args.begin());
                ligamento(args);
            } else {
                cerr << "Erro: Extensão do arquivo2 inválida. Use um arquivo .obj" << endl;
                return EXIT_FAILURE;
            }
        } else {
            cerr << "Erro: Extensão do arquivo1 inválida. Use um arquivo .obj" << endl;
            return EXIT_FAILURE;
        }

        return EXIT_SUCCESS;
    }
}

