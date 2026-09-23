#include "Armas.h"

Armas::Armas()
{
    this->set_municao(50);
    this->set_dano(1);
    this->set_quantidadeDisparos(1);
}

Armas::~Armas()
{
    this->sem_municao(0);
}

int Armas::get_municao() const
{
    return this->municao;
}

int Armas::get_dano() const
{
    return this->dano;
}

int Armas::get_quantidadeDisparos() const
{
    return this->quantidadeDisparos;
}

bool Armas::sem_municao() const
{
    if(this->get_municao() > 0)
    {
        return false;
    }
    return true;
}

void Armas::atirar() 
{
    if(!this->sem_municao())
    {
        set_municao(this->get_municao() - this->quantidadeDisparos);
    }
}

void Armas::set_dano(int valor)
{
    if(valor < 1)
    {
        this->dano = 1;
    }
    else
    {
        this->dano = valor;
    }
}

void Armas::set_municao(int quantidade)
{
    if(quantidade < 1)
    {
        this->municao = 1;
    }
    else
    {
        this->municao = quantidade;
    }
}

void Armas::set_quantidadeDisparos(int quantidade)
{
    if(quantidade < 1)
    {
        this->quantidadeDisparos = 1;
    }
    else
    {
        this->quantidadeDisparos = quantidade;
    }
}