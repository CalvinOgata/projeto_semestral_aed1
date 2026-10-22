#include "mundo/Fase.h"

#include <algorithm>
#include <array>
#include <iterator>
#include <string>
#include "config/Config.h"
#include <stdexcept>

#include "config/CampanhaConfig.h"

namespace
{
// Planta autoral de uma sala: posicao na grade e conexoes {esquerda, direita, cima, baixo}.
struct PlantaSala
{
    const char *nome;
    int coluna, linha;
    int virus, brutos, baus;
    bool boss;
    std::array<int, 4> vizinhos;
    // 1 quadrantes, 2 trincheira, 3 fortaleza (boss), 4 labirinto, 5 deposito (baus).
    int cobertura;
};

// Fase 1, dos compiladores: lobby, combate inicial, tesouro, dois ramos e boss ao norte.
const PlantaSala PLANTA_FASE_1[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, {{-1, 1, -1, -1}}, 0},
    {"A - PRE-PROCESSADOR", 1, 0, 3, 0, 0, false, {{0, 3, 4, 2}}, 1},
    {"B - CACHE DE BUILD", 1, 1, 0, 0, 1, false, {{-1, -1, 1, -1}}, 5},
    {"C - ANALISE LEXICA", 2, 0, 4, 2, 0, false, {{1, -1, 5, -1}}, 2},
    {"D - PARSER", 1, -1, 4, 2, 0, false, {{-1, 5, -1, 1}}, 4},
    {"E - LINKER", 2, -1, 0, 0, 2, false, {{4, -1, 6, 3}}, 5},
    {"F - COMPILADOR FECHADO", 2, -2, 0, 0, 0, true, {{-1, -1, -1, 5}}, 3},
};

// Fase 2, da rede: corredor principal para leste com desvios ao norte e ao sul.
const PlantaSala PLANTA_FASE_2[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, {{-1, 1, -1, -1}}, 0},
    {"A - ROTEADOR", 1, 0, 4, 0, 0, false, {{0, 4, 2, 3}}, 1},
    {"B - CACHE DNS", 1, -1, 0, 0, 1, false, {{-1, -1, -1, 1}}, 5},
    {"C - CAIXA DE SPAM", 1, 1, 5, 1, 0, false, {{-1, 5, 1, -1}}, 2},
    {"D - PROXY", 2, 0, 4, 2, 0, false, {{1, 6, -1, -1}}, 4},
    {"E - BOTNET", 2, 1, 6, 2, 0, false, {{3, 7, -1, -1}}, 2},
    {"F - PING 999", 3, 0, 5, 3, 0, false, {{4, -1, 8, 7}}, 4},
    {"G - LOOT DROP", 3, 1, 0, 0, 2, false, {{5, -1, 6, -1}}, 5},
    {"H - REDE ANTISSOCIAL", 3, -1, 0, 0, 0, true, {{-1, -1, -1, 6}}, 3},
};

// Fase 3, do kernel: subida vertical a partir do lobby, com alas laterais e boss no topo.
const PlantaSala PLANTA_FASE_3[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, {{-1, -1, 1, -1}}, 0},
    {"A - KERNEL PANIC", 0, -1, 5, 1, 0, false, {{2, 3, -1, 0}}, 1},
    {"B - ROOTKIT", -1, -1, 6, 2, 0, false, {{-1, 1, 4, -1}}, 4},
    {"C - RANSOMWARE", 1, -1, 6, 2, 0, false, {{1, 6, 5, -1}}, 2},
    {"D - SPYWARE", -1, -2, 7, 2, 0, false, {{-1, 7, -1, 2}}, 2},
    {"E - WORM", 1, -2, 7, 2, 0, false, {{7, 8, -1, 3}}, 4},
    {"F - COFRE DE MEMORIA", 2, -1, 0, 0, 1, false, {{3, -1, -1, -1}}, 5},
    {"G - DEADLOCK", 0, -2, 6, 3, 0, false, {{4, 5, 9, -1}}, 1},
    {"H - SAVE POINT", 2, -2, 0, 0, 2, false, {{5, -1, -1, -1}}, 5},
    {"I - PROCESSOS ZUMBIS", 0, -3, 6, 4, 0, false, {{-1, -1, 10, 7}}, 4},
    {"J - TELA AZUL DA MORTE", 0, -4, 2, 0, 0, true, {{-1, -1, -1, 9}}, 3},
};

// Cada fase mistura um tema de computacao com uma referencia de jogos.
struct PlantaFase
{
    const char *nome;
    const char *subtitulo;
    const PlantaSala *salas;
    std::size_t quantidade;
};

const PlantaFase PLANTAS[] = {
    {"FASE DOS COMPILADORES", "SPEEDRUN DO BUILD", PLANTA_FASE_1, std::size(PLANTA_FASE_1)},
    {"FASE DA REDE", "MULTIPLAYER COM LAG", PLANTA_FASE_2, std::size(PLANTA_FASE_2)},
    {"FASE DO KERNEL", "GAME OVER EM TELA AZUL", PLANTA_FASE_3, std::size(PLANTA_FASE_3)},
};
static_assert(std::size(PLANTAS) == QUANTIDADE_FASES, "Cada fase precisa de uma planta");
}

Fase::Fase(int numero) : numero(numero)
{
    if (numero < 1 || numero > QUANTIDADE_FASES)
    {
        throw std::invalid_argument("Numero de fase invalido");
    }
    const PlantaFase &planta = PLANTAS[numero - 1];
    const float largura = 936, altura = 516;
    for (std::size_t i = 0; i < planta.quantidade; ++i)
    {
        const PlantaSala &sala = planta.salas[i];
        ConfiguracaoSala config{sala.nome, largura, altura, numero, sala.virus, {true}};
        config.brutos = sala.brutos;
        config.boss = sala.boss;
        config.portalSaida = sala.boss;
        config.baus = sala.baus;
        config.armasIniciais = false;
        config.vizinhos = sala.vizinhos;
        config.mapaRamificado = true;
        config.tema = numero;
        config.cobertura = sala.cobertura;
        salas.inserir(Sala(config));
        origens.inserir({sala.coluna * (largura + Sala::COMPRIMENTO_CORREDOR),
                        sala.linha * (altura + Sala::COMPRIMENTO_CORREDOR)});
    }
}

void Fase::reiniciar(Jogador &jogador, const FilaArmas &armas)
{
    *this = Fase(this->numero);
    jogador = Jogador();
    // Inventario vazio receberia uma fase sem como lutar; mantem a arma inicial.
    if (!armas.vazio())
    {
        jogador.get_armas() = armas;
    }
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

const char *Fase::get_nome() const
{
    return PLANTAS[numero - 1].nome;
}

const char *Fase::get_subtitulo() const
{
    return PLANTAS[numero - 1].subtitulo;
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
