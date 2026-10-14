#ifndef FILAARMAS_H
#define FILAARMAS_H

#include "Armas.h"

constexpr int MAX_ARMAS = 5;

class FilaArmas{
private:
    int ultimo;
    Armas vetor[MAX_ARMAS];

public:
    FilaArmas();
    ~FilaArmas();

    int get_indiceUltimo() const;
    int get_quantidade() const;
    Armas get_Arma(int) const;

    // Acesso direto para alterar a arma guardada (gastar municao, recarregar).
    // Devolve nullptr se a fila estiver vazia ou o indice nao existir.
    Armas* get_ponteiro(int);

    bool Vazio() const;
    bool Cheio() const;

    bool Inserir(Armas);
    bool Retirar(Armas&);
};

#endif
