#ifndef DIMENSOES_ENTIDADE_H
#define DIMENSOES_ENTIDADE_H

// Pixels logicos: hitbox de colisao e tamanho do quadro visual sao independentes.
// O centro da imagem coincide com o centro da hitbox.
struct DimensoesEntidade
{
    int larguraHitbox;
    int alturaHitbox;
    int larguraSprite;
    int alturaSprite;

    constexpr bool valida() const
    {
        return larguraHitbox > 0 && alturaHitbox > 0 &&
               larguraSprite >= larguraHitbox && alturaSprite >= alturaHitbox;
    }
};

constexpr DimensoesEntidade DIMENSOES_HEROI = {32, 32, 48, 48};
constexpr DimensoesEntidade DIMENSOES_INIMIGO_COMUM = {28, 28, 48, 48};
// Perfis reservados para futuras entidades; nao adicionam novos inimigos as fases.
constexpr DimensoesEntidade DIMENSOES_GUARDA_COSTAS = {44, 44, 64, 64};
constexpr DimensoesEntidade DIMENSOES_BOSS = {88, 88, 128, 128};

static_assert(DIMENSOES_HEROI.valida() && DIMENSOES_INIMIGO_COMUM.valida() &&
              DIMENSOES_GUARDA_COSTAS.valida() && DIMENSOES_BOSS.valida());

#endif
