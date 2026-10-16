#include "interface/Minimapa.h"

#include <algorithm>
#include <string>
#include <vector>

#include "config/Config.h"
#include "interface/Texto.h"

void Minimapa::desenhar(SDL_Renderer *renderizador, const Fase &fase, int numero,
                       const Jogador &jogador)
{
    if (renderizador == nullptr || fase.get_quantidadeSalas() == 0)
    {
        return;
    }
    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(renderizador, &anterior);
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, 6, 10, 18, 205);
    SDL_RenderFillRect(renderizador, nullptr);
    const SDL_Rect painel = {100, 38, 760, 464};
    SDL_SetRenderDrawColor(renderizador, 16, 25, 39, 250);
    SDL_RenderFillRect(renderizador, &painel);
    SDL_SetRenderDrawColor(renderizador, 86, 124, 154, 255);
    SDL_RenderDrawRect(renderizador, &painel);
    Texto::desenhar_centralizado(renderizador, "MAPA - FASE " + std::to_string(numero),
                                LARGURA_TELA / 2, 56, 2, {235, 244, 255, 255});

    // Percorre as conexoes reais para posicionar as salas, inclusive nas fases lineares.
    const auto quantidade = fase.get_quantidadeSalas();
    std::vector<SDL_Point> posicoes(quantidade, {0, 0});
    std::vector<bool> visitadas(quantidade, false);
    std::vector<std::size_t> pendentes = {0};
    visitadas[0] = true;
    const SDL_Point deslocamentos[] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
    for (std::size_t inicio = 0; inicio < pendentes.size(); ++inicio)
    {
        const auto indice = pendentes[inicio];
        for (int d = 0; d < 4; ++d)
        {
            const int vizinho = fase.get_vizinho(indice, static_cast<Direcao>(d));
            if (vizinho < 0 || static_cast<std::size_t>(vizinho) >= quantidade || visitadas[vizinho])
            {
                continue;
            }
            posicoes[vizinho] = {posicoes[indice].x + deslocamentos[d].x,
                                posicoes[indice].y + deslocamentos[d].y};
            visitadas[vizinho] = true;
            pendentes.push_back(static_cast<std::size_t>(vizinho));
        }
    }
    int minX = 0, maxX = 0, minY = 0, maxY = 0;
    for (const auto &posicao : posicoes)
    {
        minX = std::min(minX, posicao.x);
        maxX = std::max(maxX, posicao.x);
        minY = std::min(minY, posicao.y);
        maxY = std::max(maxY, posicao.y);
    }
    const float passoX = std::min(180.0f, 580.0f / std::max(1, maxX - minX));
    const float passoY = std::min(100.0f, 270.0f / std::max(1, maxY - minY));
    auto centro = [&](std::size_t indice)
    {
        return SDL_FPoint{480 + (posicoes[indice].x - (minX + maxX) / 2.0f) * passoX,
                          265 + (posicoes[indice].y - (minY + maxY) / 2.0f) * passoY};
    };
    SDL_SetRenderDrawColor(renderizador, 104, 132, 158, 255);
    for (std::size_t i = 0; i < quantidade; ++i)
    {
        for (int d = 0; d < 4; ++d)
        {
            const int vizinho = fase.get_vizinho(i, static_cast<Direcao>(d));
            if (vizinho <= static_cast<int>(i) || static_cast<std::size_t>(vizinho) >= quantidade)
            {
                continue;
            }
            const auto a = centro(i), b = centro(static_cast<std::size_t>(vizinho));
            SDL_RenderDrawLineF(renderizador, a.x, a.y, b.x, b.y);
        }
    }
    for (std::size_t i = 0; i < quantidade; ++i)
    {
        const auto &sala = fase.get_sala(i);
        const auto &config = sala.get_configuracao();
        const auto posicao = centro(i);
        const bool atual = i == fase.get_indiceAtual();
        const SDL_FRect caixa = {posicao.x - 60, posicao.y - 30, 120, 60};
        const SDL_Color cor = sala.liberada() ? SDL_Color{36, 92, 73, 255} :
            (sala.get_estado() == EstadoSala::NAO_VISITADA ? SDL_Color{42, 52, 69, 255} :
                                                          SDL_Color{119, 54, 62, 255});
        SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
        SDL_RenderFillRectF(renderizador, &caixa);
        SDL_SetRenderDrawColor(renderizador, atual ? 255 : 117, atual ? 213 : 147,
                               atual ? 92 : 173, 255);
        SDL_RenderDrawRectF(renderizador, &caixa);
        if (atual)
        {
            const SDL_FRect destaque = {caixa.x - 3, caixa.y - 3, caixa.w + 6, caixa.h + 6};
            SDL_RenderDrawRectF(renderizador, &destaque);
        }
        const std::string nome = config.mapaRamificado ? config.nome.substr(0, config.nome.find(" - ")) :
                                                                       "SALA " + std::to_string(i + 1);
        Texto::desenhar_centralizado(renderizador, nome, static_cast<int>(posicao.x),
                                    static_cast<int>(caixa.y + 7), 1, {235, 244, 255, 255});
        if (config.baus > 0 || config.boss)
        {
            Texto::desenhar_centralizado(renderizador, config.boss ? "BOSS" : "BAUS",
                                        static_cast<int>(posicao.x), static_cast<int>(caixa.y + 20),
                                        1, {212, 194, 155, 255});
        }
        if (atual)
        {
            // Ponto proporcional a posicao real dentro da sala, abaixo de seu rotulo.
            const float x = std::clamp(jogador.get_centroX() / config.largura, 0.0f, 1.0f);
            const float y = std::clamp(jogador.get_centroY() / config.altura, 0.0f, 1.0f);
            const SDL_FRect ponto = {caixa.x + 8 + x * 98, caixa.y + 32 + y * 14, 6, 6};
            SDL_SetRenderDrawColor(renderizador, 255, 222, 110, 255);
            SDL_RenderFillRectF(renderizador, &ponto);
        }
    }
    Texto::desenhar_centralizado(renderizador, "AMARELO: VOCE   VERDE: CONCLUIDA", 480, 445,
                                1, {226, 235, 247, 255});
    Texto::desenhar_centralizado(renderizador, "CINZA: NAO VISITADA   VERMELHO: EM COMBATE", 480, 460,
                                1, {183, 199, 218, 255});
    Texto::desenhar_centralizado(renderizador, "M / ESC: FECHAR   -   JOGO PAUSADO", 480, 480,
                                1, {255, 222, 110, 255});
    SDL_SetRenderDrawBlendMode(renderizador, anterior);
}
