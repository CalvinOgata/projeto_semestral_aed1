#include "nucleo/Jogo.h"

#include <cstdio>

#include "config/Config.h"
#include "graficos/Desenho.h"

namespace
{
// Trava o passo de tempo: sem isso, arrastar a janela acumula um dt enorme
// e o jogador atravessaria meio mapa de uma vez.
constexpr float DT_MAXIMO = 0.05f;
} // namespace

Jogo::Jogo()
    : janela(nullptr), renderizador(nullptr), estado(EstadoJogo::JOGANDO), tempoAnterior(0),
      esperaTiro(0.0f), atirando(false), mouseTelaX(LARGURA_TELA / 2.0f + 120.0f),
      mouseTelaY(ALTURA_TELA / 2.0f)
{
}

Jogo::~Jogo()
{
    this->finalizar();
}

bool Jogo::inicializar()
{
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "Erro ao iniciar o SDL: %s\n", SDL_GetError());
        return false;
    }

    this->janela =
        SDL_CreateWindow(TITULO_JANELA, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                         LARGURA_TELA, ALTURA_TELA, SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE);
    if (this->janela == nullptr)
    {
        std::fprintf(stderr, "Erro ao criar a janela: %s\n", SDL_GetError());
        return false;
    }

    this->renderizador =
        SDL_CreateRenderer(this->janela, -1, SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC);
    if (this->renderizador == nullptr)
    {
        // Sem aceleracao disponivel ainda da para rodar por software.
        this->renderizador = SDL_CreateRenderer(this->janela, -1, SDL_RENDERER_SOFTWARE);
    }
    if (this->renderizador == nullptr)
    {
        std::fprintf(stderr, "Erro ao criar o renderizador: %s\n", SDL_GetError());
        return false;
    }

    // Todo o jogo trabalha em 960x540; o SDL escala para o tamanho real da janela.
    SDL_RenderSetLogicalSize(this->renderizador, LARGURA_TELA, ALTURA_TELA);
    SDL_SetRenderDrawBlendMode(this->renderizador, SDL_BLENDMODE_BLEND);

    if (!Desenho::inicializar(this->renderizador))
    {
        std::fprintf(stderr, "Erro ao preparar o desenho: %s\n", SDL_GetError());
        return false;
    }

    this->fase.reiniciar(this->jogador);
    this->atualizar_camera();
    this->atualizar_mira();

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
    while (this->estado != EstadoJogo::SAINDO)
    {
        const Uint64 agora = SDL_GetPerformanceCounter();
        float dt = static_cast<float>(agora - this->tempoAnterior) /
                   static_cast<float>(SDL_GetPerformanceFrequency());
        this->tempoAnterior = agora;

        if (dt > DT_MAXIMO)
        {
            dt = DT_MAXIMO;
        }

        this->tratar_eventos();

        if (this->estado == EstadoJogo::JOGANDO)
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
    while (SDL_PollEvent(&evento))
    {
        if (evento.type == SDL_QUIT)
        {
            this->estado = EstadoJogo::SAINDO;
        }

        if (this->estado == EstadoJogo::SAINDO)
        {
            continue; // ja esta encerrando: so consome o que sobrou
        }

        // A mira acompanha o mouse mesmo com o jogo pausado.
        if (evento.type == SDL_MOUSEMOTION)
        {
            this->mouseTelaX = static_cast<float>(evento.motion.x);
            this->mouseTelaY = static_cast<float>(evento.motion.y);
        }

        if (this->estado == EstadoJogo::PAUSADO)
        {
            const AcaoMenu acao = this->menu.tratar_evento(evento);

            if (acao == AcaoMenu::CONTINUAR)
            {
                this->retomar();
            }
            else if (acao == AcaoMenu::SAIR)
            {
                this->estado = EstadoJogo::SAINDO;
            }
        }
        else if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0)
        {
            if (evento.key.keysym.sym == SDLK_ESCAPE)
            {
                this->pausar();
            }
            else if (evento.key.keysym.sym == SDLK_e)
            {
                if (this->fase.interagir(this->jogador))
                {
                    this->atirando = false;
                    this->esperaTiro = 0.0f;
                    this->atualizar_camera();
                    this->atualizar_mira();
                }
            }
            else if (evento.key.keysym.sym == SDLK_r)
            {
                this->fase.reiniciar(this->jogador);
                this->atirando = false;
                this->esperaTiro = 0.0f;
                this->atualizar_camera();
                this->atualizar_mira();
            }
        }
        else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT)
        {
            this->atirando = true;
        }
        else if (evento.type == SDL_MOUSEBUTTONUP && evento.button.button == SDL_BUTTON_LEFT)
        {
            this->atirando = false;
        }
    }
}

void Jogo::atualizar(float dt)
{
    if (this->fase.get_concluida())
    {
        return;
    }
    this->mover_jogador(dt);

    // A camera segue o jogador, entao precisa estar atualizada antes de
    // converter a posicao do mouse em coordenadas de mundo.
    this->atualizar_camera();
    this->atualizar_mira();

    this->esperaTiro -= dt;

    if (this->atirando && this->esperaTiro <= 0.0f)
    {
        this->fase.get_salaAtual().disparar(this->jogador);
        this->esperaTiro = INTERVALO_TIRO;
    }

    this->fase.get_salaAtual().atualizar(dt, this->jogador);
}

void Jogo::mover_jogador(float dt)
{
    const Uint8 *teclas = SDL_GetKeyboardState(nullptr);

    int dirX = 0;
    int dirY = 0;

    if (teclas[SDL_SCANCODE_A] || teclas[SDL_SCANCODE_LEFT])
        dirX -= 1;
    if (teclas[SDL_SCANCODE_D] || teclas[SDL_SCANCODE_RIGHT])
        dirX += 1;
    if (teclas[SDL_SCANCODE_W] || teclas[SDL_SCANCODE_UP])
        dirY -= 1;
    if (teclas[SDL_SCANCODE_S] || teclas[SDL_SCANCODE_DOWN])
        dirY += 1;

    this->jogador.mover(dirX, dirY, dt);
    this->fase.get_salaAtual().limitar_jogador(this->jogador);
}

void Jogo::atualizar_mira()
{
    const SDL_FPoint alvo = this->camera.para_mundo(this->mouseTelaX, this->mouseTelaY);
    this->jogador.mirar_em(alvo.x, alvo.y);
}

void Jogo::desenhar()
{
    SDL_SetRenderDrawColor(this->renderizador, 18, 22, 30, 255);
    SDL_RenderClear(this->renderizador);

    this->fase.get_salaAtual().desenhar(this->renderizador, this->camera,
                                       this->fase.get_indiceAtual() > 0);

    this->jogador.desenhar(this->renderizador, this->camera);
    this->fase.desenhar_painel(this->renderizador, this->jogador);

    if (this->estado == EstadoJogo::PAUSADO)
    {
        this->menu.desenhar(this->renderizador);
    }

    SDL_RenderPresent(this->renderizador);
}

void Jogo::atualizar_camera()
{
    this->camera.centralizar_em(this->jogador.get_centroX(), this->jogador.get_centroY());
    // Margens mantem paredes visiveis e o piso fora do painel/rodape nas bordas.
    SDL_FRect vista = this->fase.get_salaAtual().get_limites();
    vista.x -= 24;
    vista.y -= 80;
    vista.w += 48;
    vista.h += 130;
    this->camera.limitar_a(vista);
}

void Jogo::pausar()
{
    this->menu.reiniciar();
    this->estado = EstadoJogo::PAUSADO;
    this->atirando = false; // solta o gatilho para nao voltar atirando sozinho
}

void Jogo::retomar()
{
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::finalizar()
{
    Desenho::finalizar();

    if (this->renderizador != nullptr)
    {
        SDL_DestroyRenderer(this->renderizador);
        this->renderizador = nullptr;
    }
    if (this->janela != nullptr)
    {
        SDL_DestroyWindow(this->janela);
        this->janela = nullptr;
    }
    SDL_Quit();
}

int Jogo::get_inimigosVivos() const
{
    return this->fase.get_salaAtual().get_inimigosVivos();
}
