#include "entidades/Armas.h"

Armas::Armas() : Armas(TipoArma::PISTOLA)
{
}

Armas::Armas(TipoArma tipo) : tipo(tipo)
{
    set_municao(tipo == TipoArma::PISTOLA ? 50 : 40);
    set_dano(tipo == TipoArma::ESPADA ? 3 :
        (tipo == TipoArma::ARCO || tipo == TipoArma::LASER || tipo == TipoArma::PULSO ? 2 : 1));
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
        return "ESCOPETA CLAMAV";
    case TipoArma::LASER:
        return "RAIO VIRUSTOTAL";
    case TipoArma::DUPLA:
        return "GARFO GIT";
    case TipoArma::RICOCHETE:
        return "CANHAO CRT";
    case TipoArma::ESPADA:
        return "LAMINA DE TORVALDS";
    case TipoArma::ARCO:
        return "ARCO APACHE";
    case TipoArma::BOOMERANG:
        return "DISQUETE BUMERANGUE";
    case TipoArma::PULSO:
        return "PULSO MALWAREBYTES";
    default:
        return "ZAPPER 8-BITS";
    }
}

Afinidade Armas::get_afinidade() const
{
    switch (tipo)
    {
    case TipoArma::SHOTGUN:
    case TipoArma::LASER:
    case TipoArma::PULSO:
        return Afinidade::VERMELHA;
    case TipoArma::DUPLA:
    case TipoArma::ESPADA:
    case TipoArma::ARCO:
        return Afinidade::AZUL;
    case TipoArma::PISTOLA:
    case TipoArma::RICOCHETE:
    case TipoArma::BOOMERANG:
        return Afinidade::VERDE;
    }
    return Afinidade::NEUTRA;
}

AlcanceArma Armas::get_alcance() const
{
    switch (tipo)
    {
    case TipoArma::SHOTGUN:
    case TipoArma::ESPADA:
    case TipoArma::BOOMERANG:
        return AlcanceArma::CURTO;
    case TipoArma::PISTOLA:
    case TipoArma::DUPLA:
    case TipoArma::PULSO:
        return AlcanceArma::MEDIO;
    case TipoArma::LASER:
    case TipoArma::RICOCHETE:
    case TipoArma::ARCO:
        return AlcanceArma::LONGO;
    }
    return AlcanceArma::MEDIO;
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
    case TipoArma::PULSO:
        return 0.45f;
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
