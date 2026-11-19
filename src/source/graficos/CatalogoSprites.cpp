#include "graficos/CatalogoSprites.h"
#include "graficos/Sprite.h"
#include <array>
#include <cmath>
namespace
{
std::array<Sprite, 9> inimigos;
std::array<Sprite, 9> armas;
std::array<Sprite, 22> pisos;
bool carregado = false;
const AssetEmbutido *const pngInimigos[] = {&AssetsEmbutidos::INIMIGO_0, &AssetsEmbutidos::INIMIGO_1, &AssetsEmbutidos::INIMIGO_2, &AssetsEmbutidos::INIMIGO_3, &AssetsEmbutidos::INIMIGO_4, &AssetsEmbutidos::INIMIGO_5, &AssetsEmbutidos::INIMIGO_6, &AssetsEmbutidos::INIMIGO_7, &AssetsEmbutidos::INIMIGO_8};
const AssetEmbutido *const pngArmas[] = {&AssetsEmbutidos::ARMA_0, &AssetsEmbutidos::ARMA_1, &AssetsEmbutidos::ARMA_2, &AssetsEmbutidos::ARMA_3, &AssetsEmbutidos::ARMA_4, &AssetsEmbutidos::ARMA_5, &AssetsEmbutidos::ARMA_6, &AssetsEmbutidos::ARMA_7, &AssetsEmbutidos::ARMA_8};
const AssetEmbutido *const pngPisos[] = {&AssetsEmbutidos::PISO_0, &AssetsEmbutidos::PISO_1, &AssetsEmbutidos::PISO_2, &AssetsEmbutidos::PISO_3, &AssetsEmbutidos::PISO_4, &AssetsEmbutidos::PISO_5, &AssetsEmbutidos::PISO_6, &AssetsEmbutidos::PISO_7, &AssetsEmbutidos::PISO_8, &AssetsEmbutidos::PISO_9, &AssetsEmbutidos::PISO_10, &AssetsEmbutidos::PISO_11, &AssetsEmbutidos::PISO_12, &AssetsEmbutidos::PISO_13, &AssetsEmbutidos::PISO_14, &AssetsEmbutidos::PISO_15, &AssetsEmbutidos::PISO_16, &AssetsEmbutidos::PISO_17, &AssetsEmbutidos::PISO_18, &AssetsEmbutidos::PISO_19, &AssetsEmbutidos::PISO_20, &AssetsEmbutidos::PISO_21};
}
bool CatalogoSprites::carregar(SDL_Renderer *r)
{
    liberar();
    for (std::size_t i = 0; i < inimigos.size(); ++i)
        if (!inimigos[i].carregar(r, *pngInimigos[i], 3)) { liberar(); return false; }
    for (std::size_t i = 0; i < armas.size(); ++i)
        if (!armas[i].carregar(r, *pngArmas[i])) { liberar(); return false; }
    for (std::size_t i = 0; i < pisos.size(); ++i)
        if (!pisos[i].carregar(r, *pngPisos[i])) { liberar(); return false; }
    carregado = true;
    return true;
}
void CatalogoSprites::liberar()
{
    carregado = false;
    for (auto &s : inimigos) s.liberar();
    for (auto &s : armas) s.liberar();
    for (auto &s : pisos) s.liberar();
}
bool CatalogoSprites::desenhar_inimigo(SDL_Renderer *r, TipoInimigo tipo, Afinidade afinidade,
                                      const SDL_FRect &area, float tempo)
{
    const int a = static_cast<int>(afinidade), t = static_cast<int>(tipo);
    if (!carregado || a < 0 || a > 2 || t < 0 || t > 2) return false;
    const int poses[] = {0, 1, 0, 2};
    const float duracao = t == 0 ? 0.16f : (t == 1 ? 0.22f : 0.28f);
    return inimigos[a * 3 + t].desenhar_quadro(r, area, poses[static_cast<int>(tempo / duracao) % 4]);
}
bool CatalogoSprites::desenhar_arma(SDL_Renderer *r, TipoArma tipo, const SDL_FRect &area)
{
    const int i = static_cast<int>(tipo);
    return carregado && i >= 0 && i < 9 && armas[i].desenhar(r, area);
}
bool CatalogoSprites::desenhar_empunhada(SDL_Renderer *r, TipoArma tipo, SDL_FPoint mao, float angulo)
{
    const int i = static_cast<int>(tipo);
    if (!carregado || i < 0 || i >= 9) return false;
    const bool corpoACorpo = Armas(tipo).get_alcance() == AlcanceArma::CURTO;
    const float lado = corpoACorpo ? 46.0f : 40.0f;
    const float a = angulo * 3.14159265f / 180.0f;
    const SDL_FRect area{mao.x + std::cos(a) * 16 - lado / 2,
                         mao.y + std::sin(a) * 16 - lado / 2, lado, lado};
    // Laminas, fork, pena e chicote foram desenhados em pe; canos apontam a direita.
    const float ajuste = corpoACorpo || tipo == TipoArma::ARCO ? 90.0f : 0.0f;
    return armas[i].desenhar_quadro(r, area, 0, angulo + ajuste);
}
bool CatalogoSprites::desenhar_piso(SDL_Renderer *r, int indice, const SDL_FRect &area)
{
    return carregado && indice >= 0 && indice < 22 && pisos[indice].desenhar(r, area);
}
