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
    // Armas com que a fase atual comecou; Reiniciar e morte voltam a elas.
    FilaArmas armasInicioFase;

public:
    // Inventario vazio inicia com as armas padrao do jogador.
    void iniciar(int concluidas, Jogador &jogador, const FilaArmas &armas = FilaArmas());
    bool atualizar_transicao(Jogador &jogador);
    void reiniciar_fase(Jogador &jogador);
    bool concluida() const;
    int get_fasesConcluidas() const;
    const FilaArmas &get_armasInicioFase() const;
    Fase &get_faseAtual();
    const Fase &get_faseAtual() const;
};

#endif
