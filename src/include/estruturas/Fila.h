#ifndef FILA_H
#define FILA_H

#include <array>
#include <cstddef>

// TAD FIFO limitado, implementado com vetor circular, sem dependencias do jogo.
// Inserir/retirar/frente: O(1). Nao desloca os elementos ao retirar.
template <typename T, std::size_t Capacidade> class Fila
{
    static_assert(Capacidade > 0, "A fila precisa de capacidade positiva");

private:
    std::array<T, Capacidade> elementos{};
    std::size_t inicio = 0;
    std::size_t quantidade = 0;

public:
    bool vazio() const
    {
        return this->quantidade == 0;
    }
    bool cheio() const
    {
        return this->quantidade == Capacidade;
    }
    std::size_t get_quantidade() const
    {
        return this->quantidade;
    }

    bool inserir(const T &elemento)
    {
        if (this->cheio())
        {
            return false;
        }
        this->elementos[(this->inicio + this->quantidade) % Capacidade] = elemento;
        ++this->quantidade;
        return true;
    }

    // Em fila vazia, preserva o parametro de saida.
    bool retirar(T &elemento)
    {
        if (this->vazio())
        {
            return false;
        }
        elemento = this->elementos[this->inicio];
        this->inicio = (this->inicio + 1) % Capacidade;
        --this->quantidade;
        return true;
    }

    T *frente()
    {
        return this->vazio() ? nullptr : &this->elementos[this->inicio];
    }
    const T *frente() const
    {
        return this->consultar(0);
    }

    // Inspecao em ordem logica, sem permitir alterar a ordem FIFO.
    // Ponteiros devem ser consultados novamente apos inserir/retirar.
    const T *consultar(std::size_t indice) const
    {
        if (indice >= this->quantidade)
        {
            return nullptr;
        }
        return &this->elementos[(this->inicio + indice) % Capacidade];
    }
};

#endif
