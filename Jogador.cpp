#include "Jogador.h"

Jogador::Jogador()
{
    this->mudar_vidaMax(100);
    this->mudar_estaminaMax(100);
    this->rec_vida(100);
    this->rec_estamina(100);
}

Jogador::~Jogador()
{
    this->perder_vida(this->vida);
    this->perder_estamina(this->estamina);
}

void Jogador::rec_vida(int quantidade)
{
    this->vida += quantidade;

    if(this->get_vida() > this->get_vidaMax())
    {
        this->vida = this->vidaMax;
    }
}

void Jogador::rec_estamina(int quantidade)
{
    this->estamina += quantidade;

    if(this->get_estamina() > this->get_estaminaMax())
    {
        this->estamina = this->estaminaMax;
    }
}

void Jogador::perder_vida(int quantidade)
{
    this->vida -= quantidade;
    
    if(this->vida < 0)
    {
        this->vida = 0;
    }
}

void Jogador::perder_estamina(int quantidade)
{
    this->estamina -= quantidade;

    if(this->estamina < 0)
    {
        this->estamina = 0;
    }
}

void Jogador::mudar_vidaMax(int quantidade)
{
    this->vidaMax += quantidade;

    if(this->get_vidaMax() < 1)
    {
        this->vidaMax = 1;
    }
}

void Jogador::mudar_estaminaMax(int quantidade)
{
    this->estaminaMax += quantidade;

    if(this->get_estaminaMax() < 1)
    {
        this->estaminaMax() = 1;
    }
}

int Jogador::get_vida () const
{
    return this->vida;
}

int Jogador::get_estamina() const
{
    return this->estamina;
}

int Jogador::get_vidaMax() const
{
    return this->vidaMax;
}

int Jogador::get_estaminaMax() const
{
    return this->estaminaMax;
}

bool Jogador::Morto () const
{
    if(this->get_vida() > 0)
    {
        return false;
    }
    return true;
}