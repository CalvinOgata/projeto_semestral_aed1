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

int FilaArmas::get_quantidade() const
{
    return this->get_indiceUltimo() + 1;
}

Armas FilaArmas::get_arma(int indice) const
{
    if (this->vazio())
    {
        // Sem armas guardadas nao existe indice valido para devolver.
        return Armas();
    }
    if (indice > this->get_indiceUltimo())
    {
        return this->vetor[this->ultimo];
    }
    if (indice < 0)
    {
        return this->vetor[0];
    }
    return this->vetor[indice];
}

Armas *FilaArmas::get_ponteiro(int indice)
{
    if (this->vazio() || indice < 0 || indice > this->get_indiceUltimo())
    {
        return nullptr;
    }
    return &this->vetor[indice];
}

bool FilaArmas::vazio() const
{
    if (this->get_indiceUltimo() == -1)
    {
        return true;
    }
    return false;
}

bool FilaArmas::cheio() const
{
    if (this->get_indiceUltimo() == MAX_ARMAS - 1)
    {
        return true;
    }
    return false;
}

bool FilaArmas::inserir(Armas arma)
{
    if (this->cheio())
    {
        return false;
    }
    this->ultimo++;
    this->vetor[this->ultimo] = arma;
    return true;
}

bool FilaArmas::retirar(Armas &arma)
{
    if (this->vazio())
    {
        return false;
    }
    arma = this->vetor[0];

    for (int i = 1; i <= this->ultimo; i++)
    {
        this->vetor[i - 1] = this->vetor[i];
    }
    this->ultimo--;
    return true;
}
