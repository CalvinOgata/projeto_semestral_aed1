#include "Jogo.h"

#include <cstdio>

#include "Config.h"
#include "Texto.h"

namespace
{
    // Trava o passo de tempo: sem isso, arrastar a janela acumula um dt enorme
    // e o jogador atravessaria a arena de uma vez.
    constexpr float DT_MAXIMO = 0.05f;
}

Jogo::Jogo()
    : janela(nullptr), renderizador(nullptr),
      estado(EstadoJogo::JOGANDO), tempoAnterior(0)
{
    this->arena.x = MARGEM_ARENA;
    this->arena.y = MARGEM_ARENA;
    this->arena.w = LARGURA_TELA - 2 * MARGEM_ARENA;
    this->arena.h = ALTURA_TELA  - 2 * MARGEM_ARENA;
}

Jogo::~Jogo()
{
    this->finalizar();
}

bool Jogo::inicializar()
{
    if(SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "Erro ao iniciar o SDL: %s\n", SDL_GetError());
        return false;
    }

    this->janela = SDL_CreateWindow(TITULO_JANELA,
                                    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                    LARGURA_TELA, ALTURA_TELA,
                                    SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if(this->janela == nullptr)
    {
        std::fprintf(stderr, "Erro ao criar a janela: %s\n", SDL_GetError());
        return false;
    }

    this->renderizador = SDL_CreateRenderer(this->janela, -1,
                                            SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if(this->renderizador == nullptr)
    {
        // Sem aceleracao disponivel ainda da para rodar por software.
        this->renderizador = SDL_CreateRenderer(this->janela, -1, SDL_RENDERER_SOFTWARE);
    }
    if(this->renderizador == nullptr)
    {
        std::fprintf(stderr, "Erro ao criar o renderizador: %s\n", SDL_GetError());
        return false;
    }

    // Todo o jogo trabalha em 960x540; o SDL escala para o tamanho real da janela.
    SDL_RenderSetLogicalSize(this->renderizador, LARGURA_TELA, ALTURA_TELA);
    SDL_SetRenderDrawBlendMode(this->renderizador, SDL_BLENDMODE_BLEND);

    // Jogador no centro da arena.
    this->jogador.set_posicao(this->arena.x + (this->arena.w - this->jogador.get_largura()) / 2.0f,
                              this->arena.y + (this->arena.h - this->jogador.get_altura()) / 2.0f);

    this->estado = EstadoJogo::JOGANDO;
    this->tempoAnterior = SDL_GetPerformanceCounter();
    return true;
}

EstadoJogo Jogo::get_estado() const
{
    return this->estado;
}

void Jogo::executar()
{
    while(this->estado != EstadoJogo::SAINDO)
    {
        const Uint64 agora = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(agora - this->tempoAnterior) /
                   static_cast<float>(SDL_GetPerformanceFrequency());
        this->tempoAnterior = agora;

        if(dt > DT_MAXIMO)
        {
            dt = DT_MAXIMO;
        }

        this->tratar_eventos();

        if(this->estado == EstadoJogo::JOGANDO)
        {
            this->atualizar(dt);
        }

        this->desenhar();
    }
}

void Jogo::tratar_eventos()
{
    SDL_Event evento;

    // A fila e sempre esvaziada ate o fim. Sair do laco no meio deixa para tras
    // a sentinela interna que o SDL usa em SDL_PollEvent, e a leitura seguinte
    // devolve "fila vazia" por engano, perdendo os eventos daquele quadro.
    while(SDL_PollEvent(&evento))
    {
        if(evento.type == SDL_QUIT)
        {
            this->estado = EstadoJogo::SAINDO;
        }

        if(this->estado == EstadoJogo::SAINDO)
        {
            continue;   // ja esta encerrando: so consome o que sobrou
        }

        if(this->estado == EstadoJogo::PAUSADO)
        {
            const AcaoMenu acao = this->menu.tratar_evento(evento);

            if(acao == AcaoMenu::CONTINUAR)
            {
                this->retomar();
            }
            else if(acao == AcaoMenu::SAIR)
            {
                this->estado = EstadoJogo::SAINDO;
            }
        }
        else if(evento.type == SDL_KEYDOWN && evento.key.repeat == 0)
        {
            if(evento.key.keysym.sym == SDLK_ESCAPE)
            {
                this->pausar();
            }
        }
    }
}

void Jogo::atualizar(float dt)
{
    const Uint8 *teclas = SDL_GetKeyboardState(nullptr);

    int dirX = 0;
    int dirY = 0;

    if(teclas[SDL_SCANCODE_A] || teclas[SDL_SCANCODE_LEFT])  dirX -= 1;
    if(teclas[SDL_SCANCODE_D] || teclas[SDL_SCANCODE_RIGHT]) dirX += 1;
    if(teclas[SDL_SCANCODE_W] || teclas[SDL_SCANCODE_UP])    dirY -= 1;
    if(teclas[SDL_SCANCODE_S] || teclas[SDL_SCANCODE_DOWN])  dirY += 1;

    this->jogador.mover(dirX, dirY, dt);
    this->jogador.manter_nos_limites(this->arena);
}

void Jogo::desenhar()
{
    SDL_SetRenderDrawColor(this->renderizador, COR_FUNDO.r, COR_FUNDO.g, COR_FUNDO.b, COR_FUNDO.a);
    SDL_RenderClear(this->renderizador);

    this->desenhar_arena();
    this->jogador.desenhar(this->renderizador);
    this->desenhar_hud();

    if(this->estado == EstadoJogo::PAUSADO)
    {
        this->menu.desenhar(this->renderizador);
    }

    SDL_RenderPresent(this->renderizador);
}

void Jogo::desenhar_arena()
{
    SDL_SetRenderDrawColor(this->renderizador, COR_ARENA.r, COR_ARENA.g, COR_ARENA.b, COR_ARENA.a);
    SDL_RenderFillRect(this->renderizador, &this->arena);

    // Grade discreta: da referencia de movimento na visao de cima.
    SDL_SetRenderDrawColor(this->renderizador, COR_GRADE.r, COR_GRADE.g, COR_GRADE.b, COR_GRADE.a);

    for(int x = this->arena.x + PASSO_GRADE; x < this->arena.x + this->arena.w; x += PASSO_GRADE)
    {
        SDL_RenderDrawLine(this->renderizador, x, this->arena.y, x, this->arena.y + this->arena.h - 1);
    }
    for(int y = this->arena.y + PASSO_GRADE; y < this->arena.y + this->arena.h; y += PASSO_GRADE)
    {
        SDL_RenderDrawLine(this->renderizador, this->arena.x, y, this->arena.x + this->arena.w - 1, y);
    }

    SDL_SetRenderDrawColor(this->renderizador, COR_BORDA_ARENA.r, COR_BORDA_ARENA.g, COR_BORDA_ARENA.b, COR_BORDA_ARENA.a);
    SDL_RenderDrawRect(this->renderizador, &this->arena);
}

void Jogo::desenhar_hud()
{
    Texto::desenhar(this->renderizador, "WASD OU SETAS MOVER - ESC MENU",
                    this->arena.x + 8, this->arena.y - 18, 2, COR_TEXTO_APAGADO);
}

void Jogo::pausar()
{
    this->menu.reiniciar();
    this->estado = EstadoJogo::PAUSADO;
}

void Jogo::retomar()
{
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::finalizar()
{
    if(this->renderizador != nullptr)
    {
        SDL_DestroyRenderer(this->renderizador);
        this->renderizador = nullptr;
    }
    if(this->janela != nullptr)
    {
        SDL_DestroyWindow(this->janela);
        this->janela = nullptr;
    }
    SDL_Quit();
}
