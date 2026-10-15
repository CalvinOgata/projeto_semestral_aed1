#include "persistencia/ArquivoProgresso.h"

#include <charconv>
#include <fstream>
#include <iterator>
#include <map>
#include <sstream>

#include "config/CampanhaConfig.h"

namespace
{
// Le apenas o esquema plano de inteiros do save. Rejeita chaves duplicadas,
// campos desconhecidos, lixo ao final, numeros fora de faixa e JSON truncado.
bool ler_campos(const std::string &texto, std::map<std::string, int> &campos)
{
    std::istringstream entrada(texto);
    char c;
    if (!(entrada >> c) || c != '{')
    {
        return false;
    }
    for (;;)
    {
        if (!(entrada >> c) || c != '"')
        {
            return false;
        }
        std::string chave;
        if (!std::getline(entrada, chave, '"'))
        {
            return false;
        }
        if (chave != "versao" && chave != "fases_concluidas")
        {
            return false;
        }
        if (!(entrada >> c) || c != ':')
        {
            return false;
        }
        entrada >> std::ws;
        std::string numero;
        while (entrada.peek() >= '0' && entrada.peek() <= '9')
        {
            numero += static_cast<char>(entrada.get());
        }
        if (numero.empty() || (numero.size() > 1 && numero[0] == '0'))
        {
            return false;
        }
        int valor = 0;
        const auto resultado = std::from_chars(numero.data(), numero.data() + numero.size(), valor);
        if (resultado.ec != std::errc() || !campos.emplace(chave, valor).second)
        {
            return false;
        }
        if (!(entrada >> c))
        {
            return false;
        }
        if (c == '}')
        {
            break;
        }
        if (c != ',')
        {
            return false;
        }
    }
    entrada >> std::ws;
    return entrada.eof() && campos.size() == 2;
}
}

ArquivoProgresso::ArquivoProgresso(const std::filesystem::path &caminho) : caminho(caminho)
{
}

bool ArquivoProgresso::carregar(int &fasesConcluidas, std::string &mensagem) const
{
    std::error_code erro;
    if (!std::filesystem::exists(this->caminho, erro) && !erro)
    {
        mensagem = "NENHUM JOGO SALVO";
        return false;
    }
    const auto tamanho = std::filesystem::file_size(this->caminho, erro);
    if (erro || tamanho > 4096)
    {
        mensagem = "NAO FOI POSSIVEL LER O SAVE";
        return false;
    }
    std::ifstream arquivo(this->caminho);
    if (!arquivo)
    {
        mensagem = "NAO FOI POSSIVEL LER O SAVE";
        return false;
    }
    const std::string texto((std::istreambuf_iterator<char>(arquivo)), {});
    std::map<std::string, int> campos;
    if (arquivo.bad() || !ler_campos(texto, campos) || campos["versao"] != 1 ||
        campos["fases_concluidas"] > QUANTIDADE_FASES)
    {
        mensagem = "SAVE INVALIDO OU INCOMPATIVEL";
        return false;
    }
    fasesConcluidas = campos["fases_concluidas"];
    mensagem = fasesConcluidas == QUANTIDADE_FASES ? "CAMPANHA SALVA CONCLUIDA" :
        "CONTINUAR NA FASE " + std::to_string(fasesConcluidas + 1) + "/5";
    return true;
}

bool ArquivoProgresso::salvar(int fasesConcluidas, std::string &mensagem) const
{
    mensagem = "FALHA AO SALVAR - F5: TENTAR NOVAMENTE";
    if (fasesConcluidas < 0 || fasesConcluidas > QUANTIDADE_FASES)
    {
        return false;
    }
    std::error_code erro;
    if (!this->caminho.parent_path().empty())
    {
        std::filesystem::create_directories(this->caminho.parent_path(), erro);
        if (erro)
        {
            return false;
        }
    }
    auto temporario = this->caminho;
    temporario += ".tmp";
    std::ofstream arquivo(temporario, std::ios::trunc);
    if (!arquivo)
    {
        return false;
    }
    arquivo << "{\n  \"versao\": 1,\n  \"fases_concluidas\": " << fasesConcluidas << "\n}\n";
    arquivo.flush();
    const bool escrito = arquivo.good();
    arquivo.close();
    if (!escrito || arquivo.fail())
    {
        return false;
    }
    // Mesmo diretorio: rename substitui o save somente apos a escrita completa.
    std::filesystem::rename(temporario, this->caminho, erro);
    if (erro)
    {
        return false;
    }
    mensagem = "PROGRESSO SALVO";
    return true;
}
