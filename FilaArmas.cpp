#include "FilaArmas.h"

FilaArmas::FilaArmas()
{
    this->ultimo = -1;
}

FilaArmas::~FilaArmas()
{
    this->ultimo = -1;
}

int FilaArmas::get_indiceUltimo() const
{
    return this->ultimo;
}

Armas FilaArmas::get_Arma(int indice) const
{
    if(indice > this->get_indiceUltimo())
    {
        return this->vetor[this->ultimo];
    }
    else if(indice < 0)
    {
        return this->vetor[0];
    }
    return this->vetor[indice]
}

bool FilaArmas::Vazio() const
{
    if(this->get_indiceUltimo() == -1)
    {
        return true;
    }
    return false;
}

bool FilaArmas::Cheio() const
{
    if(this->get_indiceUltimo() == MAX_ARMAS-1)
    {
        return true;
    }
    return false;
}

bool FilaArmas::Inserir(Armas arma)
{
    if(this->Cheio())
    {
        return false;
    }
    this->ultimo++;
    this->vetor[this->ultimo] = arma;
    return true;
}

bool FilaArmas::Retirar(Armas &arma)
{
    if(this->Vazio())
    {
        return false;
    }
    arma = this->get_Arma(0);

    Armas aux;
    for(int i = 1; i <= this->ultimo;; i++)
    {
        aux = this->get_Arma(i);
        this->vetor[i-1] = aux;
    }
    return true;
}