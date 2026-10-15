#ifndef FILAARMAS_H
#define FILAARMAS_H

#include "entidades/Armas.h"
#include "estruturas/Fila.h"

constexpr int MAX_ARMAS = 5;

// Adaptador do inventario: a primeira arma da fila e a equipada.
// As regras de armazenamento FIFO pertencem ao TAD Fila.
class FilaArmas
{
private:
    Fila<Armas, MAX_ARMAS> armas;

public:
    int get_quantidade() const;
    bool vazio() const;
    bool cheio() const;
    bool inserir(const Armas &arma);
    bool retirar(Armas &arma);
    bool alternar();
    Armas *frente();
    const Armas *frente() const;
};

#endif
