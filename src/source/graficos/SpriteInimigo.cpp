#include "graficos/SpriteInimigo.h"

#include <cstring>

namespace
{
struct CorPixel
{
    char codigo;
    SDL_Color cor;
};

// Contornos e sombras compartilhados entre os sprites.
const SDL_Color CONTORNO = {18, 14, 26, 255};

// Virus comum: chip infectado, trilhas e olhos luminosos.
const char *const VIRUS[] = {
    "..g..g..g..g", "..g..g..g..g", "ggGGGGGGGGgg", "..GggggggG..",
    "ggGgWggWgGgg", "..GgWggWgG..", "..GggggggG..", "ggGgRRRRgGgg",
    "..GggRRggG..", "ggGGGGGGGGgg", "..g..g..g..g", "..g..g..g..g",
};
const CorPixel CORES_VIRUS[] = {
    {'G', {19, 58, 53, 255}}, {'g', {61, 204, 113, 255}},
    {'W', {220, 255, 164, 255}}, {'R', {52, 28, 64, 255}},
};

// Fase 1, guardiao SEGFAULT: bloco de erro rachado com olhos em X.
const char *const GUARDIAO_SEGFAULT[] = {
    "................", "..KKKKKKKKKKKK..", ".KRRRRRRrRRRRRK.", ".KRWRWRRrRWRWRK.",
    ".KRRWRRrRRRWRRK.", ".KRWRWRrRRWRWRK.", ".KRRRRRrRRRRRRK.", ".KRYYYYYYYYYYRK.",
    ".KRYKYKYKYKYKRK.", ".KRYYYYYYYYYYRK.", ".KrRRRRRrRRRRrK.", ".KrrrrrrrrrrrrK.",
    "..KKKKKKKKKKKK..", "..KrrK....KrrK..", "..KrrK....KrrK..", ".KKKKK....KKKKK.",
};
const CorPixel CORES_SEGFAULT[] = {
    {'R', {196, 52, 64, 255}}, {'r', {122, 30, 44, 255}},
    {'W', {244, 236, 228, 255}}, {'Y', {255, 214, 90, 255}},
};

// Fase 2, guardiao SPAMBOT: envelope com antena que solta faiscas.
const char *const GUARDIAO_SPAMBOT[] = {
    "................", ".......YY.......", ".......KK.......", ".KKKKKKKKKKKKKK.",
    ".KwWWWWWWWWWWwK.", ".KWwWWWWWWWWwWK.", ".KWWwWWWWWWwWWK.", ".KWWWwBWWBwWWWK.",
    ".KWWWWwWWwWWWWK.", ".KWWWWWwwWWWRRK.", ".KWWWWWWWWWWRRK.", ".KwwwwwwwwwwwwK.",
    ".KKKKKKKKKKKKKK.", "...C...C...C....", "..C...C...C.....", "................",
};
const CorPixel CORES_SPAMBOT[] = {
    {'W', {230, 232, 240, 255}}, {'w', {150, 160, 182, 255}},
    {'B', {58, 120, 220, 255}}, {'R', {222, 62, 72, 255}},
    {'Y', {255, 210, 80, 255}}, {'C', {120, 220, 255, 255}},
};

// Fase 3, guardiao PROCESSO ZUMBI: cerebro exposto, olhos vermelhos e bracos estendidos.
const char *const GUARDIAO_ZUMBI[] = {
    "....KKKKKKKK....", "...KPPPgPPPPK...", "..KGGGGGGGGGGK..", "..KGRRGGGGRRGK..",
    "..KGRRGGGgRRGK..", "..KGGGGKKGGGGK..", "..KGKWKWKWKWGK..", "..KGGGGGGGGGGK..",
    "...KKKKKKKKKK...", ".KwwwwKDDKwwwwK.", "KGKwwwwDDwwwwKGK", "KGKwwwwwwwwwwKGK",
    ".K.KwwwKKwwwK.K.", "...KDDK..KDDK...", "...KDDK..KDDK...", "..KKKK....KKKK..",
};
const CorPixel CORES_ZUMBI[] = {
    {'G', {112, 172, 92, 255}}, {'g', {62, 104, 62, 255}},
    {'P', {222, 122, 152, 255}}, {'R', {236, 52, 52, 255}},
    {'W', {236, 232, 214, 255}}, {'w', {120, 124, 140, 255}},
    {'D', {48, 46, 60, 255}},
};

// Fase 1, boss COMPILADOR FECHADO: maquina trancada com chaves "{ }" e saida laranja.
const char *const BOSS_COMPILADOR[] = {
    "........KKKKKKKK........", ".......KYYYYYYYYK.......",
    "......KYYKKKKKKYYK......", "......KYK......KYK......",
    "......KYK......KYK......", "..KKKKKYKKKKKKKKYKKKKK..",
    ".KbbbbbbbbbbbbbbbbbbbbK.", ".KbBBBBBBBBBBBBBBBBBBbK.",
    ".KbBKKKKBBBBBBBBKKKKBbK.", ".KbBKOOKBBBBBBBBKOOKBbK.",
    ".KbBKOoKBBBBBBBBKoOKBbK.", ".KbBKKKKBBBBBBBBKKKKBbK.",
    ".KbBBBBBBBBBBBBBBBBBBbK.", ".KbBBBYYBBBBBBBBYYBBBbK.",
    ".KbBBYBBBKKKKKKBBBYBBbK.", ".KbBYBBBBKOOOOKBBBBYBbK.",
    ".KbBBYBBBKKKKKKBBBYBBbK.", ".KbBBBYYBBBBBBBBYYBBBbK.",
    ".KbBBBBBBBBBBBBBBBBBBbK.", ".KbbbbbbbbbbbbbbbbbbbbK.",
    ".KKKKKKKKKKKKKKKKKKKKKK.", "..KyKyKyKK....KKyKyKyK..",
    "..KyKyKyKK....KKyKyKyK..", "..KKKKKKKK....KKKKKKKK..",
};
const CorPixel CORES_COMPILADOR[] = {
    {'B', {52, 56, 72, 255}}, {'b', {92, 98, 122, 255}},
    {'O', {255, 140, 40, 255}}, {'o', {160, 72, 24, 255}},
    {'Y', {240, 196, 70, 255}}, {'y', {150, 110, 40, 255}},
};

// Fase 2, boss REDE ANTISSOCIAL: icone de app com olho vigilante, notificacao e rede.
const char *const BOSS_REDE[] = {
    "..................KKKK..", ".................KRWWRK.",
    "...KKKKKKKKKKKKKKKRWWRK.", "..KKBBBBBBBBBBBBBKRRRRK.",
    "..KBbbbKbbbbbbbbKKRWWRK.", "..KBbBBBKKKKKKKKBKKKKKK.",
    "..KBbBBKKKKKKKKKKBBBBK..", "..KBbBKKWWWWWWWWKKBBBK..",
    "..KBbBKWWIIIIIIWWKBBBK..", "..KBbBKWWIWPPPIWWKBBBK..",
    "..KBbBKWWIPPPPIWWKBBBK..", "..KBbBKWWIPPPPIWWKBBBK..",
    "..KBbBKWWIIIIIIWWKBBBK..", "..KBbBKKWWWWWWWWKKBBBK..",
    "..KBbBBKKKKKKKKKKBBBBK..", "..KBbBBBBBBBBBBBBBBBBK..",
    "..KBBBBBBBBBBBBBBBBBBK..", "..KKBBBBBBBBBBBBBBBBKK..",
    "...KKKKKKKKKKKKKKKKKK...", ".....K...K....K...K.....",
    "....K....K....K....K....", "....K....K....K....K....",
    "..CKC...CKC..CKC...CKC..", "..CCC...CCC..CCC...CCC..",
};
const CorPixel CORES_REDE[] = {
    {'B', {59, 89, 182, 255}}, {'b', {112, 152, 232, 255}},
    {'W', {245, 245, 250, 255}}, {'P', {20, 20, 40, 255}},
    {'I', {90, 200, 255, 255}}, {'R', {230, 50, 60, 255}},
    {'C', {120, 220, 255, 255}},
};

// Fase 3, boss TELA AZUL DA MORTE: monitor CRT com rosto ":(" e dentes na moldura.
const char *const BOSS_TELA_AZUL[] = {
    ".KKKKKKKKKKKKKKKKKKKKKK.", ".KwwwwwwwwwwwwwwwwwwwwK.",
    ".KwddddddddddddddddddwK.", "KKwdSSSSSSSSSSSSSSSSdwKK",
    "KKwdSSSSSSSSSSSSSbbSdwKK", ".KwdSSSWWSSSSSSWWSSSdwK.",
    ".KwdSSSWWSSSSSSWWSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.",
    ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.",
    ".KwdSSSSWWWWWWWWSSSSdwK.", ".KwdSSSWSSSSSSSSWSSSdwK.",
    ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.",
    ".KwdWdWdWdWdWdWdWdWddwK.", ".KdRdRdRddddddddddddddK.",
    ".KddddddddddddddddddddK.", ".KKKKKKKKKKKKKKKKKKKKKK.",
    "........KKKKKKKK........", "........KKKKKKKK....KY..",
    "....KKKKKKKKKKKKKKKK.KY.", "....KddddddddddddddK..KY",
    "....KKKKKKKKKKKKKKKK...K", "........................",
};
const CorPixel CORES_TELA_AZUL[] = {
    {'w', {176, 176, 188, 255}}, {'d', {100, 100, 114, 255}},
    {'S', {24, 92, 206, 255}}, {'b', {120, 170, 255, 255}},
    {'W', {244, 246, 255, 255}}, {'R', {230, 60, 60, 255}},
    {'Y', {240, 196, 70, 255}},
};

template <std::size_t N, std::size_t C>
void desenhar_bitmap(SDL_Renderer *renderizador, const char *const (&linhas)[N],
                     const CorPixel (&cores)[C], const SDL_FRect &area)
{
    const int largura = static_cast<int>(std::strlen(linhas[0]));
    const float passoX = area.w / largura, passoY = area.h / static_cast<float>(N);
    for (std::size_t y = 0; y < N; ++y)
    {
        for (int x = 0; x < largura; ++x)
        {
            const char codigo = linhas[y][x];
            if (codigo == '.')
            {
                continue;
            }
            SDL_Color cor = CONTORNO;
            for (const auto &entrada : cores)
            {
                if (entrada.codigo == codigo)
                {
                    cor = entrada.cor;
                }
            }
            SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
            const SDL_FRect pixel = {area.x + x * passoX, area.y + y * passoY, passoX, passoY};
            SDL_RenderFillRectF(renderizador, &pixel);
        }
    }
}
}

void SpriteInimigo::desenhar(SDL_Renderer *renderizador, TipoInimigo tipo, int tema,
                             const SDL_FRect &area)
{
    if (tipo == TipoInimigo::VIRUS)
    {
        desenhar_bitmap(renderizador, VIRUS, CORES_VIRUS, area);
    }
    else if (tipo == TipoInimigo::BRUTO)
    {
        if (tema == 2) desenhar_bitmap(renderizador, GUARDIAO_SPAMBOT, CORES_SPAMBOT, area);
        else if (tema == 3) desenhar_bitmap(renderizador, GUARDIAO_ZUMBI, CORES_ZUMBI, area);
        else desenhar_bitmap(renderizador, GUARDIAO_SEGFAULT, CORES_SEGFAULT, area);
    }
    else
    {
        if (tema == 2) desenhar_bitmap(renderizador, BOSS_REDE, CORES_REDE, area);
        else if (tema == 3) desenhar_bitmap(renderizador, BOSS_TELA_AZUL, CORES_TELA_AZUL, area);
        else desenhar_bitmap(renderizador, BOSS_COMPILADOR, CORES_COMPILADOR, area);
    }
}
