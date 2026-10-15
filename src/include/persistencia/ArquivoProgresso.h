#ifndef ARQUIVO_PROGRESSO_H
#define ARQUIVO_PROGRESSO_H

#include <filesystem>
#include <string>

// Formato JSON versionado, independente de SDL e das entidades em memoria.
// Salva somente o checkpoint de fase; nao serializa uma sala em andamento.
class ArquivoProgresso
{
private:
    std::filesystem::path caminho;

public:
    explicit ArquivoProgresso(const std::filesystem::path &caminho);
    bool carregar(int &fasesConcluidas, std::string &mensagem) const;
    bool salvar(int fasesConcluidas, std::string &mensagem) const;
};

#endif
