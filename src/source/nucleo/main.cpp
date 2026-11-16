#include "nucleo/Jogo.h"
#include "nucleo/Plataforma.h"

#ifdef __EMSCRIPTEN__
#include <emscripten.h>

namespace
{
bool jogoIniciado = false;

// O navegador nao aceita um laco infinito: ele chama esta funcao a cada quadro.
void quadro_web(void *dados)
{
    Jogo &jogo = *static_cast<Jogo *>(dados);
    if (!jogoIniciado)
    {
        // O menu inicial le os slots, entao espera o IndexedDB ser carregado.
        if (!Plataforma::saves_prontos())
        {
            return;
        }
        if (!jogo.inicializar())
        {
            emscripten_cancel_main_loop();
            return;
        }
        jogoIniciado = true;
    }
    jogo.passo();
}
} // namespace
#endif

int main(int /*argc*/, char * /*argv*/[])
{
    Jogo jogo;

#ifdef __EMSCRIPTEN__
    Plataforma::preparar_saves();
    // 0 quadros/s = segue o requestAnimationFrame do navegador. O "true" nunca
    // retorna, entao o objeto jogo continua vivo enquanto a pagina estiver aberta.
    emscripten_set_main_loop_arg(quadro_web, &jogo, 0, true);
#else
    if (!jogo.inicializar())
    {
        return 1;
    }

    jogo.executar();
#endif
    return 0;
}
