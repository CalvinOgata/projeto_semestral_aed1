#include "interface/FundoMenu.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
using FundoMenu::sorteio;

Uint8 canal(float valor)
{
    return static_cast<Uint8>(std::clamp(valor, 0.0f, 255.0f));
}

SDL_Color misturar(SDL_Color a, SDL_Color b, float t, float alfa = 255)
{
    return {canal(a.r + (b.r - a.r) * t), canal(a.g + (b.g - a.g) * t),
            canal(a.b + (b.b - a.b) * t), canal(alfa)};
}

void desenhar_ceu(SDL_Renderer *r)
{
    const SDL_Color topo = {6, 8, 24, 255};
    const SDL_Color baixo = {52, 16, 70, 255};
    for (int y = 0; y < FundoMenu::HORIZONTE; y += 6)
    {
        const float t = static_cast<float>(y) / FundoMenu::HORIZONTE;
        FundoMenu::preencher(r, {0, y, LARGURA_TELA, 6}, misturar(topo, baixo, t * t));
    }
}

void desenhar_estrelas(SDL_Renderer *r, float tempo, float intensidade)
{
    for (int i = 0; i < 80; ++i)
    {
        const int x = static_cast<int>(sorteio(i * 3 + 1) * LARGURA_TELA);
        const int y = static_cast<int>(sorteio(i * 3 + 2) * (FundoMenu::HORIZONTE - 90));
        const float brilho = 0.5f + 0.5f * std::sin(tempo * (1.0f + sorteio(i) * 3.0f) + i);
        const int lado = sorteio(i * 7) > 0.85f ? 2 : 1;
        FundoMenu::preencher(r, {x, y, lado, lado},
                             {200, 210, 255, canal((60 + 160 * brilho) * intensidade)});
    }
}

// Colunas de 0 e 1 caindo, como um terminal antigo ao fundo.
void desenhar_chuvaBinaria(SDL_Renderer *r, float tempo, float intensidade)
{
    constexpr int COLUNAS = 26;
    constexpr int TRILHA = 9;
    const int alturaChar = Texto::altura(2) + 4;
    for (int c = 0; c < COLUNAS; ++c)
    {
        const int x = 14 + c * 37 + static_cast<int>(sorteio(c + 100) * 12);
        const float velocidade = 26.0f + sorteio(c + 200) * 50.0f;
        const float ciclo = FundoMenu::HORIZONTE + TRILHA * alturaChar;
        const float cabeca = std::fmod(tempo * velocidade + sorteio(c + 300) * ciclo, ciclo);
        for (int k = 0; k < TRILHA; ++k)
        {
            const int y = static_cast<int>(cabeca) - k * alturaChar;
            if (y < -alturaChar || y > FundoMenu::HORIZONTE - alturaChar)
            {
                continue;
            }
            const int linha = (static_cast<int>(cabeca) / alturaChar) - k;
            const int troca = static_cast<int>(tempo * 6.0f) + c * 31 + linha * 17;
            const std::string bit = sorteio(troca) > 0.5f ? "1" : "0";
            const float desbota = 1.0f - static_cast<float>(k) / TRILHA;
            const SDL_Color cor = k == 0
                                      ? SDL_Color{170, 255, 200, canal(150 * intensidade)}
                                      : SDL_Color{40, 200, 120, canal(70 * desbota * intensidade)};
            Texto::desenhar(r, bit, x, y, 2, cor);
        }
    }
}

// Sol listrado ao estilo dos anos 80, com faixas que descem.
void desenhar_sol(SDL_Renderer *r, float tempo, float intensidade)
{
    constexpr int RAIO = 118;
    const int centroX = LARGURA_TELA / 2;
    const int centroY = FundoMenu::HORIZONTE - 6;
    const SDL_Color topo = {255, 214, 96, 255};
    const SDL_Color base = {255, 52, 150, 255};
    for (int dy = -RAIO; dy <= 0; dy += 2)
    {
        const int y = centroY + dy;
        const float t = static_cast<float>(dy + RAIO) / RAIO;
        // Faixas vazadas que ficam mais grossas perto do horizonte.
        if (t > 0.45f)
        {
            const float faixa = std::fmod(static_cast<float>(y) + tempo * 14.0f, 14.0f);
            if (faixa < (t - 0.45f) * 18.0f)
            {
                continue;
            }
        }
        const int meia = static_cast<int>(std::sqrt(static_cast<float>(RAIO * RAIO - dy * dy)));
        FundoMenu::preencher(r, {centroX - meia, y, meia * 2, 2},
                             misturar(topo, base, t, 230 * intensidade));
    }
    // Halo suave em volta do sol.
    for (int i = 1; i <= 4; ++i)
    {
        const int raio = RAIO + i * 14;
        for (int dy = -raio; dy <= 0; dy += 4)
        {
            const int meia = static_cast<int>(std::sqrt(static_cast<float>(raio * raio - dy * dy)));
            FundoMenu::preencher(r, {centroX - meia, centroY + dy, meia * 2, 4},
                                 {255, 70, 160, canal(7 * intensidade)});
        }
    }
}

// Silhueta de racks de servidores no horizonte, com LEDs piscando.
void desenhar_servidores(SDL_Renderer *r, float tempo, float intensidade)
{
    int x = 0;
    int indice = 0;
    while (x < LARGURA_TELA)
    {
        const int largura = 34 + static_cast<int>(sorteio(indice + 500) * 40);
        const int altura = 26 + static_cast<int>(sorteio(indice + 600) * 70);
        // Deixa o meio mais baixo para o sol aparecer.
        const float meio =
            std::abs(x + largura / 2.0f - LARGURA_TELA / 2.0f) / (LARGURA_TELA / 2.0f);
        const int alturaFinal = static_cast<int>(altura * std::clamp(meio * 1.6f, 0.25f, 1.0f));
        const SDL_Rect torre = {x, FundoMenu::HORIZONTE - alturaFinal, largura - 3, alturaFinal};
        FundoMenu::preencher(r, torre, {14, 8, 30, canal(255 * intensidade)});
        FundoMenu::preencher(r, {torre.x, torre.y, torre.w, 1},
                             {120, 60, 200, canal(90 * intensidade)});
        for (int led = 0; led * 9 + 8 < alturaFinal; ++led)
        {
            const float fase = sorteio(indice * 13 + led);
            const bool aceso = std::fmod(tempo * (0.6f + fase * 2.0f) + fase * 7.0f, 1.0f) < 0.55f;
            if (!aceso)
            {
                continue;
            }
            const SDL_Color cor = fase < 0.33f ? SDL_Color{80, 255, 150, 255}
                                               : (fase < 0.66f ? SDL_Color{80, 200, 255, 255}
                                                               : SDL_Color{255, 90, 120, 255});
            FundoMenu::preencher(r, {torre.x + 5, torre.y + 6 + led * 9, 3, 2},
                                 {cor.r, cor.g, cor.b, canal(200 * intensidade)});
            FundoMenu::preencher(r, {torre.x + 11, torre.y + 6 + led * 9, torre.w - 18, 1},
                                 {60, 40, 110, canal(120 * intensidade)});
        }
        x += largura;
        ++indice;
    }
}

// Piso em perspectiva que corre na direcao do jogador.
void desenhar_grade(SDL_Renderer *r, float tempo, float intensidade)
{
    const int topo = FundoMenu::HORIZONTE;
    const int altura = ALTURA_TELA - topo;
    for (int y = topo; y < ALTURA_TELA; y += 6)
    {
        const float t = static_cast<float>(y - topo) / altura;
        FundoMenu::preencher(r, {0, y, LARGURA_TELA, 6},
                             misturar({40, 8, 54, 255}, {8, 4, 18, 255}, t));
    }
    constexpr int LINHAS = 14;
    const float deslocamento = std::fmod(tempo * 0.55f, 1.0f);
    for (int k = 0; k < LINHAS; ++k)
    {
        const float pos = (k + deslocamento) / LINHAS;
        const int y = topo + static_cast<int>(altura * std::pow(pos, 2.3f));
        FundoMenu::preencher(r, {0, y, LARGURA_TELA, 1 + static_cast<int>(pos * 2)},
                             {255, 60, 210, canal((40 + 170 * pos) * intensidade)});
    }
    const int pontoFuga = LARGURA_TELA / 2;
    for (int i = -14; i <= 14; ++i)
    {
        const int baseX = pontoFuga + i * 86;
        const int topoX = pontoFuga + static_cast<int>(i * 86 * 0.12f);
        SDL_SetRenderDrawColor(r, 255, 60, 210, canal(110 * intensidade));
        SDL_RenderDrawLine(r, topoX, topo, baseX, ALTURA_TELA);
    }
    // Brilho do horizonte.
    for (int i = 0; i < 5; ++i)
    {
        FundoMenu::preencher(r, {0, topo - i * 2, LARGURA_TELA, 2},
                             {255, 120, 230, canal((140 - i * 28) * intensidade)});
    }
}

void desenhar_particulas(SDL_Renderer *r, float tempo, float intensidade)
{
    for (int i = 0; i < 34; ++i)
    {
        const float velocidade = 14.0f + sorteio(i + 900) * 30.0f;
        const float y = ALTURA_TELA - std::fmod(tempo * velocidade + sorteio(i + 950) * 620, 620);
        const float x = sorteio(i + 980) * LARGURA_TELA + std::sin(tempo + i) * 10.0f;
        const float brilho = 0.5f + 0.5f * std::sin(tempo * 3 + i);
        FundoMenu::preencher(r, {static_cast<int>(x), static_cast<int>(y), 2, 2},
                             {120, 230, 255, canal((50 + 120 * brilho) * intensidade)});
    }
}
} // namespace

// O cenario e sempre igual porque o sorteio depende so da semente.
float FundoMenu::sorteio(int semente)
{
    unsigned int x = static_cast<unsigned int>(semente) * 2654435761u;
    x ^= x >> 13;
    x *= 0x5bd1e995u;
    x ^= x >> 15;
    return static_cast<float>(x % 10000u) / 10000.0f;
}

void FundoMenu::preencher(SDL_Renderer *renderizador, const SDL_Rect &area, SDL_Color cor)
{
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
    SDL_RenderFillRect(renderizador, &area);
}

void FundoMenu::desenhar(SDL_Renderer *renderizador, float tempo, float intensidade)
{
    if (renderizador == nullptr)
    {
        return;
    }
    desenhar_ceu(renderizador);
    desenhar_estrelas(renderizador, tempo, intensidade);
    desenhar_chuvaBinaria(renderizador, tempo, intensidade);
    desenhar_sol(renderizador, tempo, intensidade);
    desenhar_servidores(renderizador, tempo, intensidade);
    desenhar_grade(renderizador, tempo, intensidade);
    desenhar_particulas(renderizador, tempo, intensidade);
    // Linhas de varredura discretas de monitor CRT.
    for (int y = 0; y < ALTURA_TELA; y += 3)
    {
        preencher(renderizador, {0, y, LARGURA_TELA, 1}, {0, 0, 0, 34});
    }
}

void FundoMenu::desenhar_logo(SDL_Renderer *renderizador, int centroX, int y, int escala,
                              float tempo, Uint8 alfa)
{
    const std::string logo = "M.C.G.A";
    const int largura = Texto::largura(logo, escala);
    const int x = centroX - largura / 2;
    const float flutuacao = std::sin(tempo * 1.7f) * 3.0f;
    const int yFinal = y + static_cast<int>(flutuacao);
    // A cada ~3,5 s uma falha rapida separa as cores e desloca uma fatia.
    const float ciclo = std::fmod(tempo, 3.5f);
    const bool falha = ciclo > 3.3f;
    const int separacao = falha ? escala : std::max(1, escala / 4);

    Texto::desenhar(renderizador, logo, x + escala, yFinal + escala, escala,
                    {40, 6, 60, static_cast<Uint8>(alfa * 0.8f)});
    Texto::desenhar(renderizador, logo, x - separacao, yFinal, escala,
                    {40, 220, 255, static_cast<Uint8>(alfa * 0.7f)});
    Texto::desenhar(renderizador, logo, x + separacao, yFinal, escala,
                    {255, 40, 170, static_cast<Uint8>(alfa * 0.7f)});
    Texto::desenhar(renderizador, logo, x, yFinal, escala, {255, 246, 230, alfa});
    // Metade de baixo dourada, como letreiro de fliperama.
    const SDL_Rect metade = {x - escala, yFinal + Texto::altura(escala) / 2 + escala / 2,
                             largura + escala * 2, Texto::altura(escala)};
    SDL_RenderSetClipRect(renderizador, &metade);
    Texto::desenhar(renderizador, logo, x, yFinal, escala, {255, 196, 80, alfa});
    SDL_RenderSetClipRect(renderizador, nullptr);

    if (falha)
    {
        const int fatiaY = yFinal + static_cast<int>(sorteio(static_cast<int>(tempo * 30)) *
                                                     Texto::altura(escala));
        const SDL_Rect fatia = {x - 40, fatiaY, largura + 80, escala * 2};
        SDL_RenderSetClipRect(renderizador, &fatia);
        preencher(renderizador, fatia, {8, 6, 20, alfa});
        Texto::desenhar(renderizador, logo, x + escala * 3, yFinal, escala, {40, 220, 255, alfa});
        SDL_RenderSetClipRect(renderizador, nullptr);
    }
}

void FundoMenu::desenhar_painel(SDL_Renderer *renderizador, const SDL_Rect &area, float tempo)
{
    preencher(renderizador, area, {8, 10, 28, 222});
    SDL_SetRenderDrawColor(renderizador, 80, 200, 255, 70);
    const SDL_Rect interna = {area.x + 3, area.y + 3, area.w - 6, area.h - 6};
    SDL_RenderDrawRect(renderizador, &interna);
    const float pulso = 0.75f + 0.25f * std::sin(tempo * 2.5f);
    SDL_SetRenderDrawColor(renderizador, 80, 200, 255, canal(200 * pulso));
    SDL_RenderDrawRect(renderizador, &area);
    // Cantos em L mais brilhantes.
    constexpr int CANTO = 16;
    const SDL_Color brilho = {150, 240, 255, 255};
    const int dir = area.x + area.w;
    const int base = area.y + area.h;
    preencher(renderizador, {area.x - 1, area.y - 1, CANTO, 3}, brilho);
    preencher(renderizador, {area.x - 1, area.y - 1, 3, CANTO}, brilho);
    preencher(renderizador, {dir - CANTO + 1, area.y - 1, CANTO, 3}, brilho);
    preencher(renderizador, {dir - 2, area.y - 1, 3, CANTO}, brilho);
    preencher(renderizador, {area.x - 1, base - 2, CANTO, 3}, brilho);
    preencher(renderizador, {area.x - 1, base - CANTO + 1, 3, CANTO}, brilho);
    preencher(renderizador, {dir - CANTO + 1, base - 2, CANTO, 3}, brilho);
    preencher(renderizador, {dir - 2, base - CANTO + 1, 3, CANTO}, brilho);
    // Faixa de varredura descendo devagar pelo painel.
    const int varredura =
        area.y + static_cast<int>(std::fmod(tempo * 60.0f, static_cast<float>(area.h)));
    preencher(renderizador, {area.x + 1, varredura, area.w - 2, 2}, {80, 200, 255, 22});
}
