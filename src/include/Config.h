#ifndef CONFIG_H
#define CONFIG_H

#include <SDL2/SDL.h>

// Resolucao logica do jogo. A janela pode ser redimensionada; o SDL escala
// o conteudo para caber, entao todo o codigo trabalha sempre nesta medida.
constexpr int LARGURA_TELA = 960;
constexpr int ALTURA_TELA  = 540;

constexpr char TITULO_JANELA[] = "Projeto AED - Roguelike";

// Mundo aberto: nao existe limite de arena, so a grade de referencia.
constexpr int PASSO_GRADE = 48;

// Jogador
constexpr float LADO_JOGADOR       = 32.0f;
constexpr float VELOCIDADE_JOGADOR = 260.0f;   // pixels por segundo

// Arma (o cano que gira acompanhando o mouse)
constexpr float COMPRIMENTO_ARMA = 34.0f;
constexpr float ESPESSURA_ARMA   = 10.0f;
constexpr float RECUO_ARMA       = 2.0f;    // quanto o pivo fica atras do centro
constexpr float INTERVALO_TIRO   = 0.14f;   // segundos entre disparos
constexpr int   MUNICAO_INICIAL  = 400;

// Projetil
constexpr float LADO_PROJETIL       = 9.0f;
constexpr float VELOCIDADE_PROJETIL = 680.0f;
constexpr float ALCANCE_PROJETIL    = 900.0f;  // some depois de percorrer isso

// Inimigos
constexpr float LADO_INIMIGO       = 28.0f;
constexpr float VELOCIDADE_INIMIGO = 115.0f;
constexpr int   VIDA_INIMIGO       = 2;
constexpr int   QUANTIDADE_INIMIGOS = 5;
constexpr float RAIO_SURGIMENTO    = 330.0f;   // distancia do jogador ao nascer

// Paleta
constexpr SDL_Color COR_MUNDO          = { 104, 104, 110, 255 };
constexpr SDL_Color COR_GRADE          = {  92,  92,  98, 255 };
constexpr SDL_Color COR_JOGADOR        = { 214,  62,  62, 255 };
constexpr SDL_Color COR_JOGADOR_BORDA  = { 132,  28,  28, 255 };
constexpr SDL_Color COR_ARMA           = {  54,  54,  60, 255 };
constexpr SDL_Color COR_PROJETIL       = { 248, 216,  96, 255 };
constexpr SDL_Color COR_INIMIGO        = {  62, 118, 214, 255 };
constexpr SDL_Color COR_INIMIGO_BORDA  = {  28,  58, 132, 255 };

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
