#include "nucleo/Plataforma.h"

#ifdef __EMSCRIPTEN__

#include <emscripten.h>

namespace
{
bool savesCarregados = false;
}

// Chamada pelo JavaScript quando o IndexedDB terminou de ser lido.
extern "C" EMSCRIPTEN_KEEPALIVE void plataforma_saves_carregados()
{
    savesCarregados = true;
}

namespace Plataforma
{
void preparar_saves()
{
    // SDL_GetBasePath devolve "/" no navegador, entao os saves ficam em /saves.
    // O sistema de arquivos do Emscripten vive na memoria; o IDBFS espelha a
    // pasta no IndexedDB para os saves sobreviverem a recarga da pagina.
    // O bloco abaixo e JavaScript, entao fica fora do clang-format.
    // clang-format off
    EM_ASM({
        try
        {
            FS.mkdir('/saves');
        }
        catch (e)
        {
        }
        FS.mount(IDBFS, {}, '/saves');
        FS.syncfs(true, function(erro) {
            if (erro)
            {
                console.error('Falha ao carregar os saves:', erro);
            }
            _plataforma_saves_carregados();
        });
    });
    // clang-format on
}

bool saves_prontos()
{
    return savesCarregados;
}

void persistir_saves()
{
    // clang-format off
    EM_ASM({
        FS.syncfs(false, function(erro) {
            if (erro)
            {
                console.error('Falha ao gravar os saves:', erro);
            }
        });
    });
    // clang-format on
}
} // namespace Plataforma

#else

namespace Plataforma
{
void preparar_saves()
{
}

bool saves_prontos()
{
    return true;
}

void persistir_saves()
{
}
} // namespace Plataforma

#endif
