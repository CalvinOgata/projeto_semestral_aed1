#include "entidades/Armas.h"

Armas::Armas()
{
    this->set_municao(50);
    this->set_dano(1);
    this->set_quantidadeDisparos(1);
}

Armas::~Armas()
{
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
    if (this->get_municao() > 0)
    {
        return false;
    }
    return true;
}

bool Armas::atirar()
{
    if (this->sem_municao())
    {
        return false;
    }
    this->set_municao(this->get_municao() - this->get_quantidadeDisparos());
    return true;
}

void Armas::recarregar(int quantidade)
{
    if (quantidade > 0)
    {
        this->set_municao(this->get_municao() + quantidade);
    }
}

void Armas::set_dano(int valor)
{
    if (valor < 1)
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
    // Precisa aceitar zero, senao sem_municao() nunca seria verdadeiro.
    if (quantidade < 0)
    {
        this->municao = 0;
    }
    else
    {
        this->municao = quantidade;
    }
}

void Armas::set_quantidadeDisparos(int quantidade)
{
    if (quantidade < 1)
    {
        this->quantidadeDisparos = 1;
    }
    else
    {
        this->quantidadeDisparos = quantidade;
    }
}
