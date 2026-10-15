#ifndef CAMERA_H
#define CAMERA_H

#include <SDL2/SDL.h>

// Converte entre coordenadas de mundo e de tela.
class Camera
{
private:
    float x; // canto superior esquerdo da vista, em coordenadas de mundo
    float y;

public:
    Camera();

    void centralizar_em(float mundoX, float mundoY);

    void limitar_a(const SDL_FRect &area);

    float get_x() const;
    float get_y() const;

    SDL_FPoint para_tela(float mundoX, float mundoY) const;
    SDL_FRect para_tela(const SDL_FRect &mundo) const;
    SDL_FPoint para_mundo(float telaX, float telaY) const;
};

#endif
