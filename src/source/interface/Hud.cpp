#include "interface/Hud.h"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <string>

#include "config/Config.h"
#include "graficos/IconeArma.h"
#include "graficos/Sprite.h"
#include "interface/Texto.h"

namespace
{
enum class Icone
{
    VIDA,
    ESCUDO
};

void desenhar_icone(SDL_Renderer *renderizador, int x, int y, Icone icone, SDL_Color cor)
{
    SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, 220);
    if (icone == Icone::VIDA)
    {
        // Coracao de 24x25, grande o bastante para acompanhar a hotbar.
        const SDL_Rect partes[] = {{x + 2, y, 8, 6},       {x + 14, y, 8, 6},
                                   {x, y + 5, 24, 8},      {x + 3, y + 13, 18, 5},
                                   {x + 6, y + 18, 12, 4}, {x + 9, y + 22, 6, 3}};
        for (const auto &parte : partes)
        {
            SDL_RenderFillRect(renderizador, &parte);
        }
    }
    else
    {
        // Escudo de 24x25, com cruz igualmente ampliada.
        const SDL_Point pontos[] = {{x, y},           {x + 24, y}, {x + 24, y + 13},
                                    {x + 12, y + 25}, {x, y + 13}, {x, y}};
        SDL_RenderDrawLines(renderizador, pontos, 6);
        SDL_RenderDrawLine(renderizador, x + 12, y + 4, x + 12, y + 19);
        SDL_RenderDrawLine(renderizador, x + 5, y + 10, x + 19, y + 10);
    }
}

void desenhar_cicloFila(SDL_Renderer *renderizador)
{
    // Uma unica seta circular anti-horaria, sem fundo, ao lado da hotbar.
    // O arco espesso, a sombra e a ponta preenchida preservam a leitura em pixel art.
    constexpr int CENTRO_X = 728;
    constexpr int CENTRO_Y = 49;
    constexpr float PI = 3.14159265358979323846f;

    auto desenhar_arco =
        [&](int deslocamentoX, int deslocamentoY, int raioInicial, int raioFinal, SDL_Color cor)
    {
        SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
        const int minimoQuadrado = raioInicial * raioInicial;
        const int maximoQuadrado = raioFinal * raioFinal;
        for (int y = -raioFinal; y <= raioFinal; ++y)
        {
            for (int x = -raioFinal; x <= raioFinal; ++x)
            {
                const int distanciaQuadrada = x * x + y * y;
                if (distanciaQuadrada < minimoQuadrado || distanciaQuadrada > maximoQuadrado)
                {
                    continue;
                }
                const float angulo =
                    std::atan2(static_cast<float>(-y), static_cast<float>(x)) * 180.0f / PI;
                if (angulo >= -125.0f && angulo <= 140.0f)
                {
                    SDL_RenderDrawPoint(renderizador, CENTRO_X + deslocamentoX + x,
                                        CENTRO_Y + deslocamentoY + y);
                }
            }
        }
    };

    auto desenhar_ponta = [&](float deslocamentoX, float deslocamentoY, SDL_Color cor)
    {
        const float angulo = 140.0f * PI / 180.0f;
        const float fimX = CENTRO_X + 25.0f * std::cos(angulo);
        const float fimY = CENTRO_Y - 25.0f * std::sin(angulo);
        // Tangente do arco no sentido anti-horario.
        const SDL_FPoint direcao = {-std::sin(angulo), -std::cos(angulo)};
        const SDL_FPoint perpendicular = {-direcao.y, direcao.x};
        const SDL_FPoint ponta = {fimX + direcao.x * 7.0f + deslocamentoX,
                                  fimY + direcao.y * 7.0f + deslocamentoY};
        // A base volta sobre o arco: isso une as duas formas sem uma quina solta.
        const SDL_FPoint base = {ponta.x - direcao.x * 14.0f, ponta.y - direcao.y * 14.0f};
        const SDL_Vertex vertices[] = {
            {ponta, cor, {0, 0}},
            {{base.x + perpendicular.x * 7.0f, base.y + perpendicular.y * 7.0f}, cor, {0, 0}},
            {{base.x - perpendicular.x * 7.0f, base.y - perpendicular.y * 7.0f}, cor, {0, 0}},
        };
        SDL_RenderGeometry(renderizador, nullptr, vertices, 3, nullptr, 0);
    };

    desenhar_arco(2, 3, 21, 29, {5, 10, 18, 150});
    desenhar_ponta(2, 3, {5, 10, 18, 150});
    desenhar_arco(0, 0, 22, 28, {224, 236, 247, 255});
    desenhar_arco(0, 0, 22, 23, {255, 255, 255, 255});
    desenhar_ponta(0, 0, {235, 245, 253, 255});

    Texto::desenhar_centralizado(renderizador, "Q", CENTRO_X + 1,
                                 CENTRO_Y - Texto::altura(2) / 2 + 2, 2, {5, 10, 18, 190});
    Texto::desenhar_centralizado(renderizador, "Q", CENTRO_X, CENTRO_Y - Texto::altura(2) / 2, 2,
                                 {255, 255, 255, 255});
}

void desenhar_inventario(SDL_Renderer *renderizador, const FilaArmas &armas)
{
    constexpr int LADO_ATUAL = 64;
    constexpr int LADO_RESERVA = 48;
    constexpr int ESPACO = 8;
    constexpr int MARGEM_HORIZONTAL = 16;
    constexpr int MARGEM_VERTICAL = 17;
    constexpr int LARGURA = LADO_ATUAL + (MAX_ARMAS - 1) * (LADO_RESERVA + ESPACO);
    int x = LARGURA_TELA - MARGEM_HORIZONTAL - LARGURA;
    for (int i = 0; i < MAX_ARMAS; ++i)
    {
        const bool equipada = i == 0;
        const int lado = equipada ? LADO_ATUAL : LADO_RESERVA;
        const int y = MARGEM_VERTICAL + (LADO_ATUAL - lado) / 2;
        const SDL_Rect caixa = {x, y, lado, lado};
        SDL_SetRenderDrawColor(renderizador, 12, 18, 26, equipada ? 210 : 165);
        SDL_RenderFillRect(renderizador, &caixa);
        const SDL_Color borda =
            equipada ? SDL_Color{245, 202, 99, 255} : SDL_Color{151, 172, 194, 150};
        SDL_SetRenderDrawColor(renderizador, borda.r, borda.g, borda.b, borda.a);
        SDL_RenderDrawRect(renderizador, &caixa);
        if (equipada)
        {
            const SDL_Rect destaque = {x + 1, y + 1, lado - 2, lado - 2};
            SDL_RenderDrawRect(renderizador, &destaque);
        }
        const auto *arma = armas.consultar(i);
        if (arma != nullptr)
        {
            const SDL_FRect area = {static_cast<float>(x + 8), static_cast<float>(y + 8),
                                    static_cast<float>(lado - 16), static_cast<float>(lado - 16)};
            IconeArma::desenhar(renderizador, arma->get_tipo(), area);
        }
        x += lado + ESPACO;
    }
    desenhar_cicloFila(renderizador);
}

void desenhar_barra(SDL_Renderer *renderizador, int y, int valor, int maximo, SDL_Color cor,
                    Icone icone)
{
    desenhar_icone(renderizador, 26, y, icone, cor);
    const std::string valorTexto = std::to_string(valor) + "/" + std::to_string(maximo);
    Texto::desenhar(renderizador, valorTexto, 272 - Texto::largura(valorTexto, 1), y, 1,
                    {230, 237, 245, 220});
    const SDL_Rect trilha = {62, y + 12, 210, 14};
    SDL_SetRenderDrawColor(renderizador, 8, 12, 20, 160);
    SDL_RenderFillRect(renderizador, &trilha);
    const float proporcao =
        maximo > 0 ? std::clamp(static_cast<float>(valor) / maximo, 0.0f, 1.0f) : 0.0f;
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

std::string formatar_relogio(float segundos)
{
    const int decimosTotais = std::max(0, static_cast<int>(segundos * 10.0f));
    const int minutos = decimosTotais / 600;
    const int segundosInteiros = decimosTotais / 10 % 60;
    const int decimos = decimosTotais % 10;
    char texto[24];
    std::snprintf(texto, sizeof(texto), "%02d:%02d.%d", minutos, segundosInteiros, decimos);
    return texto;
}
} // namespace

void Hud::desenhar(SDL_Renderer *renderizador, const Jogador &jogador, const Sprite &spriteJogador,
                   int vidas, int pontuacao, float tempoFase)
{
    if (renderizador == nullptr)
    {
        return;
    }
    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(renderizador, &anterior);
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);
    Texto::desenhar(renderizador, "VIDAS", 18, 13, 1, {230, 237, 245, 220});
    for (int i = 0; i < std::clamp(vidas, 0, 3); ++i)
    {
        const SDL_FRect quadro = {70.0f + i * 46.0f, 2.0f, 42.0f, 38.0f};
        spriteJogador.desenhar_recorte(renderizador, quadro, {0.20f, 0.0f, 0.60f, 0.46f});
    }

    // Vida e escudo ficam diretamente sobre a gameplay, sem painel de fundo.
    desenhar_barra(renderizador, 50, jogador.get_vida(), jogador.get_vidaMax(), {235, 88, 111, 255},
                   Icone::VIDA);
    desenhar_barra(renderizador, 90, jogador.get_escudo(), jogador.get_escudoMax(),
                   {73, 190, 234, 255}, Icone::ESCUDO);

    // Pontuacao e tempo tambem nao usam caixa ou borda de fundo.
    const std::string pontos = std::to_string(std::max(0, pontuacao));
    const std::string relogio = formatar_relogio(tempoFase);
    Texto::desenhar(renderizador, "PONTUACAO", 24, 136, 1, {230, 237, 245, 220});
    Texto::desenhar(renderizador, pontos, 272 - Texto::largura(pontos, 1), 136, 1,
                    {255, 222, 110, 240});
    Texto::desenhar(renderizador, "TEMPO", 24, 159, 1, {230, 237, 245, 220});
    Texto::desenhar(renderizador, relogio, 272 - Texto::largura(relogio, 1), 159, 1,
                    {230, 237, 245, 240});

    desenhar_inventario(renderizador, jogador.get_armas());

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
