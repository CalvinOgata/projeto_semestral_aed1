#ifndef CONTROLES_TOQUE_H
#define CONTROLES_TOQUE_H

#include <SDL2/SDL.h>

// Pedidos dos botoes de toque que o jogo precisa atender na hora.
enum class AcaoToque
{
    NENHUMA,
    PAUSAR,
    MINIMAPA,
    PEGAR_ARMA,
    TROCAR_ARMA,
    SOLTAR_ARMA
};

// Controles para tela sensivel ao toque (celular e tablet no navegador).
// A metade esquerda da tela vira um joystick que nasce onde o dedo encosta;
// a direita tem o botao de atirar e os botoes de arma. Cada dedo e seguido
// pelo seu identificador, entao da para andar e atirar ao mesmo tempo.
class ControlesToque
{
private:
    static constexpr int MAX_DEDOS = 10;

    enum class Funcao
    {
        LIVRE,
        JOYSTICK,
        ATIRAR,
        BOTAO // botoes de toque unico: agem ao encostar e o dedo fica ocupado
    };

    struct Dedo
    {
        SDL_FingerID id = 0;
        Funcao funcao = Funcao::LIVRE;
    };

    Dedo dedos[MAX_DEDOS];
    float baseX; // centro do joystick, em coordenadas logicas
    float baseY;
    float alavancaX; // deslocamento atual do dedo em relacao a base
    float alavancaY;
    float ultimaDirecaoX = 1.0f; // direcao lembrada para mirar sem inimigos
    float ultimaDirecaoY = 0.0f;
    bool joystickAtivo = false;
    bool atirando = false;

public:
    ControlesToque();

    // Recebe apenas eventos SDL_FINGER* durante o jogo.
    AcaoToque tratar_evento(const SDL_Event &evento);
    // Esquece todos os dedos (pausa, mapa, troca de estado).
    void soltar_tudo();

    // Vetor de movimento com comprimento entre 0 e 1.
    void get_direcao(float &x, float &y) const;
    void get_ultimaDirecao(float &x, float &y) const;
    bool get_atirando() const;

    // armaPerto acende o botao PEGAR quando ha arma ao alcance.
    void desenhar(SDL_Renderer *renderizador, bool armaPerto) const;

private:
    Dedo *buscar_dedo(SDL_FingerID id);
    Dedo *dedo_livre();
    void mover_alavanca(float x, float y);
};

#endif
