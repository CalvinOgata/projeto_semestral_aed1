#ifndef FASE_H
#define FASE_H

#include "estruturas/ListaSequencial.h"
#include "mundo/Sala.h"

// Salas e conexoes direcionais. A lista armazena; Fase decide a navegacao.
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
    void atualizar(float dt, Jogador &jogador);
    // Verifica contato com portas liberadas uma vez por atualizacao.
    bool atualizar_transicao(Jogador &jogador);
    Sala &get_salaAtual();
    const Sala &get_salaAtual() const;
    std::size_t get_indiceAtual() const;
    bool get_concluida() const;
    std::size_t get_quantidadeSalas() const;
    const Sala &get_sala(std::size_t indice) const;
    int get_vizinho(std::size_t indice, Direcao direcao) const;
};

#endif
