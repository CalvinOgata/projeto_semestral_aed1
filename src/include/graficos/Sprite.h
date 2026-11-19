#ifndef SPRITE_H
#define SPRITE_H

#include "graficos/AssetsEmbutidos.h"

#include <SDL2/SDL.h>
#include <string>

// Recurso grafico pertencente ao jogo, nao ao jogador copiavel/reiniciavel.
class Sprite
{
private:
    SDL_Texture *textura = nullptr;
    int largura = 0;
    int altura = 0;
    int quadros = 1;
    std::string erro;

public:
    Sprite() = default;
    ~Sprite();
    Sprite(const Sprite &) = delete;
    Sprite &operator=(const Sprite &) = delete;

    // Decodifica um PNG embutido no executavel (ver AssetsEmbutidos.h).
    bool carregar(SDL_Renderer *renderizador, const AssetEmbutido &png, int quadros = 1);
    void liberar(); // chamar antes de destruir o renderizador
    const std::string &get_erro() const;
    bool desenhar(SDL_Renderer *renderizador, const SDL_FRect &quadro) const;
    bool desenhar_quadro(SDL_Renderer *renderizador, const SDL_FRect &quadro, int indice,
                         double angulo = 0, SDL_RendererFlip flip = SDL_FLIP_NONE) const;
    // Recorta uma regiao normalizada da mesma textura e a encaixa no quadro.
    // Assim o HUD pode usar literalmente a cabeca do sprite do jogador.
    bool desenhar_recorte(SDL_Renderer *renderizador, const SDL_FRect &quadro,
                         const SDL_FRect &recorteNormalizado) const;
};

#endif
