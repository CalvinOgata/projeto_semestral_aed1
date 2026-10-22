#include "persistencia/ArquivoProgresso.h"

#include <charconv>
#include <fstream>
#include <iterator>
#include <cctype>
#include <set>

#include "config/CampanhaConfig.h"

namespace
{
// Leitor especifico dos esquemas v1 e v2. Rejeita chaves duplicadas ou
// desconhecidas, lixo ao final, numeros fora de faixa e JSON truncado.
class LeitorJson
{
private:
    const std::string &texto;
    std::size_t posicao = 0;

public:
    explicit LeitorJson(const std::string &texto) : texto(texto)
    {
    }

    void pular_espacos()
    {
        while (posicao < texto.size() && std::isspace(static_cast<unsigned char>(texto[posicao])))
        {
            ++posicao;
        }
    }

    bool consumir(char esperado)
    {
        pular_espacos();
        if (posicao < texto.size() && texto[posicao] == esperado)
        {
            ++posicao;
            return true;
        }
        return false;
    }

    bool proximo_e(char esperado)
    {
        pular_espacos();
        return posicao < texto.size() && texto[posicao] == esperado;
    }

    bool fim()
    {
        pular_espacos();
        return posicao == texto.size();
    }

    // Chave entre aspas seguida de ':'; o esquema nao usa escapes.
    bool ler_chave(std::string &chave)
    {
        if (!consumir('"'))
        {
            return false;
        }
        const auto fimChave = texto.find('"', posicao);
        if (fimChave == std::string::npos)
        {
            return false;
        }
        chave = texto.substr(posicao, fimChave - posicao);
        posicao = fimChave + 1;
        return consumir(':');
    }

    // Inteiro nao negativo, sem zeros a esquerda.
    bool ler_inteiro(int &valor)
    {
        pular_espacos();
        const std::size_t inicio = posicao;
        while (posicao < texto.size() && texto[posicao] >= '0' && texto[posicao] <= '9')
        {
            ++posicao;
        }
        const std::size_t tamanho = posicao - inicio;
        if (tamanho == 0 || (tamanho > 1 && texto[inicio] == '0'))
        {
            return false;
        }
        const auto resultado = std::from_chars(texto.data() + inicio, texto.data() + posicao, valor);
        return resultado.ec == std::errc();
    }
};

// Objeto {"tipo": n, "municao": n}, com as duas chaves exatamente uma vez.
bool ler_arma(LeitorJson &leitor, ArmaSalva &arma)
{
    if (!leitor.consumir('{'))
    {
        return false;
    }
    std::set<std::string> lidas;
    do
    {
        std::string chave;
        if (!leitor.ler_chave(chave) || !lidas.insert(chave).second)
        {
            return false;
        }
        int *destino = chave == "tipo" ? &arma.tipo : (chave == "municao" ? &arma.municao : nullptr);
        if (destino == nullptr || !leitor.ler_inteiro(*destino))
        {
            return false;
        }
    } while (leitor.consumir(','));
    return leitor.consumir('}') && lidas.size() == 2 && arma.tipo < QUANTIDADE_TIPOS_ARMA;
}

bool ler_armas(LeitorJson &leitor, std::vector<ArmaSalva> &armas)
{
    if (!leitor.consumir('['))
    {
        return false;
    }
    if (leitor.consumir(']'))
    {
        return true;
    }
    do
    {
        ArmaSalva arma;
        if (static_cast<int>(armas.size()) == CAPACIDADE_INVENTARIO || !ler_arma(leitor, arma))
        {
            return false;
        }
        armas.push_back(arma);
    } while (leitor.consumir(','));
    return leitor.consumir(']');
}

// v1: versao e fases_concluidas. v2: tambem exige armas.
bool ler_progresso(const std::string &texto, ProgressoSalvo &progresso)
{
    LeitorJson leitor(texto);
    if (!leitor.consumir('{'))
    {
        return false;
    }
    std::set<std::string> lidas;
    int versao = 0;
    do
    {
        std::string chave;
        if (!leitor.ler_chave(chave) || !lidas.insert(chave).second)
        {
            return false;
        }
        const bool lido = chave == "versao" ? leitor.ler_inteiro(versao) :
            chave == "fases_concluidas" ? leitor.ler_inteiro(progresso.fasesConcluidas) :
            chave == "armas" ? ler_armas(leitor, progresso.armas) : false;
        if (!lido)
        {
            return false;
        }
    } while (leitor.consumir(','));
    if (!leitor.consumir('}') || !leitor.fim() || !lidas.count("versao") ||
        !lidas.count("fases_concluidas"))
    {
        return false;
    }
    return (versao == 1 && lidas.size() == 2) || (versao == 2 && lidas.size() == 3);
}
}

ArquivoProgresso::ArquivoProgresso(const std::filesystem::path &caminho) : caminho(caminho)
{
}

bool ArquivoProgresso::carregar(ProgressoSalvo &progresso, std::string &mensagem) const
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
    ProgressoSalvo lido;
    if (arquivo.bad() || !ler_progresso(texto, lido) || lido.fasesConcluidas > QUANTIDADE_FASES)
    {
        mensagem = "SAVE INVALIDO OU INCOMPATIVEL";
        return false;
    }
    progresso = lido;
    const int fasesConcluidas = progresso.fasesConcluidas;
    mensagem = fasesConcluidas == QUANTIDADE_FASES ? "CAMPANHA SALVA CONCLUIDA" :
        "CONTINUAR NA FASE " + std::to_string(fasesConcluidas + 1) + "/" +
        std::to_string(QUANTIDADE_FASES);
    return true;
}

bool ArquivoProgresso::salvar(const ProgressoSalvo &progresso, std::string &mensagem) const
{
    mensagem = "FALHA AO SALVAR - F5: TENTAR NOVAMENTE";
    if (progresso.fasesConcluidas < 0 || progresso.fasesConcluidas > QUANTIDADE_FASES ||
        static_cast<int>(progresso.armas.size()) > CAPACIDADE_INVENTARIO)
    {
        return false;
    }
    for (const auto &arma : progresso.armas)
    {
        if (arma.tipo < 0 || arma.tipo >= QUANTIDADE_TIPOS_ARMA || arma.municao < 0)
        {
            return false;
        }
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
    arquivo << "{\n  \"versao\": 2,\n  \"fases_concluidas\": " << progresso.fasesConcluidas
            << ",\n  \"armas\": [";
    for (std::size_t i = 0; i < progresso.armas.size(); ++i)
    {
        arquivo << (i == 0 ? "\n" : ",\n") << "    {\"tipo\": " << progresso.armas[i].tipo
                << ", \"municao\": " << progresso.armas[i].municao << "}";
    }
    arquivo << (progresso.armas.empty() ? "]\n}\n" : "\n  ]\n}\n");
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
