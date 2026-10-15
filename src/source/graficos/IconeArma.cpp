#include "graficos/IconeArma.h"

#include <algorithm>

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
            SDL_Color cor = sprite.detalhe;
            if (pixel == 'M')
            {
                cor = {229, 239, 249, 255};
            }
            else if (pixel == 'm')
            {
                cor = {106, 130, 154, 255};
            }
            else if (pixel == 'A')
            {
                cor = {255, 243, 202, 255};
            }
            SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
            const SDL_FRect ponto = {inicioX + x * escala, inicioY + y * escala, escala, escala};
            SDL_RenderFillRectF(renderizador, &ponto);
        }
    }
}
