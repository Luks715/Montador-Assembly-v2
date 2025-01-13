#include <iostream>
#include <fstream>
#include <string>
#include <regex>
#include <unordered_set>

using namespace std;

map<string, map<string, int>> tabelas_defs;
map<string, map<string, int>> tabelas_usos;
map<string, vector<string>> enderecos_relativos;
map<string, vector<string>> arquivos_obj;
map<string, int> fatores_de_correcao;

int busca_label(string label) {
    // Itera sobre cada mapa dentro de tabelas_defs
    for (const auto& map_pair : tabelas_defs) {
        const auto& map_labels = map_pair.second; // Acessa o mapa interno
        auto it = map_labels.find(label);
        if (it != map_labels.end()) {
            return it->second;
        }
    }

    // Se não encontrar a label, retorna um valor indicando que não foi encontrada (ou lance uma exceção)
    cout << "ERRO SEMÂNTIO: LABEL EXTERNA: " << label << " NÃO DEFINIDA EM NENHUM MÓDULO" << endl;
    return 0; // Valor padrão indicando erro
}


std::vector<std::string> divide_string(const std::string& str) {
    std::vector<std::string> partes;
    std::istringstream stream(str);
    std::string palavra;

    while (stream >> palavra) {
        partes.push_back(palavra); // Adiciona cada parte ao vetor
    }

    return partes;
}

void ligamento(const std::vector<std::string>& arquivos){
    for(int i = 0; i < arquivos.size(); i++){
        string caminho_arquivo_obj = "../teste_obj/" + arquivos[i];

        ifstream inputFile(caminho_arquivo_obj); 
        if (!inputFile.is_open()) {
            cerr << "O arquivo: " << caminho_arquivo_obj << " Não existe" << endl;
            return;
        }

        tabelas_usos.insert({arquivos[i], {}});
        tabelas_defs.insert({arquivos[i], {}});

        // PREENCHE AS TABELAS DE DEFINIÇÕES E USO DOS MÓDULOS COM BASE NO QUE FOI FORNECIDO PELO MONTADOR
        // SALVA OS VALORES RELATIVOS E O ARQUIVO MONTADO DE CADA ARQUIVO PASSADO
        string linha;
        while(getline(inputFile, linha)) {
            if (linha.find("D,") != std::string::npos) {
                vector<string> partes = divide_string(linha);
                auto& tabela_defs_i = tabelas_defs[arquivos[i]];

                tabela_defs_i[partes[1]] = std::stoi(partes[2]);

            } else if (linha.find("U,") != std::string::npos) {
                vector<string> partes = divide_string(linha);
                auto& tabela_usos_i = tabelas_usos[arquivos[i]];

                if (tabela_usos_i.empty()) {
                    tabela_usos_i[partes[1]] = std::stoi(partes[2]);
                } else {
                    if (tabela_usos_i.find(partes[1]) == tabela_usos_i.end()) {
                        tabela_usos_i[partes[1]] = stoi(partes[2]);
                    }
                }

            } else if (linha.find("R,") != std::string::npos) {
                vector<string> relativos = divide_string(linha);
                relativos.erase(relativos.begin());
                enderecos_relativos.insert({arquivos[i], relativos});

            } else {
                vector<string> arquivo_montado = divide_string(linha);
                arquivos_obj.insert({arquivos[i], arquivo_montado});
            }
        }
        inputFile.close();
    }

    // CALCULA O FATOR DE CORREÇÃO PARA CADA MÓDULO
    for (int i = 0; i < arquivos.size(); i++) {
        if (i == 0){
            fatores_de_correcao[arquivos[0]] = 0;
        } else {
            fatores_de_correcao[arquivos[i]] = fatores_de_correcao[arquivos[i - 1]] + arquivos_obj[arquivos[i - 1]].size();
        }
    }

    // SOMA O FATOR DE CORREÇÃO NAS TABELAS DE DEFINIÇÃO DE CADA MÓDULO
    for (int i = 0; i < arquivos.size(); i++) {
        auto& tabela_defs_i = tabelas_defs[arquivos[i]];

        for (auto& map_pair : tabela_defs_i) {
            map_pair.second += fatores_de_correcao[arquivos[i]];
        }
    }

    // SUBSTITUI OS VALORES DAS LABELS EXTERNAS NO CÓDIGO PELO NOVO VALOR DA TABELA DE DEFINIÇÃO
    for (int i = 0; i < arquivos_obj.size(); i++) {
        auto& tabela_usos_i = tabelas_usos[arquivos[i]];
        auto& arquivo_obj   = arquivos_obj[arquivos[i]];

        for (auto& map_pair : tabela_usos_i) {
            int novo_endereco = busca_label(map_pair.first);
            arquivo_obj[map_pair.second] = to_string(stoi(arquivo_obj[map_pair.second]) + novo_endereco);
        }
    }

    string arquivo_e_nome = regex_replace(arquivos[0], regex("\\.obj$"), ".e");
    string caminho_arquivo_e = "../teste_e/" + arquivo_e_nome;
    
    // VERIFICA SE O ARQUIVO DE SAÍDA FOI CRIADO
    ofstream outputFile(caminho_arquivo_e); 
    if (!outputFile.is_open()) {
        cerr << "Erro: Não foi possível criar o arquivo de saída " << arquivo_e_nome << endl;
        return;
    }

    for (const auto& arquivo_montado : arquivos_obj) {
        // Itera sobre todos os itens da lista no arquivo
        for (const auto& num : arquivo_montado.second) {
            outputFile << num << " ";
        }
    }
    outputFile << endl;
    outputFile.close();
}