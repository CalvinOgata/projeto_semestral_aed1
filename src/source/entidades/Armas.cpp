#include "entidades/Armas.h"

Armas::Armas() : Armas(TipoArma::PISTOLA)
{
}

Armas::Armas(TipoArma tipo) : tipo(tipo)
{
    set_municao(tipo == TipoArma::PISTOLA ? 50 : 40);
    set_dano(tipo == TipoArma::ESPADA ? 3 : (tipo == TipoArma::ARCO || tipo == TipoArma::LASER ? 2 : 1));
    set_quantidadeDisparos(tipo == TipoArma::SHOTGUN ? 3 : (tipo == TipoArma::DUPLA ? 2 : 1));
}

TipoArma Armas::get_tipo() const
{
    return tipo;
}
const char *Armas::get_nome() const
{
    switch (tipo)
    {
    case TipoArma::SHOTGUN:
        return "SHOTGUN";
    case TipoArma::LASER:
        return "LASER";
    case TipoArma::DUPLA:
        return "DUPLA";
    case TipoArma::RICOCHETE:
        return "RICOCHETE";
    case TipoArma::ESPADA:
        return "ESPADA";
    case TipoArma::ARCO:
        return "ARCO";
    case TipoArma::BOOMERANG:
        return "BOOMERANG";
    default:
        return "PISTOLA";
    }
}
float Armas::get_intervalo() const
{
    switch (tipo)
    {
    case TipoArma::SHOTGUN:
        return 0.5f;
    case TipoArma::LASER:
        return 0.35f;
    case TipoArma::ESPADA:
        return 0.4f;
    case TipoArma::ARCO:
        return 0.6f;
    case TipoArma::BOOMERANG:
        return 0.7f;
    default:
        return 0.14f;
    }
}
bool Armas::municao_infinita() const
{
    return tipo == TipoArma::ESPADA || tipo == TipoArma::BOOMERANG;
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
    if (municao_infinita() || this->get_municao() > 0)
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
    // Um cartucho por ataque, independente do numero de projeteis.
    if (!municao_infinita())
    {
        this->set_municao(this->get_municao() - 1);
    }
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
