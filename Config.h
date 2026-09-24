#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>

// Resolucao logica do jogo. A janela pode ser redimensionada; o SDL escala
// o conteudo para caber, entao todo o codigo trabalha sempre nesta medida.
constexpr int LARGURA_TELA = 960;
constexpr int ALTURA_TELA  = 540;

constexpr char TITULO_JANELA[] = "Projeto AED - Roguelike";

// Jogador
constexpr float LADO_JOGADOR       = 32.0f;
constexpr float VELOCIDADE_JOGADOR = 260.0f;   // pixels por segundo

// Arena
constexpr int MARGEM_ARENA = 24;
constexpr int PASSO_GRADE  = 48;

// Paleta
constexpr SDL_Color COR_FUNDO          = {  36,  36,  40, 255 };
constexpr SDL_Color COR_ARENA          = { 104, 104, 110, 255 };
constexpr SDL_Color COR_GRADE          = {  92,  92,  98, 255 };
constexpr SDL_Color COR_BORDA_ARENA    = {  64,  64,  70, 255 };
constexpr SDL_Color COR_JOGADOR        = { 214,  62,  62, 255 };
constexpr SDL_Color COR_JOGADOR_BORDA  = { 132,  28,  28, 255 };

// Menu de pausa
constexpr SDL_Color COR_VEU            = {   0,   0,   0, 165 };
constexpr SDL_Color COR_PAINEL         = {  28,  28,  32, 240 };
constexpr SDL_Color COR_PAINEL_BORDA   = { 214,  62,  62, 255 };
constexpr SDL_Color COR_TEXTO          = { 232, 232, 236, 255 };
constexpr SDL_Color COR_TEXTO_APAGADO  = { 140, 140, 148, 255 };
constexpr SDL_Color COR_OPCAO          = { 176, 176, 184, 255 };
constexpr SDL_Color COR_OPCAO_ATIVA    = { 255, 255, 255, 255 };
constexpr SDL_Color COR_REALCE         = {  58,  58,  66, 255 };

#endif
