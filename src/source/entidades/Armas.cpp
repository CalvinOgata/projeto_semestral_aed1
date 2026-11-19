#include "entidades/Armas.h"

Armas::Armas() : Armas(TipoArma::PISTOLA)
{
}

Armas::Armas(TipoArma tipo) : tipo(tipo)
{
    set_municao(tipo == TipoArma::PISTOLA ? 50 : 40);
    const int danos[] = {1, 1, 3, 4, 3, 3, 2, 3, 4};
    set_dano(danos[static_cast<int>(tipo)]);
    set_quantidadeDisparos(tipo == TipoArma::SHOTGUN ? 3 : 1);
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
        return "CLAMS DO CLAMAV";
    case TipoArma::LASER:
        return "RAIO AVIRA";
    case TipoArma::DUPLA:
        return "FORK OPEN SOURCE";
    case TipoArma::RICOCHETE:
        return "CANHAO CRT";
    case TipoArma::ESPADA:
        return "LAMINAS MALWAREBYTES";
    case TipoArma::ARCO:
        return "PENAS APACHE";
    case TipoArma::BOOMERANG:
        return "CHICOTE MACINTOSH";
    case TipoArma::PULSO:
        return "RIFLE WIRESHARK";
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
    case TipoArma::ESPADA:
        return Afinidade::VERMELHA;
    case TipoArma::DUPLA:
    case TipoArma::PULSO:
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
    case TipoArma::DUPLA:
    case TipoArma::ESPADA:
    case TipoArma::BOOMERANG:
        return AlcanceArma::CURTO;
    case TipoArma::PISTOLA:
    case TipoArma::SHOTGUN:
    case TipoArma::ARCO:
        return AlcanceArma::MEDIO;
    case TipoArma::LASER:
    case TipoArma::RICOCHETE:
    case TipoArma::PULSO:
        return AlcanceArma::LONGO;
    }
    return AlcanceArma::MEDIO;
}
float Armas::get_intervalo() const
{
    const float intervalos[] = {0.20f, 0.50f, 0.65f, 0.50f, 0.65f, 0.38f, 0.45f, 0.65f, 0.75f};
    return intervalos[static_cast<int>(tipo)];
}
float Armas::get_alcancePixels() const
{
    const float alcances[] = {420, 320, 900, 116, 1050, 96, 480, 150, 900};
    return alcances[static_cast<int>(tipo)];
}
float Armas::get_aberturaGraus() const
{
    if (tipo == TipoArma::ESPADA) return 120;
    if (tipo == TipoArma::DUPLA) return 40;
    if (tipo == TipoArma::BOOMERANG) return 70;
    return 0;
}
float Armas::get_velocidadeProjetil() const
{
    if (get_alcance() == AlcanceArma::CURTO || tipo == TipoArma::LASER) return 0;
    if (tipo == TipoArma::SHOTGUN) return 420;
    if (tipo == TipoArma::ARCO) return 580;
    if (tipo == TipoArma::PULSO) return 850;
    return 520;
}
bool Armas::municao_infinita() const
{
    return get_alcance() == AlcanceArma::CURTO;
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
