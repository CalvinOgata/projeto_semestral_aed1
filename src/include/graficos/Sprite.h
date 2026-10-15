#ifndef SPRITE_H
#define SPRITE_H

#include <SDL2/SDL.h>
#include <string>

// Recurso grafico pertencente ao jogo, nao ao jogador copiavel/reiniciavel.
class Sprite
{
private:
    SDL_Texture *textura = nullptr;
    int largura = 0;
    int altura = 0;
    std::string erro;

public:
    Sprite() = default;
    ~Sprite();
    Sprite(const Sprite &) = delete;
    Sprite &operator=(const Sprite &) = delete;

    bool carregar(SDL_Renderer *renderizador, const std::string &caminho);
    void liberar(); // chamar antes de destruir o renderizador
    const std::string &get_erro() const;
    bool desenhar(SDL_Renderer *renderizador, const SDL_FRect &quadro) const;
};

#endif
