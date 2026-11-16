#ifndef AFINIDADE_H
#define AFINIDADE_H

// Cor tematica compartilhada por armas e inimigos. A classe/forca do inimigo
// continua independente desta afinidade.
enum class Afinidade
{
    VERMELHA,
    AZUL,
    VERDE,
    NEUTRA
};

// A afinidade correta dobra o dano. Ataques neutros, como o soco, e
// combinacoes diferentes preservam integralmente o dano-base.
constexpr int calcular_danoAfinidade(int danoBase, Afinidade ataque, Afinidade alvo)
{
    if (danoBase <= 0)
    {
        return 0;
    }
    return ataque != Afinidade::NEUTRA && ataque == alvo ? danoBase * 2 : danoBase;
}

#endif
