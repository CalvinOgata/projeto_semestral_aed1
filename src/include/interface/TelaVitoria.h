#ifndef TELA_VITORIA_H
#define TELA_VITORIA_H

#include <SDL2/SDL.h>

#include <string>
#include <vector>

class Sprite;

// Encerramento da campanha em tres etapas: celebracao com fogos, um texto
// reflexivo que sobe devagar (com trocadilhos de programacao) e os creditos.
class TelaVitoria
{
private:
    enum class Etapa
    {
        CELEBRACAO,
        MENSAGEM,
        CREDITOS
    };

    // Elemento do texto que sobe: paragrafo de uma das duas "vozes", destaque,
    // logotipo ou espaco vazio. As linhas ja vem quebradas na largura certa.
    struct Bloco
    {
        enum class Tipo
        {
            TEXTO,
            LOGO,
            ESPACO
        } tipo = Tipo::ESPACO;
        std::string texto;
        int escala = 2;
        SDL_Color cor = {255, 255, 255, 255};
        int altura = 0;
    };

    Etapa etapa = Etapa::CELEBRACAO;
    float tempo = 0.0f;      // tempo total, anima o fundo
    float tempoEtapa = 0.0f; // tempo dentro da etapa atual
    float rolagem = 0.0f;    // pixels que o texto ja subiu
    float rolagemFinal = 0.0f;
    int pontuacao = 0;
    int fogosDisparados = 0;
    bool encerrada = false;
    bool modoToque = false; // avisos de toque em vez de teclas
    std::vector<Bloco> blocos;

public:
    void iniciar(int pontuacao);
    // acelerar: espaco ou botao do mouse segurado durante o texto.
    // navegar: -1 (seta para cima, volta o texto), 0 ou +1 (seta para baixo, adianta).
    void atualizar(float dt, bool acelerar, int navegar);
    void avancar(); // Enter: sai da celebracao ou, no fim, volta ao menu
    void pular();   // Esc: vai direto aos creditos
    bool terminou() const;
    void set_modoToque(bool modoToque);
    void desenhar(SDL_Renderer *renderizador, const Sprite &tux) const;

private:
    void montar_texto();
    void desenhar_celebracao(SDL_Renderer *renderizador, const Sprite &tux) const;
    void desenhar_mensagem(SDL_Renderer *renderizador) const;
};

#endif
