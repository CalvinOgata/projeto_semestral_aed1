#ifndef CAMPANHA_CONFIG_H
#define CAMPANHA_CONFIG_H

// Compartilhado pela progressao e pelo save, sem dependencia de SDL.
constexpr int QUANTIDADE_FASES = 3;
constexpr int QUANTIDADE_SLOTS_SAVE = 3;
constexpr int VIDAS_INICIAIS = 3;
// Limites do inventario salvo; conferidos contra TipoArma e FilaArmas em Campanha.
constexpr int QUANTIDADE_TIPOS_ARMA = 8;
constexpr int CAPACIDADE_INVENTARIO = 3;

// Pontuacao. O bonus de rapidez usa BASE * TEMPO_REFERENCIA / tempo_real.
constexpr int PONTOS_VIRUS = 100;
constexpr int PONTOS_BRUTO = 250;
constexpr int PONTOS_BOSS = 2000;
constexpr int PONTOS_RAPIDEZ_BASE = 1000;
constexpr float TEMPO_REFERENCIA_FASE = 300.0f;

#endif
