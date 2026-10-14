#include "Jogo.h"

#include <algorithm>
#include <cmath>
#include <cstdio>

#include "Config.h"
#include "Desenho.h"

namespace
{
    // Trava o passo de tempo: sem isso, arrastar a janela acumula um dt enorme
    // e o jogador atravessaria meio mapa de uma vez.
    constexpr float DT_MAXIMO = 0.05f;
}

Jogo::Jogo()
    : janela(nullptr), renderizador(nullptr),
      estado(EstadoJogo::JOGANDO), tempoAnterior(0),
      esperaTiro(0.0f), atirando(false),
      mouseTelaX(LARGURA_TELA / 2.0f + 120.0f),
      mouseTelaY(ALTURA_TELA / 2.0f)
{
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

    if(!Desenho::inicializar(this->renderizador))
    {
        std::fprintf(stderr, "Erro ao preparar o desenho: %s\n", SDL_GetError());
        return false;
    }

    // O mundo e aberto: a origem e so o ponto de partida, nao um canto.
    this->jogador.set_posicao(-this->jogador.get_largura() / 2.0f,
                              -this->jogador.get_altura()  / 2.0f);
    this->camera.centralizar_em(this->jogador.get_centroX(), this->jogador.get_centroY());
    this->atualizar_mira();
    this->povoar_inimigos();

    this->estado = EstadoJogo::JOGANDO;
    this->tempoAnterior = SDL_GetPerformanceCounter();
    return true;
}

EstadoJogo Jogo::get_estado() const
{
    return this->estado;
}

int Jogo::get_inimigos_vivos() const
{
    int vivos = 0;

    for(const Inimigo &inimigo : this->inimigos)
    {
        if(inimigo.get_vivo())
        {
            vivos++;
        }
    }
    return vivos;
}

void Jogo::povoar_inimigos()
{
    this->inimigos.clear();
    this->inimigos.reserve(QUANTIDADE_INIMIGOS);

    // Nascem espalhados num circulo em volta do jogador.
    for(int i = 0; i < QUANTIDADE_INIMIGOS; i++)
    {
        const float angulo = (2.0f * static_cast<float>(M_PI) * i) / QUANTIDADE_INIMIGOS;

        const float centroX = this->jogador.get_centroX() + std::cos(angulo) * RAIO_SURGIMENTO;
        const float centroY = this->jogador.get_centroY() + std::sin(angulo) * RAIO_SURGIMENTO;

        this->inimigos.push_back(Inimigo(centroX - LADO_INIMIGO / 2.0f,
                                         centroY - LADO_INIMIGO / 2.0f));
    }
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

        // A mira acompanha o mouse mesmo com o jogo pausado.
        if(evento.type == SDL_MOUSEMOTION)
        {
            this->mouseTelaX = static_cast<float>(evento.motion.x);
            this->mouseTelaY = static_cast<float>(evento.motion.y);
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
        else if(evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT)
        {
            this->atirando = true;
        }
        else if(evento.type == SDL_MOUSEBUTTONUP && evento.button.button == SDL_BUTTON_LEFT)
        {
            this->atirando = false;
        }
    }
}

void Jogo::atualizar(float dt)
{
    this->mover_jogador(dt);

    // A camera segue o jogador, entao precisa estar atualizada antes de
    // converter a posicao do mouse em coordenadas de mundo.
    this->camera.centralizar_em(this->jogador.get_centroX(), this->jogador.get_centroY());
    this->atualizar_mira();

    this->esperaTiro -= dt;

    if(this->atirando && this->esperaTiro <= 0.0f)
    {
        this->disparar();
        this->esperaTiro = INTERVALO_TIRO;
    }

    this->atualizar_projeteis(dt);
    this->atualizar_inimigos(dt);
    this->resolver_colisoes();
    this->limpar_inativos();
}

void Jogo::mover_jogador(float dt)
{
    const Uint8 *teclas = SDL_GetKeyboardState(nullptr);

    int dirX = 0;
    int dirY = 0;

    if(teclas[SDL_SCANCODE_A] || teclas[SDL_SCANCODE_LEFT])  dirX -= 1;
    if(teclas[SDL_SCANCODE_D] || teclas[SDL_SCANCODE_RIGHT]) dirX += 1;
    if(teclas[SDL_SCANCODE_W] || teclas[SDL_SCANCODE_UP])    dirY -= 1;
    if(teclas[SDL_SCANCODE_S] || teclas[SDL_SCANCODE_DOWN])  dirY += 1;

    // Mapa aberto: nada de limites, o jogador anda o quanto quiser.
    this->jogador.mover(dirX, dirY, dt);
}

void Jogo::atualizar_mira()
{
    const SDL_FPoint alvo = this->camera.para_mundo(this->mouseTelaX, this->mouseTelaY);
    this->jogador.mirar_em(alvo.x, alvo.y);
}

void Jogo::disparar()
{
    Armas *arma = this->jogador.get_armaEquipada();

    if(arma == nullptr || !arma->atirar())
    {
        return;   // sem arma na fila ou sem municao
    }

    const SDL_FPoint boca = this->jogador.get_boca_arma();
    const SDL_FPoint direcao = this->jogador.get_direcaoMira();

    this->projeteis.push_back(Projetil(boca.x, boca.y, direcao.x, direcao.y, arma->get_dano()));
}

void Jogo::atualizar_projeteis(float dt)
{
    for(Projetil &projetil : this->projeteis)
    {
        projetil.atualizar(dt);
    }
}

void Jogo::atualizar_inimigos(float dt)
{
    const float alvoX = this->jogador.get_centroX();
    const float alvoY = this->jogador.get_centroY();

    for(Inimigo &inimigo : this->inimigos)
    {
        inimigo.perseguir(alvoX, alvoY, dt);
    }

    // Sem isso os cinco se empilham num quadrado so ao chegar no jogador.
    for(std::size_t a = 0; a < this->inimigos.size(); a++)
    {
        for(std::size_t b = a + 1; b < this->inimigos.size(); b++)
        {
            this->inimigos[a].afastar_de(this->inimigos[b]);
        }
    }
}

void Jogo::resolver_colisoes()
{
    for(Projetil &projetil : this->projeteis)
    {
        if(!projetil.get_ativo())
        {
            continue;
        }

        const SDL_FRect areaProjetil = projetil.get_retangulo();

        for(Inimigo &inimigo : this->inimigos)
        {
            if(!inimigo.get_vivo())
            {
                continue;
            }

            const SDL_FRect areaInimigo = inimigo.get_retangulo();

            if(SDL_HasIntersectionF(&areaProjetil, &areaInimigo))
            {
                inimigo.levar_dano(projetil.get_dano());
                projetil.desativar();
                break;   // cada tiro acerta um inimigo so
            }
        }
    }
}

void Jogo::limpar_inativos()
{
    this->projeteis.erase(std::remove_if(this->projeteis.begin(), this->projeteis.end(),
                                         [](const Projetil &p){ return !p.get_ativo(); }),
                          this->projeteis.end());

    this->inimigos.erase(std::remove_if(this->inimigos.begin(), this->inimigos.end(),
                                        [](const Inimigo &i){ return !i.get_vivo(); }),
                         this->inimigos.end());
}

void Jogo::desenhar()
{
    SDL_SetRenderDrawColor(this->renderizador, COR_MUNDO.r, COR_MUNDO.g, COR_MUNDO.b, COR_MUNDO.a);
    SDL_RenderClear(this->renderizador);

    this->desenhar_grade();

    for(const Inimigo &inimigo : this->inimigos)
    {
        inimigo.desenhar(this->renderizador, this->camera);
    }

    for(const Projetil &projetil : this->projeteis)
    {
        projetil.desenhar(this->renderizador, this->camera);
    }

    this->jogador.desenhar(this->renderizador, this->camera);

    if(this->estado == EstadoJogo::PAUSADO)
    {
        this->menu.desenhar(this->renderizador);
    }

    SDL_RenderPresent(this->renderizador);
}

void Jogo::desenhar_grade()
{
    SDL_SetRenderDrawColor(this->renderizador, COR_GRADE.r, COR_GRADE.g, COR_GRADE.b, COR_GRADE.a);

    const float camX = this->camera.get_x();
    const float camY = this->camera.get_y();

    // As linhas ficam presas ao mundo, nao a tela: e isso que da a sensacao
    // de movimento num mapa que nao tem borda nenhuma.
    const float primeiraX = std::floor(camX / PASSO_GRADE) * PASSO_GRADE;
    const float primeiraY = std::floor(camY / PASSO_GRADE) * PASSO_GRADE;

    for(float mundoX = primeiraX; mundoX < camX + LARGURA_TELA; mundoX += PASSO_GRADE)
    {
        const float telaX = mundoX - camX;
        SDL_RenderDrawLineF(this->renderizador, telaX, 0.0f, telaX, ALTURA_TELA);
    }

    for(float mundoY = primeiraY; mundoY < camY + ALTURA_TELA; mundoY += PASSO_GRADE)
    {
        const float telaY = mundoY - camY;
        SDL_RenderDrawLineF(this->renderizador, 0.0f, telaY, LARGURA_TELA, telaY);
    }
}

void Jogo::pausar()
{
    this->menu.reiniciar();
    this->estado = EstadoJogo::PAUSADO;
    this->atirando = false;   // solta o gatilho para nao voltar atirando sozinho
}

void Jogo::retomar()
{
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::finalizar()
{
    Desenho::finalizar();

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
