#include "interface/Minimapa.h"

#include <algorithm>
#include <string>
#include <vector>

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
constexpr float LARGURA_NO = 104.0f;
constexpr float ALTURA_NO = 34.0f;

std::vector<SDL_FPoint> calcular_posicoes(const Fase &fase)
{
    const auto quantidade = fase.get_quantidadeSalas();
    std::vector<SDL_FPoint> posicoes(quantidade);
    if (quantidade == 0) return posicoes;

    auto minimo = fase.get_origemSala(0);
    auto maximo = minimo;
    for (std::size_t i = 1; i < quantidade; ++i)
    {
        const auto origem = fase.get_origemSala(i);
        minimo.x = std::min(minimo.x, origem.x);
        minimo.y = std::min(minimo.y, origem.y);
        maximo.x = std::max(maximo.x, origem.x);
        maximo.y = std::max(maximo.y, origem.y);
    }

    // Mesmas coordenadas do cenario, sem ordenar irmaos por indice ou pre-ordem.
    // Escalas positivas preservam esquerda/direita e cima/baixo. Os limites da
    // fase inteira mantem os nos imoveis quando novos ramos sao revelados.
    const auto &config = fase.get_sala(0).get_configuracao();
    const float escalaX = maximo.x > minimo.x ?
        std::min(160.0f / (config.largura + Sala::COMPRIMENTO_CORREDOR),
                 640.0f / (maximo.x - minimo.x)) : 0.0f;
    const float escalaY = maximo.y > minimo.y ?
        std::min(62.0f / (config.altura + Sala::COMPRIMENTO_CORREDOR),
                 250.0f / (maximo.y - minimo.y)) : 0.0f;
    for (std::size_t i = 0; i < quantidade; ++i)
    {
        const auto origem = fase.get_origemSala(i);
        posicoes[i] = {480.0f + (origem.x - (minimo.x + maximo.x) / 2.0f) * escalaX,
                      128.0f + (origem.y - minimo.y) * escalaY};
    }
    return posicoes;
}

// Usa a porta real tanto para o texto quanto para as pontas do ramo desenhado.
int direcao_entre(const Fase &fase, std::size_t pai, std::size_t filho)
{
    for (int d = 0; d < 4; ++d)
    {
        if (fase.get_vizinho(pai, static_cast<Direcao>(d)) == static_cast<int>(filho))
            return d;
    }
    return -1;
}
}

void Minimapa::desenhar(SDL_Renderer *renderizador, const Fase &fase, const Jogador &jogador,
                        bool toque)
{
    if (renderizador == nullptr || fase.contar_salasDescobertas() == 0)
    {
        return;
    }
    (void)jogador;
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
    Texto::desenhar_centralizado(renderizador, std::string("MAPA - ") + fase.get_nome(),
                                LARGURA_TELA / 2, 56, 2, {235, 244, 255, 255});

    const auto ordem = fase.percorrer_salasDescobertas();
    const auto quantidadeTotal = fase.get_quantidadeSalas();
    const auto quantidadeDescoberta = fase.contar_salasDescobertas();
    Texto::desenhar_centralizado(
        renderizador,
        "SALAS NA ARVORE: " + std::to_string(quantidadeDescoberta) + "/" +
            std::to_string(quantidadeTotal),
        LARGURA_TELA / 2, 78, 1, {255, 222, 110, 255});

    const auto posicoes = calcular_posicoes(fase);
    SDL_SetRenderDrawColor(renderizador, 118, 148, 176, 255);
    for (const auto indiceFilho : ordem)
    {
        const int indicePai = fase.get_paiSalaDescoberta(indiceFilho);
        if (indicePai < 0) continue;
        const int direcao = direcao_entre(fase, static_cast<std::size_t>(indicePai), indiceFilho);
        if (direcao < 0) continue;
        auto pai = posicoes[static_cast<std::size_t>(indicePai)];
        auto filho = posicoes[indiceFilho];
        const float dx = direcao == 0 ? -LARGURA_NO / 2 : direcao == 1 ? LARGURA_NO / 2 : 0;
        const float dy = direcao == 2 ? -ALTURA_NO / 2 : direcao == 3 ? ALTURA_NO / 2 : 0;
        SDL_RenderDrawLineF(renderizador, pai.x + dx, pai.y + dy, filho.x - dx, filho.y - dy);
    }

    for (const auto indice : ordem)
    {
        const auto &sala = fase.get_sala(indice);
        const auto &config = sala.get_configuracao();
        const auto posicao = posicoes[indice];
        const bool atual = indice == fase.get_indiceAtual();
        const SDL_FRect caixa = {posicao.x - LARGURA_NO / 2.0f,
                                 posicao.y - ALTURA_NO / 2.0f, LARGURA_NO, ALTURA_NO};
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
            const SDL_FRect marcador = {caixa.x + caixa.w - 9, caixa.y + caixa.h - 9, 5, 5};
            SDL_SetRenderDrawColor(renderizador, 255, 222, 110, 255);
            SDL_RenderFillRectF(renderizador, &marcador);
        }
        const auto separador = config.nome.find(" - ");
        std::string nome = config.mapaRamificado ? config.nome.substr(0, separador) :
                                                 "SALA " + std::to_string(indice + 1);
        const int pai = fase.get_paiSalaDescoberta(indice);
        if (pai >= 0)
        {
            constexpr const char *DIRECOES[] = {"ESQ", "DIR", "CIMA", "BAIXO"};
            const int direcao = direcao_entre(fase, static_cast<std::size_t>(pai), indice);
            if (direcao >= 0)
            {
                nome += std::string(" - ") + DIRECOES[direcao];
            }
        }
        Texto::desenhar_centralizado(renderizador, nome, static_cast<int>(posicao.x),
                                    static_cast<int>(caixa.y + 5), 1,
                                    {235, 244, 255, 255});
        if (config.baus > 0 || config.boss)
        {
            Texto::desenhar_centralizado(renderizador, config.boss ? "BOSS" : "BAUS",
                                        static_cast<int>(posicao.x),
                                        static_cast<int>(caixa.y + 18), 1,
                                        {212, 194, 155, 255});
        }
    }

    std::string percurso = "PRE-ORDEM:";
    for (const auto indice : ordem)
    {
        const auto &nome = fase.get_sala(indice).get_configuracao().nome;
        percurso += " " + nome.substr(0, nome.find(" - "));
    }
    Texto::desenhar_centralizado(renderizador, percurso, 480, 420, 1,
                                {207, 220, 236, 255});
    Texto::desenhar_centralizado(renderizador, "ESQ/DIR/CIMA/BAIXO: PORTA NA SALA PAI", 480, 438,
                                1, {226, 235, 247, 255});
    Texto::desenhar_centralizado(renderizador, "AMARELO: VOCE   VERDE: CONCLUIDA", 480, 454,
                                1, {226, 235, 247, 255});
    Texto::desenhar_centralizado(renderizador, "CINZA: NAO VISITADA   VERMELHO: EM COMBATE",
                                480, 470, 1, {183, 199, 218, 255});
    Texto::desenhar_centralizado(renderizador,
                                 toque ? "TOQUE PARA FECHAR   -   JOGO PAUSADO"
                                       : "M / ESC: FECHAR   -   JOGO PAUSADO",
                                 480, 486, 1, {255, 222, 110, 255});
    SDL_SetRenderDrawBlendMode(renderizador, anterior);
}
