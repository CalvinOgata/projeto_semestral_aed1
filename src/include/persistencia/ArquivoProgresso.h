#ifndef ARQUIVO_PROGRESSO_H
#define ARQUIVO_PROGRESSO_H

#include <filesystem>
#include <string>
#include <vector>

// Arma do inventario em forma neutra: o indice de TipoArma e a municao restante.
struct ArmaSalva
{
    int tipo = 0;
    int municao = 0;
};

// Checkpoint de fase. Armas vazias significam o inventario inicial (save v1).
struct ProgressoSalvo
{
    int fasesConcluidas = 0;
    std::vector<ArmaSalva> armas;
};

// Formato JSON versionado, independente de SDL e das entidades em memoria.
// Salva somente o checkpoint de fase; nao serializa uma sala em andamento.
class ArquivoProgresso
{
private:
    std::filesystem::path caminho;

public:
    explicit ArquivoProgresso(const std::filesystem::path &caminho);
    bool carregar(ProgressoSalvo &progresso, std::string &mensagem) const;
    bool salvar(const ProgressoSalvo &progresso, std::string &mensagem) const;
};

#endif
