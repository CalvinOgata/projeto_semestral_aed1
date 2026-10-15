#ifndef COMBATE_H
#define COMBATE_H

#include "entidades/Inimigo.h"
#include "entidades/Jogador.h"
#include "entidades/Projetil.h"
#include "estruturas/ListaSequencial.h"

// Dono das entidades temporarias e das regras de combate.
// As listas cuidam do armazenamento; esta classe decide quando inserir/remover.
class Combate
{
private:
    ListaSequencial<Projetil> projeteis;
    ListaSequencial<Inimigo> inimigos;

    void atualizar_projeteis(float dt);
    void atualizar_inimigos(float dt, const Jogador &jogador);
    void resolver_colisoes();
    void limpar_inativos();

public:
    void reiniciar(const Jogador &jogador);
    void disparar(Jogador &jogador);
    void atualizar(float dt, const Jogador &jogador);
    void desenhar(SDL_Renderer *renderizador, const Camera &camera) const;
    int get_inimigosVivos() const;
};

#endif
