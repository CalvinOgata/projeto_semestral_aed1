#include "entidades/Obstaculo.h"

#include <algorithm>

#include "config/Config.h"

Obstaculo::Obstaculo(TipoObstaculo tipo, float centroX, float centroY, bool vertical)
    : tipo(tipo), vida(tipo == TipoObstaculo::CAIXOTE ? VIDA_CAIXOTE : VIDA_BARREIRA),
      vidaMax(vida)
{
    float largura = 40, altura = 40;
    if (tipo == TipoObstaculo::BARREIRA)
    {
        largura = vertical ? 28 : 120;
        altura = vertical ? 120 : 28;
    }
    this->area = {centroX - largura / 2, centroY - altura / 2, largura, altura};
}

bool Obstaculo::get_intacto() const
{
    return this->vida > 0;
}

void Obstaculo::levar_dano(int quantidade)
{
    this->vida = std::max(0, this->vida - std::max(0, quantidade));
}

SDL_FRect Obstaculo::get_retangulo() const
{
    return this->area;
}

SDL_FPoint Obstaculo::separar(const SDL_FRect &corpo) const
{
    if (!this->get_intacto())
    {
        return {0, 0};
    }
    const float sobreposicaoX = std::min(corpo.x + corpo.w - area.x, area.x + area.w - corpo.x);
    const float sobreposicaoY = std::min(corpo.y + corpo.h - area.y, area.y + area.h - corpo.y);
    if (sobreposicaoX <= 0 || sobreposicaoY <= 0)
    {
        return {0, 0};
    }
    // Sai pelo eixo de menor penetracao, permitindo deslizar ao longo da face.
    if (sobreposicaoX < sobreposicaoY)
    {
        const bool esquerda = corpo.x + corpo.w / 2 < area.x + area.w / 2;
        return {esquerda ? -sobreposicaoX : sobreposicaoX, 0};
    }
    const bool acima = corpo.y + corpo.h / 2 < area.y + area.h / 2;
    return {0, acima ? -sobreposicaoY : sobreposicaoY};
}

void Obstaculo::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitbox) const
{
    if (renderizador == nullptr || !this->get_intacto())
    {
        return;
    }
    const SDL_FRect tela = camera.para_tela(this->area);
    auto bloco = [&](float x, float y, float w, float h, SDL_Color cor) {
        SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
        const SDL_FRect ret = {tela.x + x, tela.y + y, w, h};
        SDL_RenderFillRectF(renderizador, &ret);
    };
    // Sombra no piso para destacar o volume.
    bloco(4, tela.h - 2, tela.w, 6, {10, 12, 18, 140});
    const bool danificado = this->vida * 2 <= this->vidaMax;
    if (this->tipo == TipoObstaculo::CAIXOTE)
    {
        bloco(0, 0, tela.w, tela.h, {58, 34, 24, 255});
        bloco(3, 3, tela.w - 6, tela.h - 6, {156, 98, 52, 255});
        // Tabuas horizontais e reforco diagonal.
        bloco(3, tela.h / 3, tela.w - 6, 2, {110, 66, 36, 255});
        bloco(3, tela.h * 2 / 3, tela.w - 6, 2, {110, 66, 36, 255});
        for (int i = 0; i < 8; ++i)
        {
            const float passo = (tela.w - 14) / 8;
            bloco(5 + i * passo, 5 + i * passo, passo + 2, 4, {196, 136, 72, 255});
        }
        bloco(0, 0, 6, 6, {120, 124, 140, 255});
        bloco(tela.w - 6, 0, 6, 6, {120, 124, 140, 255});
        bloco(0, tela.h - 6, 6, 6, {120, 124, 140, 255});
        bloco(tela.w - 6, tela.h - 6, 6, 6, {120, 124, 140, 255});
        // Etiqueta de pacote de dados.
        bloco(tela.w - 16, 8, 9, 6, {226, 232, 240, 255});
        bloco(tela.w - 14, 10, 5, 2, {60, 140, 220, 255});
    }
    else
    {
        // Firewall: tijolos de metal com rejunte incandescente.
        bloco(0, 0, tela.w, tela.h, {36, 18, 22, 255});
        const bool vertical = tela.h > tela.w;
        const float tijolo = 18;
        const int linhas = static_cast<int>((vertical ? tela.w : tela.h) / 9);
        const float comprimento = vertical ? tela.h : tela.w;
        for (int linha = 0; linha < linhas; ++linha)
        {
            for (float p = (linha % 2) * -tijolo / 2; p < comprimento - 2; p += tijolo)
            {
                const float inicio = std::max(2.0f, p + 1), fim = std::min(comprimento - 2, p + tijolo - 1);
                if (fim <= inicio)
                {
                    continue;
                }
                const SDL_Color cor = (linha + static_cast<int>(p)) % 3 == 0 ?
                    SDL_Color{150, 56, 46, 255} : SDL_Color{122, 44, 40, 255};
                if (vertical)
                    bloco(2 + linha * 8.0f, inicio, 7, fim - inicio, cor);
                else
                    bloco(inicio, 2 + linha * 8.0f, fim - inicio, 7, cor);
            }
        }
        // Faixa de alerta acesa no topo.
        if (vertical)
            bloco(tela.w / 2 - 1, 2, 3, tela.h - 4, {255, 150, 60, 200});
        else
            bloco(2, tela.h / 2 - 1, tela.w - 4, 3, {255, 150, 60, 200});
    }
    if (danificado)
    {
        // Rachaduras indicam que mais alguns acertos quebram o obstaculo.
        bloco(tela.w * 0.3f, tela.h * 0.2f, 3, tela.h * 0.35f, {24, 14, 14, 255});
        bloco(tela.w * 0.3f, tela.h * 0.55f, tela.w * 0.25f, 3, {24, 14, 14, 255});
        bloco(tela.w * 0.55f, tela.h * 0.55f, 3, tela.h * 0.25f, {24, 14, 14, 255});
    }
    if (mostrarHitbox)
    {
        SDL_SetRenderDrawColor(renderizador, 100, 255, 140, 255);
        SDL_RenderDrawRectF(renderizador, &tela);
    }
}
