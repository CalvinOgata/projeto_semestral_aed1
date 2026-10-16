#include "mundo/Fase.h"

#include <string>
#include <stdexcept>

#include "config/CampanhaConfig.h"

Fase::Fase(int numero) : numero(numero)
{
    if (numero < 1 || numero > QUANTIDADE_FASES)
    {
        throw std::invalid_argument("Numero de fase invalido");
    }
    if (numero == 1)
    {
        const std::array<std::array<int, 4>, 7> conexoes = {{
            {{-1, 1, -1, -1}}, {{0, 3, 4, 2}}, {{-1, -1, 1, -1}},
            {{1, -1, 5, -1}}, {{-1, 5, -1, 1}}, {{4, -1, 6, 3}},
            {{-1, -1, -1, 5}}
        }};
        const char *nomes[] = {"LOBBY", "A - VIRUS", "B - TESOURO", "C - QUARENTENA",
                               "D - FIREWALL", "E - ARSENAL", "F - BOSS"};
        for (int i = 0; i < 7; ++i)
        {
            ConfiguracaoSala config{nomes[i], 936, 516, 1,
                i == 1 ? 3 : ((i == 3 || i == 4) ? 4 : 0), {true}};
            config.brutos = (i == 3 || i == 4) ? 2 : 0;
            config.boss = i == 6;
            config.baus = i == 2 ? 1 : (i == 5 ? 2 : 0);
            config.armasIniciais = false;
            config.vizinhos = conexoes[i];
            config.mapaRamificado = true;
            salas.inserir(Sala(config));
        }
        return;
    }
    const int extra = numero - 1;
    // Fase autoral inicial. Cada registro pode ter tamanho, dificuldade e requisito proprios.
    this->salas.inserir(Sala({"SETOR DE BOOT", 936, 516, 1 + extra, 4 + extra * 2, {true}}));
    this->salas.inserir(Sala({"REDE INFECTADA", 1120, 640, 2 + extra, 7 + extra * 2, {true}}));
    this->salas.inserir(Sala({"NUCLEO EM QUARENTENA", 1440, 800, 3 + extra, 10 + extra * 2, {true}}));
}

void Fase::reiniciar(Jogador &jogador)
{
    *this = Fase(this->numero);
    jogador = Jogador();
    this->get_salaAtual().entrar(jogador);
}

bool Fase::atualizar_transicao(Jogador &jogador)
{
    Sala &atual = this->get_salaAtual();
    if (this->concluida || !atual.liberada())
    {
        return false;
    }
    if (numero == 1)
    {
        const bool liberar = salas.obter(1).liberada() && salas.obter(3).liberada() &&
                             salas.obter(4).liberada();
        for (auto &sala : salas) sala.liberar_boss(liberar);
        if (indiceAtual == 6 && atual.tocando_portal(jogador))
        {
            atual.sair();
            concluida = true;
            return true;
        }
        for (int d = 0; d < 4; ++d)
        {
            const auto direcao = static_cast<Direcao>(d);
            const int destino = atual.get_configuracao().vizinhos[d];
            if (destino < 0 || !atual.porta_liberada(direcao) ||
                !atual.tocando_porta(jogador, direcao)) continue;
            atual.sair();
            indiceAtual = static_cast<std::size_t>(destino);
            get_salaAtual().entrar(jogador, static_cast<Direcao>(d ^ 1));
            return true;
        }
        return false;
    }
    const bool direita = atual.tocando_porta(jogador, true);
    const bool esquerda = this->indiceAtual > 0 && atual.tocando_porta(jogador, false);
    if (!direita && !esquerda)
    {
        return false;
    }
    atual.sair();
    if (direita && this->indiceAtual + 1 == this->salas.get_quantidade())
    {
        this->concluida = true;
        return true;
    }
    if (direita)
    {
        ++this->indiceAtual;
    }
    else
    {
        --this->indiceAtual;
    }
    this->get_salaAtual().entrar(jogador, esquerda);
    return true;
}

Sala &Fase::get_salaAtual()
{
    return this->salas.obter(this->indiceAtual);
}

const Sala &Fase::get_salaAtual() const
{
    return this->salas.obter(this->indiceAtual);
}

std::size_t Fase::get_indiceAtual() const
{
    return this->indiceAtual;
}

bool Fase::get_concluida() const
{
    return this->concluida;
}

std::size_t Fase::get_quantidadeSalas() const
{
    return salas.get_quantidade();
}

const Sala &Fase::get_sala(std::size_t indice) const
{
    return salas.obter(indice);
}

int Fase::get_vizinho(std::size_t indice, Direcao direcao) const
{
    const auto &config = get_sala(indice).get_configuracao();
    if (config.mapaRamificado)
    {
        return config.vizinhos[static_cast<int>(direcao)];
    }
    if (direcao == Direcao::ESQUERDA && indice > 0)
    {
        return static_cast<int>(indice) - 1;
    }
    if (direcao == Direcao::DIREITA && indice + 1 < get_quantidadeSalas())
    {
        return static_cast<int>(indice) + 1;
    }
    return -1;
}

void Fase::atualizar(float dt, Jogador &jogador)
{
    for (std::size_t i = 0; i < salas.get_quantidade(); ++i)
    {
        if (i == indiceAtual) salas.obter(i).atualizar(dt, jogador);
        else salas.obter(i).envelhecer_coletas(dt);
    }
}
