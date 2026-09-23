#ifndef JOGADOR_H
#define JOGADOR_H

class Jogador{
private:
    int vida;
    int estamina;
    int vidaMax;
    int estaminaMax;
    
    public:
    Jogador();
    ~Jogador();

    void rec_vida(int);
    void rec_estamina(int);
    void perder_vida(int);
    void perder_estamina(int);
    void mudar_vidaMax(int);
    void mudar_estaminaMax(int);

    int get_estamina() const;
    int get_vida() const;
    int get_vidaMax() const;
    int get_estaminaMax() const;
    bool Morto() const;
};

#endif