#ifndef TELA_DERROTA_H
#define TELA_DERROTA_H

#include <SDL2/SDL.h>

// Animacao quando o Tux cai: o sistema "e invadido" (falhas na imagem, janelas
// de erro, enxame de virus) e depois vem a mensagem de incentivo. E desenhada por
// cima da partida congelada; o jogo so reinicia a fase quando ela termina.
class TelaDerrota
{
public:
    static constexpr float DURACAO = 8.5f;
    static constexpr float INICIO_PULO = 2.4f; // antes disso Enter nao pula

private:
    float tempo = 0.0f;
    int vidasRestantes = 0;
    int semente = 0;
    bool encerrada = false;

public:
    void iniciar(int vidasRestantes, int semente);
    void atualizar(float dt);
    void pular(); // Enter: vai direto para o fim, se ja passou da invasao
    bool terminou() const;
    void desenhar(SDL_Renderer *renderizador) const;
};

#endif
