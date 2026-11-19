#include "persistencia/ArquivoConfiguracoes.h"

#include <algorithm>
#include <fstream>
#include <string>

namespace
{
bool ler_inteiro(const std::string &texto, int minimo, int maximo, int &valor)
{
    if (texto.empty() || texto.size() > 3 ||
        !std::all_of(texto.begin(), texto.end(), [](char c) { return c >= '0' && c <= '9'; }))
    {
        return false;
    }
    const int lido = std::stoi(texto);
    if (lido < minimo || lido > maximo)
    {
        return false;
    }
    valor = lido;
    return true;
}
} // namespace

ArquivoConfiguracoes::ArquivoConfiguracoes(const std::filesystem::path &caminho) : caminho(caminho)
{
}

Configuracoes ArquivoConfiguracoes::carregar() const
{
    Configuracoes configuracoes;
    std::ifstream arquivo(this->caminho);
    std::string linha;
    while (arquivo && std::getline(arquivo, linha))
    {
        const auto separador = linha.find('=');
        if (separador == std::string::npos)
        {
            continue;
        }
        const std::string chave = linha.substr(0, separador);
        const std::string valor = linha.substr(separador + 1);
        int numero = 0;
        if (chave == "volume_musica" && ler_inteiro(valor, 0, Configuracoes::VOLUME_MAXIMO, numero))
        {
            configuracoes.volumeMusica = numero;
        }
        else if (chave == "volume_efeitos" &&
                 ler_inteiro(valor, 0, Configuracoes::VOLUME_MAXIMO, numero))
        {
            configuracoes.volumeEfeitos = numero;
        }
        else if (chave == "tela_cheia" && ler_inteiro(valor, 0, 1, numero))
        {
            configuracoes.telaCheia = numero == 1;
        }
        else if (chave == "mostrar_hitboxes" && ler_inteiro(valor, 0, 1, numero))
        {
            configuracoes.mostrarHitboxes = numero == 1;
        }
    }
    return configuracoes;
}

bool ArquivoConfiguracoes::salvar(const Configuracoes &configuracoes) const
{
    std::error_code erro;
    if (!this->caminho.parent_path().empty())
    {
        std::filesystem::create_directories(this->caminho.parent_path(), erro);
        if (erro)
        {
            return false;
        }
    }
    // Mesmo cuidado dos saves: grava ao lado e so entao substitui o original.
    auto temporario = this->caminho;
    temporario += ".tmp";
    {
        std::ofstream arquivo(temporario, std::ios::trunc);
        if (!arquivo)
        {
            return false;
        }
        arquivo << "volume_musica=" << configuracoes.volumeMusica << "\n"
                << "volume_efeitos=" << configuracoes.volumeEfeitos << "\n"
                << "tela_cheia=" << (configuracoes.telaCheia ? 1 : 0) << "\n"
                << "mostrar_hitboxes=" << (configuracoes.mostrarHitboxes ? 1 : 0) << "\n";
        arquivo.flush();
        if (!arquivo.good())
        {
            std::filesystem::remove(temporario, erro);
            return false;
        }
    }
    std::filesystem::rename(temporario, this->caminho, erro);
    if (erro)
    {
        std::filesystem::remove(temporario, erro);
        return false;
    }
    return true;
}
