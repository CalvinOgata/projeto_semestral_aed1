#ifndef ARQUIVO_CONFIGURACOES_H
#define ARQUIVO_CONFIGURACOES_H

#include <filesystem>

// Preferencias do jogador, independentes dos slots de progresso.
struct Configuracoes
{
    static constexpr int VOLUME_MAXIMO = 10;
    int volumeMusica = 7;
    int volumeEfeitos = 8;
    bool telaCheia = false;
    bool mostrarHitboxes = false;
};

// Arquivo texto simples "chave=valor". Linhas desconhecidas ou invalidas sao
// ignoradas e mantem o valor padrao, para um arquivo antigo nunca travar o jogo.
class ArquivoConfiguracoes
{
private:
    std::filesystem::path caminho;

public:
    explicit ArquivoConfiguracoes(const std::filesystem::path &caminho);
    Configuracoes carregar() const;
    bool salvar(const Configuracoes &configuracoes) const;
};

#endif
