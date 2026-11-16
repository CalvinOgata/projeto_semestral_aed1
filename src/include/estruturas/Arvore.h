#ifndef ARVORE_H
#define ARVORE_H

#include <cstddef>
#include <memory>
#include <vector>

// TAD arvore enraizada generica. Cada valor aparece no maximo uma vez e cada
// no, exceto a raiz, possui exatamente um pai.
template <typename T> class Arvore
{
private:
    struct No
    {
        T valor;
        No *pai;
        std::vector<std::unique_ptr<No>> filhos;

        No(const T &valor, No *pai) : valor(valor), pai(pai)
        {
        }
    };

    std::unique_ptr<No> raiz;
    std::size_t quantidade = 0;

    static No *buscar_no(No *atual, const T &valor)
    {
        if (atual == nullptr || atual->valor == valor)
        {
            return atual;
        }
        for (auto &filho : atual->filhos)
        {
            if (No *encontrado = buscar_no(filho.get(), valor))
            {
                return encontrado;
            }
        }
        return nullptr;
    }

    static const No *buscar_no(const No *atual, const T &valor)
    {
        if (atual == nullptr || atual->valor == valor)
        {
            return atual;
        }
        for (const auto &filho : atual->filhos)
        {
            if (const No *encontrado = buscar_no(filho.get(), valor))
            {
                return encontrado;
            }
        }
        return nullptr;
    }

    static void percorrer_no(const No *atual, std::vector<T> &ordem)
    {
        if (atual == nullptr)
        {
            return;
        }
        ordem.push_back(atual->valor);
        for (const auto &filho : atual->filhos)
        {
            percorrer_no(filho.get(), ordem);
        }
    }

public:
    bool definir_raiz(const T &valor)
    {
        if (this->raiz != nullptr)
        {
            return false;
        }
        this->raiz = std::make_unique<No>(valor, nullptr);
        this->quantidade = 1;
        return true;
    }

    bool inserir_filho(const T &valorPai, const T &valorFilho)
    {
        No *pai = buscar_no(this->raiz.get(), valorPai);
        if (pai == nullptr || buscar_no(this->raiz.get(), valorFilho) != nullptr)
        {
            return false;
        }
        pai->filhos.push_back(std::make_unique<No>(valorFilho, pai));
        ++this->quantidade;
        return true;
    }

    const T *obter_filho(const T &valorPai, const T &valorFilho) const
    {
        const No *pai = buscar_no(this->raiz.get(), valorPai);
        if (pai == nullptr)
        {
            return nullptr;
        }
        for (const auto &filho : pai->filhos)
        {
            if (filho->valor == valorFilho)
            {
                return &filho->valor;
            }
        }
        return nullptr;
    }

    const T *obter_pai(const T &valor) const
    {
        const No *no = buscar_no(this->raiz.get(), valor);
        return no == nullptr || no->pai == nullptr ? nullptr : &no->pai->valor;
    }

    const T *buscar(const T &valor) const
    {
        const No *no = buscar_no(this->raiz.get(), valor);
        return no == nullptr ? nullptr : &no->valor;
    }

    bool contem(const T &valor) const
    {
        return this->buscar(valor) != nullptr;
    }

    std::vector<T> percorrer_pre_ordem() const
    {
        std::vector<T> ordem;
        ordem.reserve(this->quantidade);
        percorrer_no(this->raiz.get(), ordem);
        return ordem;
    }

    std::size_t contar_nos() const
    {
        return this->quantidade;
    }

    void limpar()
    {
        this->raiz.reset();
        this->quantidade = 0;
    }
};

#endif
