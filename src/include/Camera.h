#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>

// Converte entre coordenadas de mundo (ilimitadas) e de tela.
// O mundo e aberto: a camera e que anda junto com o jogador.
class Camera{
private:
    float x;   // canto superior esquerdo da vista, em coordenadas de mundo
    float y;

public:
    Camera();

    void centralizar_em(float mundoX, float mundoY);

    float get_x() const;
    float get_y() const;

    SDL_FPoint para_tela(float mundoX, float mundoY) const;
    SDL_FRect  para_tela(const SDL_FRect &mundo) const;
    SDL_FPoint para_mundo(float telaX, float telaY) const;
};

#endif
