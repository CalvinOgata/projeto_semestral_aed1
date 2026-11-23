#ifndef ARMAS_H
#define ARMAS_H

#include "entidades/Afinidade.h"

enum class TipoArma
{
    PISTOLA,
    SHOTGUN,
    LASER,
    DUPLA,
    RICOCHETE,
    ESPADA,
    ARCO,
    BOOMERANG,
    // Acrescentada no fim para preservar os indices 0..7 dos saves antigos.
    PULSO
};

enum class AlcanceArma
{
    CURTO,
    MEDIO,
    LONGO
};

class Armas
{
private:
    TipoArma tipo;
    int municao;
    int dano;
    int quantidadeDisparos;

public:
    Armas();
    explicit Armas(TipoArma tipo);
    TipoArma get_tipo() const;
    const char *get_nome() const;
    Afinidade get_afinidade() const;
    AlcanceArma get_alcance() const;
    float get_intervalo() const;
    float get_alcancePixels() const;
    float get_aberturaGraus() const; // leque do golpe corpo a corpo
    float get_velocidadeProjetil() const;
    bool municao_infinita() const;
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
