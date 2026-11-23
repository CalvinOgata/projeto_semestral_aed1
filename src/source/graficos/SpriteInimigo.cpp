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

// Basico vermelho, ILOVEYOU.EXE: carta infectada com olhos e asas digitais.
const char *const ILOVEYOU[] = {
    "............", "..rr....rr..", "...rr..rr...", "..rrrrrrrr..", ".rRRRRRRRRr.", ".rRWRRRRWRr.",
    ".rRRRrrRRRr.", ".rRRrRRrRRr.", ".rRrRRRRrRr.", ".rRRRRRRRRr.", "..rrrrrrrr..", "............",
};
const CorPixel CORES_ILOVEYOU[] = {
    {'R', {232, 58, 72, 255}},
    {'r', {126, 24, 42, 255}},
    {'W', {255, 225, 230, 255}},
};

// Basico azul, EULA-BOT: documento de licenca mecanico.
const char *const EULA[] = {
    "...BBBBBB...", "..BWWWWWWB..", "..BWbbbbWB..", "..BWWWWWWB..", "..BWbbbbWB..", "..BWWWWWWB..",
    "..BWbbbbWB..", "..BWWWWWWB..", ".bBBb..bBBb.", "bb..b..b..bb", ".b..b..b..b.", "............",
};
const CorPixel CORES_EULA[] = {
    {'B', {47, 111, 214, 255}},
    {'b', {83, 171, 245, 255}},
    {'W', {232, 245, 255, 255}},
};

// Basico verde, NPU DRONE: chip neural, trilhas e olho-sensor.
const char *const NPU[] = {
    "..g..g..g..g", "..g..g..g..g", "ggGGGGGGGGgg", "..GggggggG..", "ggGgWggWgGgg", "..GgWggWgG..",
    "..GggggggG..", "ggGgRRRRgGgg", "..GggRRggG..", "ggGGGGGGGGgg", "..g..g..g..g", "..g..g..g..g",
};
const CorPixel CORES_NPU[] = {
    {'G', {19, 58, 53, 255}},
    {'g', {61, 204, 113, 255}},
    {'W', {220, 255, 164, 255}},
    {'R', {52, 28, 64, 255}},
};

// Medio azul, SENTINELA DRM: bloco de licenca rachado com olhos em X.
const char *const SENTINELA_DRM[] = {
    "................", "..KKKKKKKKKKKK..", ".KRRRRRRrRRRRRK.", ".KRWRWRRrRWRWRK.",
    ".KRRWRRrRRRWRRK.", ".KRWRWRrRRWRWRK.", ".KRRRRRrRRRRRRK.", ".KRYYYYYYYYYYRK.",
    ".KRYKYKYKYKYKRK.", ".KRYYYYYYYYYYRK.", ".KrRRRRRrRRRRrK.", ".KrrrrrrrrrrrrK.",
    "..KKKKKKKKKKKK..", "..KrrK....KrrK..", "..KrrK....KrrK..", ".KKKKK....KKKKK.",
};
const CorPixel CORES_DRM[] = {
    {'R', {49, 105, 205, 255}},
    {'r', {31, 56, 126, 255}},
    {'W', {225, 242, 255, 255}},
    {'Y', {82, 196, 255, 255}},
};

// Medio vermelho, MIRAI SWARM: roteador infectado com antenas e dispositivos IoT.
const char *const MIRAI_SWARM[] = {
    "................", ".......YY.......", ".......KK.......", ".KKKKKKKKKKKKKK.",
    ".KwWWWWWWWWWWwK.", ".KWwWWWWWWWWwWK.", ".KWWwWWWWWWwWWK.", ".KWWWwBWWBwWWWK.",
    ".KWWWWwWWwWWWWK.", ".KWWWWWwwWWWRRK.", ".KWWWWWWWWWWRRK.", ".KwwwwwwwwwwwwK.",
    ".KKKKKKKKKKKKKK.", "...C...C...C....", "..C...C...C.....", "................",
};
const CorPixel CORES_MIRAI[] = {
    {'W', {238, 224, 228, 255}}, {'w', {158, 112, 124, 255}}, {'B', {126, 24, 42, 255}},
    {'R', {232, 58, 72, 255}},   {'Y', {255, 158, 92, 255}},  {'C', {255, 92, 104, 255}},
};

// Medio verde, TPU SENTINEL: acelerador neural robusto com placas laterais.
const char *const TPU_SENTINEL[] = {
    "....KKKKKKKK....", "...KPPPgPPPPK...", "..KGGGGGGGGGGK..", "..KGRRGGGGRRGK..",
    "..KGRRGGGgRRGK..", "..KGGGGKKGGGGK..", "..KGKWKWKWKWGK..", "..KGGGGGGGGGGK..",
    "...KKKKKKKKKK...", ".KwwwwKDDKwwwwK.", "KGKwwwwDDwwwwKGK", "KGKwwwwwwwwwwKGK",
    ".K.KwwwKKwwwK.K.", "...KDDK..KDDK...", "...KDDK..KDDK...", "..KKKK....KKKK..",
};
const CorPixel CORES_TPU[] = {
    {'G', {72, 204, 104, 255}},  {'g', {31, 104, 61, 255}},   {'P', {146, 255, 164, 255}},
    {'R', {196, 255, 102, 255}}, {'W', {226, 244, 230, 255}}, {'w', {92, 132, 106, 255}},
    {'D', {48, 46, 60, 255}},
};

// Boss azul, COMPILADOR FECHADO: maquina trancada com chaves "{ }".
const char *const BOSS_COMPILADOR[] = {
    "........KKKKKKKK........", ".......KYYYYYYYYK.......", "......KYYKKKKKKYYK......",
    "......KYK......KYK......", "......KYK......KYK......", "..KKKKKYKKKKKKKKYKKKKK..",
    ".KbbbbbbbbbbbbbbbbbbbbK.", ".KbBBBBBBBBBBBBBBBBBBbK.", ".KbBKKKKBBBBBBBBKKKKBbK.",
    ".KbBKOOKBBBBBBBBKOOKBbK.", ".KbBKOoKBBBBBBBBKoOKBbK.", ".KbBKKKKBBBBBBBBKKKKBbK.",
    ".KbBBBBBBBBBBBBBBBBBBbK.", ".KbBBBYYBBBBBBBBYYBBBbK.", ".KbBBYBBBKKKKKKBBBYBBbK.",
    ".KbBYBBBBKOOOOKBBBBYBbK.", ".KbBBYBBBKKKKKKBBBYBBbK.", ".KbBBBYYBBBBBBBBYYBBBbK.",
    ".KbBBBBBBBBBBBBBBBBBBbK.", ".KbbbbbbbbbbbbbbbbbbbbK.", ".KKKKKKKKKKKKKKKKKKKKKK.",
    "..KyKyKyKK....KKyKyKyK..", "..KyKyKyKK....KKyKyKyK..", "..KKKKKKKK....KKKKKKKK..",
};
const CorPixel CORES_COMPILADOR[] = {
    {'B', {39, 79, 163, 255}},  {'b', {72, 139, 224, 255}},  {'O', {116, 224, 255, 255}},
    {'o', {35, 103, 154, 255}}, {'Y', {211, 242, 255, 255}}, {'y', {74, 165, 214, 255}},
};

// Boss verde, H100 COLOSSUS: servidor de IA com olho, racks e rede neural.
const char *const H100_COLOSSUS[] = {
    "..................KKKK..", ".................KRWWRK.", "...KKKKKKKKKKKKKKKRWWRK.",
    "..KKBBBBBBBBBBBBBKRRRRK.", "..KBbbbKbbbbbbbbKKRWWRK.", "..KBbBBBKKKKKKKKBKKKKKK.",
    "..KBbBBKKKKKKKKKKBBBBK..", "..KBbBKKWWWWWWWWKKBBBK..", "..KBbBKWWIIIIIIWWKBBBK..",
    "..KBbBKWWIWPPPIWWKBBBK..", "..KBbBKWWIPPPPIWWKBBBK..", "..KBbBKWWIPPPPIWWKBBBK..",
    "..KBbBKWWIIIIIIWWKBBBK..", "..KBbBKKWWWWWWWWKKBBBK..", "..KBbBBKKKKKKKKKKBBBBK..",
    "..KBbBBBBBBBBBBBBBBBBK..", "..KBBBBBBBBBBBBBBBBBBK..", "..KKBBBBBBBBBBBBBBBBKK..",
    "...KKKKKKKKKKKKKKKKKK...", ".....K...K....K...K.....", "....K....K....K....K....",
    "....K....K....K....K....", "..CKC...CKC..CKC...CKC..", "..CCC...CCC..CCC...CCC..",
};
const CorPixel CORES_H100[] = {
    {'B', {35, 116, 61, 255}},  {'b', {77, 199, 105, 255}},  {'W', {230, 250, 233, 255}},
    {'P', {15, 35, 22, 255}},   {'I', {135, 255, 157, 255}}, {'R', {194, 255, 92, 255}},
    {'C', {92, 230, 118, 255}},
};

// Boss vermelho, WANNACRY PRIME: cofre-servidor corrompido e acorrentado.
const char *const WANNACRY_PRIME[] = {
    ".KKKKKKKKKKKKKKKKKKKKKK.", ".KwwwwwwwwwwwwwwwwwwwwK.", ".KwddddddddddddddddddwK.",
    "KKwdSSSSSSSSSSSSSSSSdwKK", "KKwdSSSSSSSSSSSSSbbSdwKK", ".KwdSSSWWSSSSSSWWSSSdwK.",
    ".KwdSSSWWSSSSSSWWSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.",
    ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdSSSSWWWWWWWWSSSSdwK.", ".KwdSSSWSSSSSSSSWSSSdwK.",
    ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdSSSSSSSSSSSSSSSSdwK.", ".KwdWdWdWdWdWdWdWdWddwK.",
    ".KdRdRdRddddddddddddddK.", ".KddddddddddddddddddddK.", ".KKKKKKKKKKKKKKKKKKKKKK.",
    "........KKKKKKKK........", "........KKKKKKKK....KY..", "....KKKKKKKKKKKKKKKK.KY.",
    "....KddddddddddddddK..KY", "....KKKKKKKKKKKKKKKK...K", "........................",
};
const CorPixel CORES_WANNACRY[] = {
    {'w', {176, 176, 188, 255}}, {'d', {100, 100, 114, 255}}, {'S', {150, 28, 48, 255}},
    {'b', {246, 76, 88, 255}},   {'W', {255, 232, 234, 255}}, {'R', {255, 55, 66, 255}},
    {'Y', {255, 164, 86, 255}},
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
} // namespace

void SpriteInimigo::desenhar(SDL_Renderer *renderizador, TipoInimigo tipo, Afinidade afinidade,
                             const SDL_FRect &area)
{
    if (tipo == TipoInimigo::VIRUS)
    {
        if (afinidade == Afinidade::VERMELHA)
            desenhar_bitmap(renderizador, ILOVEYOU, CORES_ILOVEYOU, area);
        else if (afinidade == Afinidade::AZUL)
            desenhar_bitmap(renderizador, EULA, CORES_EULA, area);
        else
            desenhar_bitmap(renderizador, NPU, CORES_NPU, area);
    }
    else if (tipo == TipoInimigo::BRUTO)
    {
        if (afinidade == Afinidade::VERMELHA)
            desenhar_bitmap(renderizador, MIRAI_SWARM, CORES_MIRAI, area);
        else if (afinidade == Afinidade::AZUL)
            desenhar_bitmap(renderizador, SENTINELA_DRM, CORES_DRM, area);
        else
            desenhar_bitmap(renderizador, TPU_SENTINEL, CORES_TPU, area);
    }
    else
    {
        if (afinidade == Afinidade::VERMELHA)
            desenhar_bitmap(renderizador, WANNACRY_PRIME, CORES_WANNACRY, area);
        else if (afinidade == Afinidade::AZUL)
            desenhar_bitmap(renderizador, BOSS_COMPILADOR, CORES_COMPILADOR, area);
        else
            desenhar_bitmap(renderizador, H100_COLOSSUS, CORES_H100, area);
    }
}
