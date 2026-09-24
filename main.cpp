#include "Jogo.h"

int main(int /*argc*/, char* /*argv*/[])
{
    Jogo jogo;

    if(!jogo.inicializar())
    {
        return 1;
    }

    jogo.executar();
    return 0;
}
