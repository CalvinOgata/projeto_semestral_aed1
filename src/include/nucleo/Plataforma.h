#ifndef PLATAFORMA_H
#define PLATAFORMA_H

// Diferencas entre a versao desktop e a versao web (Emscripten).
// No desktop todas as funcoes sao triviais; o resto do jogo nao usa #ifdef.
namespace Plataforma
{
// A versao web nao tem como fechar a aba, entao o menu esconde "SAIR".
// Tela cheia depende de um gesto do usuario no navegador; la a pagina cuida disso.
// O navegador precisa de um buffer de audio maior para nao picotar.
#ifdef __EMSCRIPTEN__
constexpr bool PERMITE_SAIR = false;
constexpr bool PERMITE_TELA_CHEIA = false;
constexpr int AMOSTRAS_AUDIO = 2048;
#else
constexpr bool PERMITE_SAIR = true;
constexpr bool PERMITE_TELA_CHEIA = true;
constexpr int AMOSTRAS_AUDIO = 1024;
#endif

// Web: monta saves/ sobre o IndexedDB do navegador e comeca a carregar os
// arquivos gravados em sessoes anteriores. Desktop: nao faz nada.
void preparar_saves();

// Web: verdadeiro quando o carregamento iniciado acima terminou.
// Desktop: sempre verdadeiro.
bool saves_prontos();

// Web: copia os arquivos de saves/ para o IndexedDB depois de gravar ou apagar.
// Desktop: nao faz nada, pois o arquivo ja esta no disco.
void persistir_saves();
} // namespace Plataforma

#endif
