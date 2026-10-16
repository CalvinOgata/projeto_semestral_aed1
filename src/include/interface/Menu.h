#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>
#include <string>

// O que o menu pede ao jogo depois de tratar um evento.
enum class AcaoMenu
{
    NENHUMA,
    CONTINUAR,
    NOVO_JOGO,
    REINICIAR,
    MENU_INICIAL,
    SAIR
};

class Menu
{
public:
    static constexpr int NUM_OPCOES_INICIAIS = 3;
    static constexpr int NUM_OPCOES_PAUSA = 4;

private:
    int selecionado;
    bool inicial = false;
    bool podeContinuar = false;
    std::string detalhe;

public:
    Menu();
    ~Menu();

    void reiniciar();
    void configurar_inicial(bool podeContinuar, const std::string &detalhe);
    void configurar_pausa(const std::string &detalhe = "");

    int get_selecionado() const;
    void mover_selecao(int);

    AcaoMenu tratar_evento(const SDL_Event &);
    void desenhar(SDL_Renderer *) const;

private:
    int get_quantidadeOpcoes() const;
    AcaoMenu acao_da_opcao(int) const;
    SDL_Rect retangulo_painel() const;
    SDL_Rect retangulo_opcao(int) const;
};

#endif
