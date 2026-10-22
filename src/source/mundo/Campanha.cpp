#include "mundo/Campanha.h"

#include <stdexcept>

void Campanha::iniciar(int concluidas, Jogador &jogador, const FilaArmas &armas)
{
    if (concluidas < 0 || concluidas > TOTAL_FASES)
    {
        throw std::invalid_argument("Progresso de campanha invalido");
    }
    this->fasesConcluidas = concluidas;
    this->armasInicioFase = armas.vazio() ? Jogador().get_armas() : armas;
    if (!this->concluida())
    {
        this->fase = Fase(concluidas + 1);
        this->fase.reiniciar(jogador, this->armasInicioFase);
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
            // O inventario acompanha o jogador para a proxima fase.
            this->armasInicioFase = jogador.get_armas();
            this->fase = Fase(this->fasesConcluidas + 1);
            this->fase.reiniciar(jogador, this->armasInicioFase);
        }
    }
    return true;
}

void Campanha::reiniciar_fase(Jogador &jogador)
{
    if (!this->concluida())
    {
        this->fase.reiniciar(jogador, this->armasInicioFase);
    }
}

bool Campanha::concluida() const
{
    return this->fasesConcluidas == TOTAL_FASES;
}

const FilaArmas &Campanha::get_armasInicioFase() const
{
    return this->armasInicioFase;
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
