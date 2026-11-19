#ifndef SOM_H
#define SOM_H

#include "audio/Repertorio.h"

// Mixer do jogo sobre o dispositivo de audio do SDL. Efeitos e trilhas sao
// sintetizados durante a tela de carregamento e ficam prontos na memoria.
// Sem placa de som o jogo continua normalmente, apenas mudo.
namespace Som
{
bool inicializar();
void finalizar();

// Sintetiza o proximo pedaco do repertorio, gastando no maximo o orcamento
// em milissegundos. Devolve o progresso total (0 a 1).
float carregar_etapa(int orcamentoMs);
bool carregado();

void tocar(EfeitoSonoro efeito, float volume = 1.0f);
// Troca a trilha com transicao suave; a mesma trilha nao recomeca.
void tocar_musica(Musica musica);
// Volumes de 0 a 1 vindos das configuracoes.
void set_volumes(float musica, float efeitos);
// Abaixa a trilha (pausa, minimapa) sem interrompe-la.
void set_abafado(bool abafado);
} // namespace Som

#endif
