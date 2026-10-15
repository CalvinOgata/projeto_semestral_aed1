#ifndef FASE_H
#define FASE_H

#include "estruturas/ListaSequencial.h"
#include "mundo/Sala.h"

// Ordem de progressao e sala ativa. A lista armazena; Fase decide a navegacao.
class Fase
{
private:
    ListaSequencial<Sala> salas;
    std::size_t indiceAtual = 0;
    bool concluida = false;

public:
    Fase();
    void reiniciar(Jogador &jogador);
    bool interagir(Jogador &jogador);
    Sala &get_salaAtual();
    const Sala &get_salaAtual() const;
    std::size_t get_indiceAtual() const;
    bool get_concluida() const;
    void desenhar_painel(SDL_Renderer *renderizador, const Jogador &jogador) const;
};

#endif
