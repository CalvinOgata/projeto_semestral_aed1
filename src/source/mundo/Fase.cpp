#include "mundo/Fase.h"

#include <string>
#include <algorithm>
#include "config/Config.h"
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
            config.portalSaida = i == 6;
            config.baus = i == 2 ? 1 : (i == 5 ? 2 : 0);
            config.armasIniciais = false;
            config.vizinhos = conexoes[i];
            config.mapaRamificado = true;
            salas.inserir(Sala(config));
            const int colunas[] = {0, 1, 1, 2, 1, 2, 2};
            const int linhas[] = {0, 0, 1, 0, -1, -1, -2};
            origens.inserir({colunas[i] * (936 + Sala::COMPRIMENTO_CORREDOR),
                            linhas[i] * (516 + Sala::COMPRIMENTO_CORREDOR)});
        }
        return;
    }
    const int extra = numero - 1;
    const char *nomes[] = {"SETOR DE BOOT", "REDE INFECTADA", "NUCLEO EM QUARENTENA"};
    const float larguras[] = {936, 1120, 1440}, alturas[] = {516, 640, 800};
    float x = 0;
    for (int i = 0; i < 3; ++i)
    {
        ConfiguracaoSala config{nomes[i], larguras[i], alturas[i], 1 + extra + i,
                               4 + i * 3 + extra * 2, {true}};
        config.mapaRamificado = true;
        config.vizinhos = {{i - 1, i == 2 ? -1 : i + 1, -1, -1}};
        config.portalSaida = i == 2;
        salas.inserir(Sala(config));
        origens.inserir({x, (516 - alturas[i]) / 2});
        x += larguras[i] + Sala::COMPRIMENTO_CORREDOR;
    }
}

void Fase::reiniciar(Jogador &jogador)
{
    *this = Fase(this->numero);
    jogador = Jogador();
    this->get_salaAtual().entrar(jogador);
    preparar_adjacentes();
}

bool Fase::atualizar_transicao(Jogador &jogador)
{
    Sala &atual = this->get_salaAtual();
    if (this->concluida || !atual.liberada())
    {
        return false;
    }
    preparar_adjacentes();
    if (atual.tocando_portal(jogador))
    {
        atual.sair();
        concluida = true;
        return true;
    }
    for (int d = 0; d < 4; ++d)
    {
        const auto direcao = static_cast<Direcao>(d);
        const int destino = get_vizinho(indiceAtual, direcao);
        if (destino < 0 || !atual.porta_liberada(direcao)) continue;
        const auto &config = atual.get_configuracao();
        const float comprimento = Sala::COMPRIMENTO_CORREDOR;
        const bool chegou = d == 0 ? jogador.get_x() <= -comprimento - jogador.get_largura() :
                            d == 1 ? jogador.get_x() >= config.largura + comprimento :
                            d == 2 ? jogador.get_y() <= -comprimento - jogador.get_altura() :
                                     jogador.get_y() >= config.altura + comprimento;
        if (!chegou) continue;
        const auto origem = origens.obter(indiceAtual), alvo = origens.obter(destino);
        const float x = jogador.get_x() + origem.x - alvo.x;
        const float y = jogador.get_y() + origem.y - alvo.y;
        atual.sair();
        indiceAtual = static_cast<std::size_t>(destino);
        get_salaAtual().ativar();
        jogador.set_posicao(x, y);
        get_salaAtual().limitar_jogador(jogador);
        preparar_adjacentes();
        return true;
    }
    return false;
}

void Fase::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool hitboxes) const
{
    const auto origem = origens.obter(indiceAtual);
    for (int camada = 0; camada < 2; ++camada)
    {
        for (std::size_t i = 0; i < salas.get_quantidade(); ++i)
        {
            const auto pos = origens.obter(i);
            Camera local;
            local.centralizar_em(camera.get_x() + origem.x - pos.x + LARGURA_TELA / 2.0f,
                                 camera.get_y() + origem.y - pos.y + ALTURA_TELA / 2.0f);
            if (camada == 0)
            {
                salas.obter(i).desenhar_corredores(renderizador, local);
            }
            else
            {
                salas.obter(i).desenhar(renderizador, local, i > 0, hitboxes);
            }
        }
    }
}

SDL_FRect Fase::get_limitesVisuais() const
{
    const auto origem = origens.obter(indiceAtual);
    float esquerda = 0, cima = 0, direita = 0, baixo = 0;
    for (std::size_t i = 0; i < salas.get_quantidade(); ++i)
    {
        const auto pos = origens.obter(i);
        const auto &config = salas.obter(i).get_configuracao();
        esquerda = std::min(esquerda, pos.x - 12);
        cima = std::min(cima, pos.y - 12);
        direita = std::max(direita, pos.x + config.largura + 12);
        baixo = std::max(baixo, pos.y + config.altura + 12);
    }
    return {esquerda - origem.x, cima - origem.y, direita - esquerda, baixo - cima};
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
    preparar_adjacentes();
}

void Fase::preparar_adjacentes()
{
    if (!get_salaAtual().liberada()) return;
    for (int d = 0; d < 4; ++d)
    {
        const int vizinho = get_vizinho(indiceAtual, static_cast<Direcao>(d));
        if (vizinho >= 0) salas.obter(static_cast<std::size_t>(vizinho)).preparar();
    }
}
