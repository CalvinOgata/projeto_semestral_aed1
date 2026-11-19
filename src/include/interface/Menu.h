#ifndef MENU_H
#define MENU_H

#include <SDL2/SDL.h>

#include <array>
#include <string>

#include "config/CampanhaConfig.h"
#include "nucleo/Plataforma.h"
#include "persistencia/ArquivoConfiguracoes.h"

class Sprite;

// O que o menu pede ao jogo depois de tratar um evento. A navegacao entre as
// telas informativas (manual, configuracoes, sobre) e resolvida pelo proprio menu.
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
    VOLTAR,
    CONFIGURACOES_ALTERADAS
};

enum class TelaMenu
{
    PRINCIPAL,
    JOGAR,
    SLOTS_NOVO,
    SLOTS_CONTINUAR,
    MANUAL,
    CONFIGURACOES,
    SOBRE,
    PAUSA
};

class Menu
{
public:
    // "SAIR" e sempre a ultima opcao e nao aparece na versao web.
    static constexpr int NUM_OPCOES_PRINCIPAIS = Plataforma::PERMITE_SAIR ? 5 : 4;
    static constexpr int NUM_OPCOES_JOGAR = 3;
    static constexpr int NUM_OPCOES_PAUSA = Plataforma::PERMITE_SAIR ? 5 : 4;
    static constexpr int NUM_OPCOES_SLOTS = QUANTIDADE_SLOTS_SAVE + 1;

private:
    // Linhas da tela de configuracoes; tela cheia some onde nao e suportada.
    enum class ItemConfig
    {
        VOLUME_MUSICA,
        VOLUME_EFEITOS,
        TELA_CHEIA,
        HITBOXES,
        VOLTAR
    };

    int selecionado = 0;
    TelaMenu tela = TelaMenu::PRINCIPAL;
    bool podeContinuar = false;
    std::string detalhe;
    std::array<std::string, QUANTIDADE_SLOTS_SAVE> detalhesSlots;
    std::array<bool, QUANTIDADE_SLOTS_SAVE> slotsDisponiveis = {};
    Configuracoes configuracoes;
    int abaManual = 0;
    bool manualDaPausa = false; // o manual aberto na pausa volta para ela
    float tempo = 0.0f;         // anima selecao, fundo e personagens

public:
    Menu();
    ~Menu();

    void reiniciar();
    void atualizar(float dt);
    // telaJogar volta direto para "novo jogo / continuar" (ao sair dos slots).
    void configurar_inicial(bool podeContinuar, const std::string &detalhe, bool telaJogar = false);
    void configurar_pausa(const std::string &detalhe = "");
    void configurar_slots(bool novoJogo,
                          const std::array<std::string, QUANTIDADE_SLOTS_SAVE> &detalhes,
                          const std::array<bool, QUANTIDADE_SLOTS_SAVE> &disponiveis);
    void set_configuracoes(const Configuracoes &configuracoes);
    const Configuracoes &get_configuracoes() const;

    TelaMenu get_tela() const;
    int get_selecionado() const;
    void mover_selecao(int passo);

    AcaoMenu tratar_evento(const SDL_Event &evento);
    void desenhar(SDL_Renderer *renderizador, const Sprite &tux) const;

private:
    bool tela_slots() const;
    bool tela_painelInteiro() const; // manual e sobre ocupam quase a tela toda
    int get_quantidadeOpcoes() const;
    bool opcao_desabilitada(int indice) const;
    ItemConfig item_config(int indice) const;
    void abrir_tela(TelaMenu nova, int selecao);
    AcaoMenu ativar_opcao(int indice);
    AcaoMenu ajustar_config(int indice, int passo);
    void trocar_aba(int passo);
    SDL_Rect retangulo_painel() const;
    SDL_Rect retangulo_opcao(int indice) const;
    SDL_Rect retangulo_aba(int indice) const;

    void desenhar_botao(SDL_Renderer *renderizador, const SDL_Rect &area, const std::string &texto,
                        bool ativa, bool desabilitada, int escala) const;
    void desenhar_principal(SDL_Renderer *renderizador, const Sprite &tux) const;
    void desenhar_manual(SDL_Renderer *renderizador, const SDL_Rect &painel) const;
    void desenhar_configuracoes(SDL_Renderer *renderizador, const SDL_Rect &painel) const;
    void desenhar_sobre(SDL_Renderer *renderizador, const SDL_Rect &painel) const;
};

#endif
