#ifndef FASE_H
#define FASE_H

#include "estruturas/ListaSequencial.h"
#include "mundo/Sala.h"

// Ordem de progressao e sala ativa. A lista armazena; Fase decide a navegacao.
class Fase
{
private:
    int numero;
    ListaSequencial<Sala> salas;
    std::size_t indiceAtual = 0;
    bool concluida = false;

public:
    explicit Fase(int numero = 1);
    void reiniciar(Jogador &jogador);
    // Verifica contato com portas liberadas uma vez por atualizacao.
    bool atualizar_transicao(Jogador &jogador);
    Sala &get_salaAtual();
    const Sala &get_salaAtual() const;
    std::size_t get_indiceAtual() const;
    bool get_concluida() const;
};

#endif
