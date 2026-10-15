#include "interface/Hud.h"

#include <algorithm>
#include <string>

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
enum class Icone
{
    VIDA,
    ESCUDO,
    ENERGIA
};

void desenhar_icone(SDL_Renderer *renderizador, int x, int y, Icone icone, SDL_Color cor)
{
    SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, 220);
    if (icone == Icone::VIDA)
    {
        // Coracao pixelado, preenchido em linhas.
        const SDL_Rect partes[] = {{x + 1, y, 5, 4}, {x + 9, y, 5, 4},
                                   {x, y + 3, 15, 5}, {x + 2, y + 8, 11, 3},
                                   {x + 4, y + 11, 7, 2}, {x + 6, y + 13, 3, 2}};
        for (const auto &parte : partes)
        {
            SDL_RenderFillRect(renderizador, &parte);
        }
    }
    else if (icone == Icone::ESCUDO)
    {
        const SDL_Point pontos[] = {{x, y}, {x + 14, y}, {x + 14, y + 8},
                                    {x + 7, y + 15}, {x, y + 8}, {x, y}};
        SDL_RenderDrawLines(renderizador, pontos, 6);
        SDL_RenderDrawLine(renderizador, x + 7, y + 3, x + 7, y + 11);
        SDL_RenderDrawLine(renderizador, x + 3, y + 6, x + 11, y + 6);
    }
    else
    {
        const SDL_Point pontos[] = {{x + 9, y}, {x + 2, y + 8}, {x + 7, y + 8},
                                    {x + 4, y + 15}, {x + 13, y + 5},
                                    {x + 8, y + 5}, {x + 9, y}};
        SDL_RenderDrawLines(renderizador, pontos, 7);
    }
}

void desenhar_barra(SDL_Renderer *renderizador, int y, const char *nome, int valor,
                    int maximo, SDL_Color cor, Icone icone)
{
    desenhar_icone(renderizador, 26, y + 3, icone, cor);
    Texto::desenhar(renderizador, nome, 50, y, 1, {230, 237, 245, 220});
    const std::string valorTexto = std::to_string(valor) + "/" + std::to_string(maximo);
    Texto::desenhar(renderizador, valorTexto, 204 - Texto::largura(valorTexto, 1),
                    y, 1, {230, 237, 245, 220});
    const SDL_Rect trilha = {50, y + 12, 154, 9};
    SDL_SetRenderDrawColor(renderizador, 8, 12, 20, 160);
    SDL_RenderFillRect(renderizador, &trilha);
    const float proporcao = maximo > 0 ?
        std::clamp(static_cast<float>(valor) / maximo, 0.0f, 1.0f) : 0.0f;
    const SDL_Rect carga = {trilha.x, trilha.y, static_cast<int>(trilha.w * proporcao), trilha.h};
    SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, 195);
    SDL_RenderFillRect(renderizador, &carga);
    if (carga.w > 0)
    {
        SDL_SetRenderDrawColor(renderizador, 255, 255, 255, 90);
        SDL_RenderDrawLine(renderizador, carga.x, carga.y, carga.x + carga.w - 1, carga.y);
    }
    // Marcadores discretos dividem a barra em quatro partes.
    SDL_SetRenderDrawColor(renderizador, 8, 12, 20, 115);
    for (int i = 1; i < 4; ++i)
    {
        const int divisor = trilha.x + trilha.w * i / 4;
        SDL_RenderDrawLine(renderizador, divisor, trilha.y, divisor, trilha.y + trilha.h - 1);
    }
}
}

void Hud::desenhar(SDL_Renderer *renderizador, const Jogador &jogador)
{
    if (renderizador == nullptr)
    {
        return;
    }
    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(renderizador, &anterior);
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    // Uma unica caixa para os tres atributos, sobreposta no canto superior esquerdo.
    const SDL_Rect caixa = {16, 16, 200, 104};
    SDL_SetRenderDrawColor(renderizador, 12, 18, 26, 140);
    SDL_RenderFillRect(renderizador, &caixa);
    SDL_SetRenderDrawColor(renderizador, 180, 200, 220, 70);
    SDL_RenderDrawRect(renderizador, &caixa);
    desenhar_barra(renderizador, 25, "VIDA", jogador.get_vida(), jogador.get_vidaMax(),
                    {235, 88, 111, 255}, Icone::VIDA);
    desenhar_barra(renderizador, 55, "ESCUDO", jogador.get_escudo(), jogador.get_escudoMax(),
                    {73, 190, 234, 255}, Icone::ESCUDO);
    desenhar_barra(renderizador, 85, "ENERGIA", jogador.get_estamina(), jogador.get_estaminaMax(),
                    {237, 197, 76, 255}, Icone::ENERGIA);

    const auto *arma = jogador.get_armas().frente();
    const int municao = arma ? arma->get_municao() : 0;
    const std::string numero = arma && arma->municao_infinita() ? "--" : std::to_string(municao);
    const int escala = 3;
    const int x = LARGURA_TELA - 20 - Texto::largura(numero, escala);
    const int y = ALTURA_TELA - 20 - Texto::altura(escala);
    // Somente o numero, com sombra discreta para leitura sobre o piso.
    Texto::desenhar(renderizador, numero, x + 1, y + 1, escala, {8, 12, 20, 180});
    Texto::desenhar(renderizador, numero, x, y, escala,
                    municao > 0 ? SDL_Color{230, 237, 245, 230} : SDL_Color{235, 88, 111, 230});
    SDL_SetRenderDrawBlendMode(renderizador, anterior);
}
