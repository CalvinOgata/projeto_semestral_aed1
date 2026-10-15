#include "mundo/Fase.h"

#include <string>
#include <stdexcept>

#include "config/CampanhaConfig.h"

Fase::Fase(int numero) : numero(numero)
{
    if (numero < 1 || numero > QUANTIDADE_FASES)
    {
        throw std::invalid_argument("Numero de fase invalido");
    }
    const int extra = numero - 1;
    // Fase autoral inicial. Cada registro pode ter tamanho, dificuldade e requisito proprios.
    this->salas.inserir(Sala({"SETOR DE BOOT", 936, 516, 1 + extra, 4 + extra * 2, {true}}));
    this->salas.inserir(Sala({"REDE INFECTADA", 1120, 640, 2 + extra, 7 + extra * 2, {true}}));
    this->salas.inserir(Sala({"NUCLEO EM QUARENTENA", 1440, 800, 3 + extra, 10 + extra * 2, {true}}));
}

void Fase::reiniciar(Jogador &jogador)
{
    *this = Fase(this->numero);
    jogador = Jogador();
    this->get_salaAtual().entrar(jogador);
}

bool Fase::atualizar_transicao(Jogador &jogador)
{
    Sala &atual = this->get_salaAtual();
    if (this->concluida || !atual.liberada())
    {
        return false;
    }
    const bool direita = atual.tocando_porta(jogador, true);
    const bool esquerda = this->indiceAtual > 0 && atual.tocando_porta(jogador, false);
    if (!direita && !esquerda)
    {
        return false;
    }
    atual.sair();
    if (direita && this->indiceAtual + 1 == this->salas.get_quantidade())
    {
        this->concluida = true;
        return true;
    }
    if (direita)
    {
        ++this->indiceAtual;
    }
    else
    {
        --this->indiceAtual;
    }
    this->get_salaAtual().entrar(jogador, esquerda);
    return true;
}

Sala &Fase::get_salaAtual()
{
    return this->salas.obter(this->indiceAtual);
}

const Sala &Fase::get_salaAtual() const
{
    return this->salas.obter(this->indiceAtual);
}

std::size_t Fase::get_indiceAtual() const
{
    return this->indiceAtual;
}

bool Fase::get_concluida() const
{
    return this->concluida;
}
