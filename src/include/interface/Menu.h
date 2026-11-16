#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

#include <array>
#include <string>

#include "config/CampanhaConfig.h"
#include "nucleo/Plataforma.h"

// O que o menu pede ao jogo depois de tratar um evento.
enum class AcaoMenu
{
    NENHUMA,
    CONTINUAR,
    NOVO_JOGO,
    REINICIAR,
    MENU_INICIAL,
    SAIR,
    SLOT_1,
    SLOT_2,
    SLOT_3,
    VOLTAR
};

enum class TelaMenu
{
    INICIAL,
    PAUSA,
    SLOTS_NOVO,
    SLOTS_CONTINUAR
};

class Menu
{
public:
    // "SAIR" e sempre a ultima opcao e nao aparece na versao web.
    static constexpr int NUM_OPCOES_INICIAIS = Plataforma::PERMITE_SAIR ? 3 : 2;
    static constexpr int NUM_OPCOES_PAUSA = Plataforma::PERMITE_SAIR ? 4 : 3;
    static constexpr int NUM_OPCOES_SLOTS = QUANTIDADE_SLOTS_SAVE + 1;

private:
    int selecionado = 0;
    TelaMenu tela = TelaMenu::INICIAL;
    bool podeContinuar = false;
    std::string detalhe;
    std::array<std::string, QUANTIDADE_SLOTS_SAVE> detalhesSlots;
    std::array<bool, QUANTIDADE_SLOTS_SAVE> slotsDisponiveis = {};

public:
    Menu();
    ~Menu();

    void reiniciar();
    void configurar_inicial(bool podeContinuar, const std::string &detalhe);
    void configurar_pausa(const std::string &detalhe = "");
    void configurar_slots(
        bool novoJogo, const std::array<std::string, QUANTIDADE_SLOTS_SAVE> &detalhes,
        const std::array<bool, QUANTIDADE_SLOTS_SAVE> &disponiveis);

    int get_selecionado() const;
    void mover_selecao(int passo);

    AcaoMenu tratar_evento(const SDL_Event &evento);
    void desenhar(SDL_Renderer *renderizador) const;

private:
    bool tela_inicial() const;
    bool tela_slots() const;
    int get_quantidadeOpcoes() const;
    AcaoMenu acao_da_opcao(int indice) const;
    SDL_Rect retangulo_painel() const;
    SDL_Rect retangulo_opcao(int indice) const;
};

#endif
