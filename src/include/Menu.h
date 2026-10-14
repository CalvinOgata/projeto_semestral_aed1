#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

// O que o menu pede ao jogo depois de tratar um evento.
enum class AcaoMenu
{
    NENHUMA,
    CONTINUAR,
    SAIR
};

class Menu
{
public:
    static constexpr int NUM_OPCOES = 2;

private:
    int selecionado;

public:
    Menu();
    ~Menu();

    void reiniciar();

    int get_selecionado() const;
    void mover_selecao(int);

    AcaoMenu tratar_evento(const SDL_Event &);
    void desenhar(SDL_Renderer *) const;

private:
    AcaoMenu acao_da_opcao(int) const;
    SDL_Rect retangulo_painel() const;
    SDL_Rect retangulo_opcao(int) const;
};

#endif
