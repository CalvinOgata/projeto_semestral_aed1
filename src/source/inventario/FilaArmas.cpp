#include "inventario/FilaArmas.h"

int FilaArmas::get_quantidade() const
{
    return static_cast<int>(this->armas.get_quantidade());
}

bool FilaArmas::vazio() const
{
    return this->armas.vazio();
}
bool FilaArmas::cheio() const
{
    return this->armas.cheio();
}
bool FilaArmas::inserir(const Armas &arma)
{
    return this->armas.inserir(arma);
}
bool FilaArmas::retirar(Armas &arma)
{
    return this->armas.retirar(arma);
}
Armas *FilaArmas::frente()
{
    return this->armas.frente();
}
const Armas *FilaArmas::frente() const
{
    return this->armas.frente();
}
