#include "mundo/Campanha.h"

#include <stdexcept>

void Campanha::iniciar(int concluidas, Jogador &jogador)
{
    if (concluidas < 0 || concluidas > TOTAL_FASES)
    {
        throw std::invalid_argument("Progresso de campanha invalido");
    }
    this->fasesConcluidas = concluidas;
    if (!this->concluida())
    {
        this->fase = Fase(concluidas + 1);
        this->fase.reiniciar(jogador);
    }
}

bool Campanha::atualizar_transicao(Jogador &jogador)
{
    if (this->concluida() || !this->fase.atualizar_transicao(jogador))
    {
        return false;
    }
    if (this->fase.get_concluida())
    {
        ++this->fasesConcluidas;
        if (!this->concluida())
        {
            this->fase = Fase(this->fasesConcluidas + 1);
            this->fase.reiniciar(jogador);
        }
    }
    return true;
}

void Campanha::reiniciar_fase(Jogador &jogador)
{
    if (!this->concluida())
    {
        this->fase.reiniciar(jogador);
    }
}

bool Campanha::concluida() const
{
    return this->fasesConcluidas == TOTAL_FASES;
}

int Campanha::get_fasesConcluidas() const
{
    return this->fasesConcluidas;
}

Fase &Campanha::get_faseAtual()
{
    return this->fase;
}

const Fase &Campanha::get_faseAtual() const
{
    return this->fase;
}
