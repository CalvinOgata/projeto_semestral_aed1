#ifndef CAMPANHA_H
#define CAMPANHA_H

#include "config/CampanhaConfig.h"
#include "mundo/Fase.h"

class Campanha
{
public:
    static constexpr int TOTAL_FASES = QUANTIDADE_FASES;

private:
    int fasesConcluidas = 0;
    Fase fase;

public:
    void iniciar(int concluidas, Jogador &jogador);
    bool atualizar_transicao(Jogador &jogador);
    void reiniciar_fase(Jogador &jogador);
    bool concluida() const;
    int get_fasesConcluidas() const;
    Fase &get_faseAtual();
    const Fase &get_faseAtual() const;
};

#endif
