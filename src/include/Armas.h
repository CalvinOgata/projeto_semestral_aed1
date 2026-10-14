#ifndef ARMAS_H
#define ARMAS_H

class Armas
{
private:
    int municao;
    int dano;
    int quantidadeDisparos;

public:
    Armas();
    ~Armas();

    int get_municao() const;
    int get_dano() const;
    int get_quantidadeDisparos() const;
    bool sem_municao() const;

    // Consome municao. Devolve true quando o disparo aconteceu.
    bool atirar();
    void recarregar(int);

    void set_dano(int);
    void set_municao(int);
    void set_quantidadeDisparos(int);
};

#endif
