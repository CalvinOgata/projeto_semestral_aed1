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

Armas FilaArmas::get_Arma(int indice) const
{
    if(this->Vazio())
    {
        // Sem armas guardadas nao existe indice valido para devolver.
        return Armas();
    }
    if(indice > this->get_indiceUltimo())
    {
        return this->vetor[this->ultimo];
    }
    if(indice < 0)
    {
        return this->vetor[0];
    }
    return this->vetor[indice];
}

Armas* FilaArmas::get_ponteiro(int indice)
{
    if(this->Vazio() || indice < 0 || indice > this->get_indiceUltimo())
    {
        return nullptr;
    }
    return &this->vetor[indice];
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
    arma = this->vetor[0];

    for(int i = 1; i <= this->ultimo; i++)
    {
        this->vetor[i-1] = this->vetor[i];
    }
    this->ultimo--;
    return true;
}
