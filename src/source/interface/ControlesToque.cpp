#include "interface/ControlesToque.h"

#include <algorithm>
#include <cmath>

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
// Botao redondo desenhado na tela, em coordenadas logicas (960x540).
struct Botao
{
    float x;
    float y;
    float raio;
    const char *rotulo;
};

const Botao BOTAO_ATIRAR = {858.0f, 418.0f, 62.0f, ""};
const Botao BOTAO_PEGAR = {760.0f, 350.0f, 32.0f, "PEGAR"};
const Botao BOTAO_TROCAR = {748.0f, 472.0f, 32.0f, "TROCAR"};
const Botao BOTAO_SOLTAR = {872.0f, 296.0f, 28.0f, "SOLTAR"};
const Botao BOTAO_PAUSA = {448.0f, 32.0f, 22.0f, ""};
const Botao BOTAO_MAPA = {512.0f, 32.0f, 22.0f, "MAPA"};

// Barra de armas no canto superior direito, com a seta de troca a esquerda:
// tocar nela troca a arma.
const SDL_FRect AREA_INVENTARIO = {692.0f, 6.0f, 266.0f, 96.0f};

constexpr float RAIO_JOYSTICK = 64.0f;
constexpr float RAIO_ALAVANCA = 28.0f;
constexpr float ZONA_MORTA = 0.15f;
constexpr float FOLGA_TOQUE = 12.0f; // dedos sao menos precisos que o desenho
constexpr float BASE_PADRAO_X = 130.0f;
constexpr float BASE_PADRAO_Y = 410.0f;

bool tocou(const Botao &botao, float x, float y)
{
    const float raio = botao.raio + FOLGA_TOQUE;
    return std::hypot(x - botao.x, y - botao.y) <= raio;
}

// Circulo preenchido como um leque de triangulos.
void preencher_circulo(SDL_Renderer *r, float cx, float cy, float raio, SDL_Color cor)
{
    constexpr int SEGMENTOS = 32;
    SDL_Vertex vertices[SEGMENTOS + 1];
    int indices[SEGMENTOS * 3];
    vertices[0] = {{cx, cy}, cor, {0.0f, 0.0f}};
    for (int i = 0; i < SEGMENTOS; ++i)
    {
        const float angulo = static_cast<float>(i) * 2.0f * static_cast<float>(M_PI) / SEGMENTOS;
        vertices[i + 1] = {
            {cx + raio * std::cos(angulo), cy + raio * std::sin(angulo)}, cor, {0.0f, 0.0f}};
        indices[i * 3] = 0;
        indices[i * 3 + 1] = i + 1;
        indices[i * 3 + 2] = (i + 1) % SEGMENTOS + 1;
    }
    SDL_RenderGeometry(r, nullptr, vertices, SEGMENTOS + 1, indices, SEGMENTOS * 3);
}

void contornar_circulo(SDL_Renderer *r, float cx, float cy, float raio, SDL_Color cor)
{
    constexpr int SEGMENTOS = 40;
    SDL_FPoint pontos[SEGMENTOS + 1];
    for (int i = 0; i <= SEGMENTOS; ++i)
    {
        const float angulo = static_cast<float>(i) * 2.0f * static_cast<float>(M_PI) / SEGMENTOS;
        pontos[i] = {cx + raio * std::cos(angulo), cy + raio * std::sin(angulo)};
    }
    SDL_SetRenderDrawColor(r, cor.r, cor.g, cor.b, cor.a);
    SDL_RenderDrawLinesF(r, pontos, SEGMENTOS + 1);
}

void desenhar_botao(SDL_Renderer *r, const Botao &botao, bool aceso)
{
    const Uint8 alfa = aceso ? 150 : 90;
    preencher_circulo(r, botao.x, botao.y, botao.raio, {12, 18, 34, alfa});
    contornar_circulo(r, botao.x, botao.y, botao.raio,
                      aceso ? SDL_Color{255, 222, 110, 230} : SDL_Color{200, 214, 240, 150});
    contornar_circulo(r, botao.x, botao.y, botao.raio - 1.0f,
                      aceso ? SDL_Color{255, 222, 110, 160} : SDL_Color{200, 214, 240, 90});
    if (botao.rotulo[0] != '\0')
    {
        Texto::desenhar_centralizado(r, botao.rotulo, static_cast<int>(botao.x),
                                     static_cast<int>(botao.y) - Texto::altura(1) / 2, 1,
                                     aceso ? SDL_Color{255, 236, 170, 255}
                                           : SDL_Color{225, 232, 248, 210});
    }
}
} // namespace

ControlesToque::ControlesToque()
    : baseX(BASE_PADRAO_X), baseY(BASE_PADRAO_Y), alavancaX(0.0f), alavancaY(0.0f)
{
}

ControlesToque::Dedo *ControlesToque::buscar_dedo(SDL_FingerID id)
{
    for (Dedo &dedo : this->dedos)
    {
        if (dedo.funcao != Funcao::LIVRE && dedo.id == id)
        {
            return &dedo;
        }
    }
    return nullptr;
}

ControlesToque::Dedo *ControlesToque::dedo_livre()
{
    for (Dedo &dedo : this->dedos)
    {
        if (dedo.funcao == Funcao::LIVRE)
        {
            return &dedo;
        }
    }
    return nullptr;
}

AcaoToque ControlesToque::tratar_evento(const SDL_Event &evento)
{
    if (evento.type != SDL_FINGERDOWN && evento.type != SDL_FINGERUP &&
        evento.type != SDL_FINGERMOTION)
    {
        return AcaoToque::NENHUMA;
    }
    // O SDL entrega o dedo normalizado (0..1) sobre a area logica do jogo.
    const float x = evento.tfinger.x * LARGURA_TELA;
    const float y = evento.tfinger.y * ALTURA_TELA;
    Dedo *dedo = this->buscar_dedo(evento.tfinger.fingerId);

    if (evento.type == SDL_FINGERMOTION)
    {
        if (dedo && dedo->funcao == Funcao::JOYSTICK)
        {
            this->mover_alavanca(x, y);
        }
        return AcaoToque::NENHUMA;
    }
    if (evento.type == SDL_FINGERUP)
    {
        if (!dedo)
        {
            return AcaoToque::NENHUMA;
        }
        if (dedo->funcao == Funcao::JOYSTICK)
        {
            this->joystickAtivo = false;
            this->alavancaX = 0.0f;
            this->alavancaY = 0.0f;
            this->baseX = BASE_PADRAO_X;
            this->baseY = BASE_PADRAO_Y;
        }
        else if (dedo->funcao == Funcao::ATIRAR)
        {
            this->atirando = false;
        }
        dedo->funcao = Funcao::LIVRE;
        return AcaoToque::NENHUMA;
    }

    // SDL_FINGERDOWN: um dedo novo decide sua funcao pelo lugar onde encostou.
    if (dedo == nullptr)
    {
        dedo = this->dedo_livre();
        if (dedo == nullptr)
        {
            return AcaoToque::NENHUMA;
        }
        dedo->id = evento.tfinger.fingerId;
    }
    dedo->funcao = Funcao::BOTAO;
    if (tocou(BOTAO_PAUSA, x, y))
    {
        return AcaoToque::PAUSAR;
    }
    if (tocou(BOTAO_MAPA, x, y))
    {
        return AcaoToque::MINIMAPA;
    }
    if (tocou(BOTAO_PEGAR, x, y))
    {
        return AcaoToque::PEGAR_ARMA;
    }
    if (tocou(BOTAO_TROCAR, x, y) ||
        (x >= AREA_INVENTARIO.x && y >= AREA_INVENTARIO.y &&
         x < AREA_INVENTARIO.x + AREA_INVENTARIO.w && y < AREA_INVENTARIO.y + AREA_INVENTARIO.h))
    {
        return AcaoToque::TROCAR_ARMA;
    }
    if (tocou(BOTAO_SOLTAR, x, y))
    {
        return AcaoToque::SOLTAR_ARMA;
    }
    if (x < LARGURA_TELA / 2.0f)
    {
        if (this->joystickAtivo)
        {
            dedo->funcao = Funcao::LIVRE; // ja ha um dedo cuidando do movimento
            return AcaoToque::NENHUMA;
        }
        // A base nasce sob o dedo, sem deixar o circulo sair da tela.
        dedo->funcao = Funcao::JOYSTICK;
        this->joystickAtivo = true;
        this->baseX = std::clamp(x, RAIO_JOYSTICK + 8.0f, LARGURA_TELA / 2.0f - RAIO_JOYSTICK);
        this->baseY = std::clamp(y, RAIO_JOYSTICK + 8.0f, ALTURA_TELA - RAIO_JOYSTICK - 8.0f);
        this->mover_alavanca(x, y);
        return AcaoToque::NENHUMA;
    }
    // Qualquer outro ponto da metade direita tambem atira: o polegar nao
    // precisa acertar o botao em cheio no meio da luta.
    dedo->funcao = Funcao::ATIRAR;
    this->atirando = true;
    return AcaoToque::NENHUMA;
}

void ControlesToque::mover_alavanca(float x, float y)
{
    float dx = x - this->baseX;
    float dy = y - this->baseY;
    const float distancia = std::hypot(dx, dy);
    if (distancia > RAIO_JOYSTICK)
    {
        dx *= RAIO_JOYSTICK / distancia;
        dy *= RAIO_JOYSTICK / distancia;
    }
    this->alavancaX = dx;
    this->alavancaY = dy;
    if (distancia / RAIO_JOYSTICK > ZONA_MORTA)
    {
        this->ultimaDirecaoX = dx / std::hypot(dx, dy);
        this->ultimaDirecaoY = dy / std::hypot(dx, dy);
    }
}

void ControlesToque::soltar_tudo()
{
    for (Dedo &dedo : this->dedos)
    {
        dedo.funcao = Funcao::LIVRE;
    }
    this->joystickAtivo = false;
    this->atirando = false;
    this->alavancaX = 0.0f;
    this->alavancaY = 0.0f;
    this->baseX = BASE_PADRAO_X;
    this->baseY = BASE_PADRAO_Y;
}

void ControlesToque::get_direcao(float &x, float &y) const
{
    x = this->alavancaX / RAIO_JOYSTICK;
    y = this->alavancaY / RAIO_JOYSTICK;
    if (std::hypot(x, y) <= ZONA_MORTA)
    {
        x = 0.0f;
        y = 0.0f;
    }
}

void ControlesToque::get_ultimaDirecao(float &x, float &y) const
{
    x = this->ultimaDirecaoX;
    y = this->ultimaDirecaoY;
}

bool ControlesToque::get_atirando() const
{
    return this->atirando;
}

void ControlesToque::desenhar(SDL_Renderer *r, bool armaPerto) const
{
    SDL_BlendMode anterior;
    SDL_GetRenderDrawBlendMode(r, &anterior);
    SDL_SetRenderDrawBlendMode(r, SDL_BLENDMODE_BLEND);

    // Joystick: base translucida e alavanca que segue o dedo.
    const Uint8 alfaBase = this->joystickAtivo ? 110 : 60;
    preencher_circulo(r, this->baseX, this->baseY, RAIO_JOYSTICK, {12, 18, 34, alfaBase});
    contornar_circulo(r, this->baseX, this->baseY, RAIO_JOYSTICK, {120, 200, 255, alfaBase});
    preencher_circulo(
        r, this->baseX + this->alavancaX, this->baseY + this->alavancaY, RAIO_ALAVANCA,
        this->joystickAtivo ? SDL_Color{120, 200, 255, 170} : SDL_Color{200, 214, 240, 90});

    // Atirar: alvo desenhado no centro do botao grande.
    desenhar_botao(r, BOTAO_ATIRAR, this->atirando);
    const SDL_Color mira =
        this->atirando ? SDL_Color{255, 222, 110, 240} : SDL_Color{235, 88, 111, 210};
    contornar_circulo(r, BOTAO_ATIRAR.x, BOTAO_ATIRAR.y, 20.0f, mira);
    contornar_circulo(r, BOTAO_ATIRAR.x, BOTAO_ATIRAR.y, 19.0f, mira);
    preencher_circulo(r, BOTAO_ATIRAR.x, BOTAO_ATIRAR.y, 5.0f, mira);
    SDL_SetRenderDrawColor(r, mira.r, mira.g, mira.b, mira.a);
    SDL_RenderDrawLineF(r, BOTAO_ATIRAR.x - 32.0f, BOTAO_ATIRAR.y, BOTAO_ATIRAR.x - 12.0f,
                        BOTAO_ATIRAR.y);
    SDL_RenderDrawLineF(r, BOTAO_ATIRAR.x + 12.0f, BOTAO_ATIRAR.y, BOTAO_ATIRAR.x + 32.0f,
                        BOTAO_ATIRAR.y);
    SDL_RenderDrawLineF(r, BOTAO_ATIRAR.x, BOTAO_ATIRAR.y - 32.0f, BOTAO_ATIRAR.x,
                        BOTAO_ATIRAR.y - 12.0f);
    SDL_RenderDrawLineF(r, BOTAO_ATIRAR.x, BOTAO_ATIRAR.y + 12.0f, BOTAO_ATIRAR.x,
                        BOTAO_ATIRAR.y + 32.0f);

    desenhar_botao(r, BOTAO_PEGAR, armaPerto);
    desenhar_botao(r, BOTAO_TROCAR, false);
    desenhar_botao(r, BOTAO_SOLTAR, false);
    desenhar_botao(r, BOTAO_MAPA, false);

    // Pausa: duas barras verticais.
    desenhar_botao(r, BOTAO_PAUSA, false);
    SDL_SetRenderDrawColor(r, 225, 232, 248, 220);
    const SDL_FRect barras[] = {{BOTAO_PAUSA.x - 7.0f, BOTAO_PAUSA.y - 8.0f, 5.0f, 16.0f},
                                {BOTAO_PAUSA.x + 2.0f, BOTAO_PAUSA.y - 8.0f, 5.0f, 16.0f}};
    SDL_RenderFillRectsF(r, barras, 2);

    SDL_SetRenderDrawBlendMode(r, anterior);
}
