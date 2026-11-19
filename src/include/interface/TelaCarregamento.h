#ifndef TELA_CARREGAMENTO_H
#define TELA_CARREGAMENTO_H

#include <SDL2/SDL.h>

class Sprite;

// Abertura do jogo: log de "boot" com tema de computacao, barra de progresso
// com o Tux andando e dicas. O progresso real (sintese do audio) e combinado
// com uma duracao minima, para a tela nunca piscar rapido demais.
class TelaCarregamento
{
public:
    static constexpr float DURACAO_MINIMA = 3.6f;
    static constexpr float DURACAO_SAIDA = 0.7f;

private:
    float tempo = 0.0f;
    float exibido = 0.0f; // progresso suavizado mostrado na barra
    int linhasVisiveis = 0;
    float tempoPronto = -1.0f; // instante em que tudo terminou
    bool pularPedido = false;

public:
    void reiniciar();
    // Recebe o progresso real e devolve quantas linhas novas do log apareceram.
    int atualizar(float dt, float progressoReal);
    void pular();          // tecla ou clique: encurta a espera depois de pronto
    bool pronta() const;   // barra cheia, mostrando "SISTEMA PRONTO"
    bool terminou() const; // transicao de saida concluida
    void desenhar(SDL_Renderer *renderizador, const Sprite &tux) const;
};

#endif
