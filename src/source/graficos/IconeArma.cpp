#include "graficos/IconeArma.h"

#include <algorithm>
#include <cmath>

#include "graficos/Desenho.h"

namespace
{
// Cada sprite ocupa 12x12 pixels: metal claro, sombra e detalhe colorido.
struct SpriteArma
{
    const char *linhas[12];
    SDL_Color detalhe;
};

const SpriteArma PISTOLA = {{
    "............", "............", "..mmmmmmmm..", "..mMMMMMMma.",
    "..mmmmmmmm..", "...mm..m....", "...mmmmm....", "...mm.......",
    "..aaa.......", "..aaa.......", "............", "............"
}, {208, 154, 94, 255}};
const SpriteArma SHOTGUN = {{
    "............", "............", ".....mmmmmmm", "....mMMMMMMM",
    "..aaammmmmmm", ".aaaamMMMMMM", ".aaammmmmmmm", "aaaa..mm....",
    "aaa.........", "............", "............", "............"
}, {218, 138, 65, 255}};
const SpriteArma LASER = {{
    "............", ".....aa.....", "...mmAAmm...", "..mMMaaMMmaa",
    "..mMMAAMMMAA", "..mmmmmmmma.", "...mm..m....", "...mmmmm....",
    "...aa.......", "...aa.......", "............", "............"
}, {70, 224, 232, 255}};
const SpriteArma DUPLA = {{
    "............", "............", "mmmm....mmmm", "MMMMmmmmMMMM",
    "mmmmMAAMmmmm", "....maam....", "....mmmm....", ".....mm.....",
    ".....aa.....", ".....aa.....", "............", "............"
}, {192, 139, 246, 255}};
const SpriteArma RICOCHETE = {{
    ".........a..", "........aAa.", ".........a..", "..mmmmmm....",
    "..mMMMMMmm..", "..mMaaMMMma.", "..mmmmmmmm..", "...mm..m....",
    "...mmmmm....", "...aa.......", "...aa.......", "............"
}, {111, 227, 126, 255}};
const SpriteArma ESPADA = {{
    ".........MM.", "........MMm.", ".......MMm..", "......MMm...",
    ".....MMm....", "..a.MMm.....", "..aaMm......", "...aa.......",
    "..m.aa......", ".m...a......", "a...........", "............"
}, {244, 196, 91, 255}};
const SpriteArma ARCO = {{
    "....a.......", "....maa.....", "....m..a....", "....m...a...",
    "....m....aM.", ".MMMMMMMMMMM", "....m....aM.", "....m...a...",
    "....m..a....", "....maa.....", "....a.......", "............"
}, {224, 164, 86, 255}};
const SpriteArma BOOMERANG = {{
    "............", ".....aa.....", "....aAAa....", "...aAAAAa...",
    "..aAAaaAAa..", ".aAAa..aAAa.", ".aAa....aAa.", ".aa......aa.",
    ".a........a.", "............", "............", "............"
}, {245, 162, 73, 255}};

const SpriteArma &obter_sprite(TipoArma tipo)
{
    switch (tipo)
    {
    case TipoArma::SHOTGUN:
        return SHOTGUN;
    case TipoArma::LASER:
        return LASER;
    case TipoArma::DUPLA:
        return DUPLA;
    case TipoArma::RICOCHETE:
        return RICOCHETE;
    case TipoArma::ESPADA:
        return ESPADA;
    case TipoArma::ARCO:
        return ARCO;
    case TipoArma::BOOMERANG:
        return BOOMERANG;
    default:
        return PISTOLA;
    }
}

SDL_Color cor_do_pixel(const SpriteArma &sprite, char pixel)
{
    switch (pixel)
    {
    case 'M':
        return {229, 239, 249, 255};
    case 'm':
        return {106, 130, 154, 255};
    case 'A':
        return {255, 243, 202, 255};
    default:
        return sprite.detalhe;
    }
}
}

void IconeArma::desenhar(SDL_Renderer *renderizador, TipoArma tipo, const SDL_FRect &area)
{
    if (renderizador == nullptr || area.w <= 0 || area.h <= 0)
    {
        return;
    }
    const auto &sprite = obter_sprite(tipo);
    const float escala = std::min(area.w, area.h) / 12.0f;
    const float inicioX = area.x + (area.w - escala * 12) / 2;
    const float inicioY = area.y + (area.h - escala * 12) / 2;
    for (int y = 0; y < 12; ++y)
    {
        for (int x = 0; x < 12; ++x)
        {
            const char pixel = sprite.linhas[y][x];
            if (pixel == '.')
            {
                continue;
            }
            const SDL_Color cor = cor_do_pixel(sprite, pixel);
            SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
            const SDL_FRect ponto = {inicioX + x * escala, inicioY + y * escala, escala, escala};
            SDL_RenderFillRectF(renderizador, &ponto);
        }
    }
}

void IconeArma::desenhar_empunhada(SDL_Renderer *renderizador, TipoArma tipo, SDL_FPoint mao,
                                float anguloGraus)
{
    const auto &sprite = obter_sprite(tipo);
    constexpr float ESCALA = 3.0f;
    SDL_FPoint apoio = {3.5f, 6.5f};
    float ajuste = 0;
    if (tipo == TipoArma::ESPADA)
    {
        apoio = {3, 8};
        ajuste = 45;
    }
    else if (tipo == TipoArma::BOOMERANG)
    {
        apoio = {6, 6};
        ajuste = 90;
    }
    else if (tipo == TipoArma::ARCO || tipo == TipoArma::DUPLA)
    {
        apoio = {5.5f, 5.5f};
    }
    const float radianos = anguloGraus * static_cast<float>(M_PI) / 180.0f;
    // Espelha na vertical local ao mirar a esquerda, mantendo o cabo para baixo.
    const bool espelhar = std::cos(radianos) < 0;
    const float angulo = anguloGraus + (espelhar ? -ajuste : ajuste);
    const float rotacao = angulo * static_cast<float>(M_PI) / 180.0f;
    const float cosseno = std::cos(rotacao), seno = std::sin(rotacao);
    for (int y = 0; y < 12; ++y)
    {
        for (int x = 0; x < 12; ++x)
        {
            const char pixel = sprite.linhas[y][x];
            if (pixel == '.')
            {
                continue;
            }
            const float localX = (x - apoio.x) * ESCALA;
            const float localY = (y + 0.5f - apoio.y) * ESCALA * (espelhar ? -1 : 1);
            Desenho::retangulo_rotacionado(renderizador,
                mao.x + localX * cosseno - localY * seno,
                mao.y + localX * seno + localY * cosseno,
                ESCALA, ESCALA, angulo, cor_do_pixel(sprite, pixel));
        }
    }
}
