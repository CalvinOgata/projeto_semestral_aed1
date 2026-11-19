#ifndef REPERTORIO_H
#define REPERTORIO_H

#include <vector>

#include "audio/Sintetizador.h"

// Catalogo sonoro do jogo: cada efeito e cada trilha e composto aqui como uma
// lista de notas. Nao depende de SDL nem das entidades do jogo.
enum class EfeitoSonoro
{
    MENU_MOVER,
    MENU_CONFIRMAR,
    MENU_VOLTAR,
    MENU_AJUSTE,
    CARREGAMENTO_LINHA,
    CARREGAMENTO_PRONTO,
    TIRO_SOCO,
    TIRO_PISTOLA,
    TIRO_SHOTGUN,
    TIRO_LASER,
    TIRO_DUPLA,
    TIRO_RICOCHETE,
    TIRO_ESPADA,
    TIRO_ARCO,
    TIRO_BOOMERANG,
    TIRO_PULSO,
    SEM_MUNICAO,
    ACERTO_INIMIGO,
    OBSTACULO_QUEBRA,
    MORTE_VIRUS,
    MORTE_BRUTO,
    MORTE_CHEFE,
    TIRO_INIMIGO,
    TIRO_CHEFE,
    INVOCACAO,
    ONDA_ALERTA,
    DANO_ESCUDO,
    DANO_VIDA,
    CORACAO,
    PORTAS_TRANCAM,
    SALA_LIBERADA,
    BAU_ABRE,
    ARMA_COLETA,
    ARMA_SOLTA,
    ARMA_TROCA,
    PORTAL_SURGE,
    FASE_CONCLUIDA,
    MORTE_JOGADOR,
    GAME_OVER,
    CONTINUE,
    VITORIA,
    CHEAT,
    TOTAL // quantidade de efeitos; nao e um som
};

enum class Musica
{
    NENHUMA,
    MENU,
    FASE,
    CHEFE,
    TOTAL
};

namespace Repertorio
{
std::vector<Sintetizador::Nota> compor_efeito(EfeitoSonoro efeito);

// Trilhas sao compostas com duracao exata de compassos e tocam em laco.
struct Partitura
{
    std::vector<Sintetizador::Nota> notas;
    float duracao = 0.0f; // segundos de um ciclo completo
};
Partitura compor_musica(Musica musica);

// Intervalo minimo entre dois disparos do mesmo efeito, para tiros rapidos e
// muitos acertos simultaneos nao virarem um ruido unico.
float intervalo_minimo(EfeitoSonoro efeito);
} // namespace Repertorio

#endif
