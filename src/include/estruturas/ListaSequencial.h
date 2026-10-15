#ifndef LISTA_SEQUENCIAL_H
#define LISTA_SEQUENCIAL_H

#include <algorithm>
#include <cstddef>
#include <vector>

// Lista dinamica sequencial apoiada em std::vector (nao e uma lista encadeada).
// Acesso O(1), insercao ao final O(1) amortizado, filtragem O(n).
template <typename T> class ListaSequencial
{
private:
    std::vector<T> elementos;

public:
    void inserir(const T &elemento)
    {
        this->elementos.push_back(elemento);
    }
    void limpar()
    {
        this->elementos.clear();
    }
    std::size_t get_quantidade() const
    {
        return this->elementos.size();
    }
    bool vazio() const
    {
        return this->elementos.empty();
    }

    // Indice invalido gera std::out_of_range.
    T &obter(std::size_t indice)
    {
        return this->elementos.at(indice);
    }
    const T &obter(std::size_t indice) const
    {
        return this->elementos.at(indice);
    }

    // Preserva a ordem dos sobreviventes. Nao chamar durante um percurso.
    template <typename Predicado> void remover_se(Predicado criterio)
    {
        this->elementos.erase(
            std::remove_if(this->elementos.begin(), this->elementos.end(), criterio),
            this->elementos.end());
    }

    // Insercao/remocao pode invalidar referencias e iteradores.
    auto begin()
    {
        return this->elementos.begin();
    }
    auto end()
    {
        return this->elementos.end();
    }
    auto begin() const
    {
        return this->elementos.begin();
    }
    auto end() const
    {
        return this->elementos.end();
    }
};

#endif
