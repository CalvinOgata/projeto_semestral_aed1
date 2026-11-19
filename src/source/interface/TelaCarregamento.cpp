#include "interface/TelaCarregamento.h"

#include <algorithm>
#include <cmath>
#include <string>

#include "config/Config.h"
#include "graficos/Sprite.h"
#include "interface/FundoMenu.h"
#include "interface/Texto.h"

namespace
{
struct LinhaBoot
{
    const char *texto;
    bool alerta;
};

const LinhaBoot LINHAS[] = {
    {"DETECTANDO PINGUIM... TUX ENCONTRADO", false}, {"MONTANDO /DEV/COMPUTADOR", false},
    {"DESCOMPACTANDO SPRITES EM PIXEL ART", false},  {"SINTETIZANDO EFEITOS SONOROS", false},
    {"COMPONDO TRILHA SONORA DE 8 BITS", false},     {"ATUALIZANDO ASSINATURAS DE VIRUS", false},
    {"3 SISTEMAS MALIGNOS DETECTADOS!", true},       {"ARMANDO ANTIVIRUS E FIREWALL", false},
    {"COMPILANDO CORAGEM COM -O2", false},
};
constexpr int QUANTIDADE_LINHAS = static_cast<int>(sizeof(LINHAS) / sizeof(LINHAS[0]));
constexpr int LINHAS_NO_TERMINAL = 6;

const char *DICAS[] = {
    "ARMAS DA MESMA COR DO INIMIGO CAUSAM DANO EM DOBRO",
    "O ESCUDO RECARREGA SOZINHO APOS 3 SEGUNDOS SEM DANO",
    "INIMIGOS DERROTADOS SOLTAM CORACOES: PEGUE ANTES QUE SUMAM",
    "USE CAIXOTES E FIREWALLS COMO COBERTURA",
    "APERTE M PARA VER O MAPA DA FASE",
    "Q TROCA DE ARMA, F PEGA DO CHAO E G SOLTA",
    "CHEGUE PERTO DE UM BAU PARA ABRI-LO",
};
constexpr int QUANTIDADE_DICAS = static_cast<int>(sizeof(DICAS) / sizeof(DICAS[0]));
constexpr float DURACAO_DICA = 2.4f;
constexpr float ESPERA_APOS_PRONTO = 1.1f;

constexpr SDL_Rect BARRA = {200, 392, 560, 22}; // o Tux anda logo acima
constexpr int SEGMENTOS = 28;

Uint8 alfa(float valor)
{
    return static_cast<Uint8>(std::clamp(valor, 0.0f, 255.0f));
}

void desenhar_spinner(SDL_Renderer *r, int cx, int cy, float tempo, float opacidade)
{
    constexpr int PONTOS = 8;
    const int ativo = static_cast<int>(tempo * 12.0f) % PONTOS;
    for (int i = 0; i < PONTOS; ++i)
    {
        const float angulo = static_cast<float>(i) / PONTOS * 6.2831853f;
        const int x = cx + static_cast<int>(std::cos(angulo) * 9.0f);
        const int y = cy + static_cast<int>(std::sin(angulo) * 9.0f);
        const int distancia = (ativo - i + PONTOS) % PONTOS;
        const float brilho = 1.0f - static_cast<float>(distancia) / PONTOS;
        FundoMenu::preencher(r, {x - 2, y - 2, 4, 4},
                             {90, 230, 255, alfa(255 * brilho * opacidade)});
    }
}
} // namespace

void TelaCarregamento::reiniciar()
{
    *this = TelaCarregamento();
}

int TelaCarregamento::atualizar(float dt, float progressoReal)
{
    this->tempo += dt;
    const float alvo = std::min(progressoReal, this->tempo / DURACAO_MINIMA);
    this->exibido += (alvo - this->exibido) * std::min(1.0f, dt * 7.0f);
    if (alvo >= 1.0f && this->exibido > 0.995f)
    {
        this->exibido = 1.0f;
    }
    if (this->exibido >= 1.0f && this->tempoPronto < 0.0f)
    {
        this->tempoPronto = this->tempo;
    }
    const int linhas =
        std::min(QUANTIDADE_LINHAS, static_cast<int>(this->exibido * QUANTIDADE_LINHAS + 0.6f));
    const int novas = linhas - this->linhasVisiveis;
    this->linhasVisiveis = linhas;
    return std::max(0, novas);
}

void TelaCarregamento::pular()
{
    this->pularPedido = true;
}

bool TelaCarregamento::pronta() const
{
    return this->tempoPronto >= 0.0f;
}

bool TelaCarregamento::terminou() const
{
    if (!this->pronta())
    {
        return false;
    }
    const float espera = this->pularPedido ? 0.0f : ESPERA_APOS_PRONTO;
    return this->tempo - this->tempoPronto >= espera + DURACAO_SAIDA;
}

void TelaCarregamento::desenhar(SDL_Renderer *r, const Sprite &tux) const
{
    // Saida: os elementos somem e o veu escuro revela o cenario do menu.
    float saida = 0.0f;
    if (this->pronta())
    {
        const float espera = this->pularPedido ? 0.0f : ESPERA_APOS_PRONTO;
        saida = std::clamp((this->tempo - this->tempoPronto - espera) / DURACAO_SAIDA, 0.0f, 1.0f);
    }
    const float opacidade = 1.0f - saida;
    const float entrada = std::clamp(this->tempo / 0.8f, 0.0f, 1.0f);

    FundoMenu::desenhar(r, this->tempo, 0.4f + 0.6f * this->exibido);
    FundoMenu::preencher(r, {0, 0, LARGURA_TELA, ALTURA_TELA},
                         {4, 4, 12, alfa((235 - 120 * this->exibido) * opacidade)});

    // Logotipo que se materializa.
    FundoMenu::desenhar_logo(r, LARGURA_TELA / 2, 52, 9, this->tempo,
                             alfa(255 * entrada * opacidade));
    Texto::desenhar_centralizado(r, "MAKE COMPUTERS GREAT AGAIN", LARGURA_TELA / 2, 136, 2,
                                 {200, 190, 255, alfa(220 * entrada * opacidade)});

    // Janela de terminal com o log de inicializacao.
    const SDL_Rect terminal = {180, 170, 600, 168};
    FundoMenu::preencher(r, terminal, {6, 10, 16, alfa(215 * opacidade)});
    FundoMenu::preencher(r, {terminal.x, terminal.y, terminal.w, 16},
                         {40, 60, 90, alfa(230 * opacidade)});
    for (int i = 0; i < 3; ++i)
    {
        const SDL_Color cores[] = {{255, 95, 90, 255}, {255, 200, 70, 255}, {90, 220, 110, 255}};
        FundoMenu::preencher(r, {terminal.x + 8 + i * 14, terminal.y + 4, 8, 8},
                             {cores[i].r, cores[i].g, cores[i].b, alfa(255 * opacidade)});
    }
    Texto::desenhar(r, "TUX - MCGA:/BOOT", terminal.x + 56, terminal.y + 5, 1,
                    {200, 220, 240, alfa(230 * opacidade)});
    SDL_SetRenderDrawColor(r, 60, 120, 170, alfa(200 * opacidade));
    SDL_RenderDrawRect(r, &terminal);

    const int primeira = std::max(0, this->linhasVisiveis - LINHAS_NO_TERMINAL);
    int y = terminal.y + 26;
    for (int i = primeira; i < this->linhasVisiveis; ++i)
    {
        const LinhaBoot &linha = LINHAS[i];
        const SDL_Color corEtiqueta =
            linha.alerta ? SDL_Color{255, 190, 60, 255} : SDL_Color{90, 240, 130, 255};
        Texto::desenhar(r, linha.alerta ? "[ !! ]" : "[ OK ]", terminal.x + 14, y, 2,
                        {corEtiqueta.r, corEtiqueta.g, corEtiqueta.b, alfa(255 * opacidade)});
        Texto::desenhar(r, linha.texto, terminal.x + 98, y, 2,
                        linha.alerta ? SDL_Color{255, 150, 130, alfa(255 * opacidade)}
                                     : SDL_Color{210, 220, 232, alfa(255 * opacidade)});
        y += 22;
    }
    // Cursor piscando na linha seguinte.
    if (std::fmod(this->tempo, 0.8f) < 0.45f && y < terminal.y + terminal.h - 14)
    {
        FundoMenu::preencher(r, {terminal.x + 14, y + 10, 10, 3},
                             {200, 255, 210, alfa(255 * opacidade)});
    }

    // Estado e porcentagem acima da barra.
    const bool pronto = this->pronta();
    if (pronto)
    {
        const bool pisca = std::fmod(this->tempo, 0.4f) < 0.28f;
        Texto::desenhar(r, "SISTEMA PRONTO!", BARRA.x, BARRA.y + 36, 2,
                        {120, 255, 160, alfa((pisca ? 255 : 140) * opacidade)});
    }
    else
    {
        desenhar_spinner(r, BARRA.x + 9, BARRA.y + 43, this->tempo, opacidade);
        const int pontos = static_cast<int>(this->tempo * 3.0f) % 4;
        Texto::desenhar(r, "CARREGANDO" + std::string(static_cast<std::size_t>(pontos), '.'),
                        BARRA.x + 26, BARRA.y + 36, 2, {200, 220, 255, alfa(255 * opacidade)});
    }
    const std::string porcentagem = std::to_string(static_cast<int>(this->exibido * 100.0f)) + "%";
    Texto::desenhar(r, porcentagem, BARRA.x + BARRA.w - Texto::largura(porcentagem, 2),
                    BARRA.y + 36, 2, {255, 220, 120, alfa(255 * opacidade)});

    // Barra segmentada com degrade ciano -> magenta e um brilho que percorre.
    FundoMenu::preencher(r, BARRA, {10, 12, 30, alfa(230 * opacidade)});
    SDL_SetRenderDrawColor(r, 90, 200, 255, alfa(220 * opacidade));
    const SDL_Rect moldura = {BARRA.x - 3, BARRA.y - 3, BARRA.w + 6, BARRA.h + 6};
    SDL_RenderDrawRect(r, &moldura);
    const int larguraSegmento = BARRA.w / SEGMENTOS;
    const int cheios = static_cast<int>(this->exibido * SEGMENTOS + 0.001f);
    const float brilhoPos = std::fmod(this->tempo * 1.2f, 1.6f) - 0.3f;
    for (int i = 0; i < SEGMENTOS; ++i)
    {
        const SDL_Rect segmento = {BARRA.x + i * larguraSegmento + 2, BARRA.y + 3,
                                   larguraSegmento - 4, BARRA.h - 6};
        if (i >= cheios)
        {
            FundoMenu::preencher(r, segmento, {30, 34, 64, alfa(160 * opacidade)});
            continue;
        }
        const float t = static_cast<float>(i) / (SEGMENTOS - 1);
        const float realce = std::max(0.0f, 1.0f - std::abs(t - brilhoPos) * 6.0f);
        const float flash =
            pronto ? std::max(0.0f, 1.0f - (this->tempo - this->tempoPronto) * 2.5f) : 0.0f;
        const float claro = std::min(1.0f, realce * 0.6f + flash);
        const float vermelho = 60 + 195 * t, verde = 220 - 160 * t, azul = 255 - 40 * t;
        const SDL_Color cor = {alfa(vermelho + (255 - vermelho) * claro),
                               alfa(verde + (255 - verde) * claro),
                               alfa(azul + (255 - azul) * claro), alfa(255 * opacidade)};
        FundoMenu::preencher(r, segmento, cor);
        FundoMenu::preencher(r, {segmento.x, segmento.y, segmento.w, 3},
                             {255, 255, 255, alfa(90 * opacidade)});
    }

    // O Tux caminha sobre a barra acompanhando o progresso.
    const float passo = std::abs(std::sin(this->tempo * 10.0f));
    const float tuxX = BARRA.x + BARRA.w * this->exibido - 20.0f;
    const float tuxY = BARRA.y - 54.0f - (pronto ? 0.0f : passo * 5.0f);
    if (opacidade > 0.01f)
    {
        tux.desenhar(r, {tuxX, tuxY, 40.0f, 50.0f});
    }

    // Dica que troca de tempos em tempos.
    const int dica = static_cast<int>(this->tempo / DURACAO_DICA) % QUANTIDADE_DICAS;
    const float fase = std::fmod(this->tempo, DURACAO_DICA);
    const float fade = std::min({1.0f, fase * 4.0f, (DURACAO_DICA - fase) * 4.0f});
    Texto::desenhar_centralizado(r, std::string("DICA: ") + DICAS[dica], LARGURA_TELA / 2, 474, 2,
                                 {170, 180, 220, alfa(230 * fade * opacidade)});
    Texto::desenhar_centralizado(r, "UM PROJETO DE ALGORITMOS E ESTRUTURAS DE DADOS 1",
                                 LARGURA_TELA / 2, 510, 1, {120, 120, 160, alfa(200 * opacidade)});
}
