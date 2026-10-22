#include "nucleo/Jogo.h"

#include <algorithm>
#include <cstdio>
#include <vector>

#include "config/Config.h"
#include "graficos/Desenho.h"
#include "interface/Texto.h"
#include "interface/Hud.h"
#include "interface/Minimapa.h"

namespace
{
static_assert(static_cast<int>(TipoArma::BOOMERANG) + 1 == QUANTIDADE_TIPOS_ARMA,
              "O save precisa conhecer todos os tipos de arma");
static_assert(MAX_ARMAS == CAPACIDADE_INVENTARIO, "O save precisa caber no inventario");

// Converte o inventario para a forma neutra do save, preservando a ordem FIFO.
std::vector<ArmaSalva> para_save(const FilaArmas &armas)
{
    std::vector<ArmaSalva> salvas;
    for (int i = 0; i < armas.get_quantidade(); ++i)
    {
        const Armas *arma = armas.consultar(static_cast<std::size_t>(i));
        salvas.push_back({static_cast<int>(arma->get_tipo()), arma->get_municao()});
    }
    return salvas;
}

FilaArmas do_save(const std::vector<ArmaSalva> &salvas)
{
    FilaArmas armas;
    for (const auto &salva : salvas)
    {
        Armas arma(static_cast<TipoArma>(salva.tipo));
        arma.set_municao(salva.municao);
        armas.inserir(arma);
    }
    return armas;
}

// Trava o passo de tempo: sem isso, arrastar a janela acumula um dt enorme
// e o jogador atravessaria meio mapa de uma vez.
constexpr float DT_MAXIMO = 0.05f;

std::filesystem::path diretorio_jogo()
{
    char *base = SDL_GetBasePath();
    const auto diretorio = base ? std::filesystem::path(base) : std::filesystem::current_path();
    SDL_free(base);
    return diretorio;
}
} // namespace

Jogo::Jogo()
    : janela(nullptr), renderizador(nullptr), estado(EstadoJogo::INICIAL), progresso(diretorio_jogo() / "saves" / "progresso.json"),
      tempoAnterior(0),
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

    const auto caminhoSprite = diretorio_jogo() / "assets" / "sprites" / "personagens" / "tux.png";
    if (!this->spriteJogador.carregar(this->renderizador, caminhoSprite.string()))
    {
        std::fprintf(stderr, "Erro ao carregar %s: %s\n", caminhoSprite.string().c_str(),
                     this->spriteJogador.get_erro().c_str());
        return false;
    }

    this->abrir_inicio();
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

        if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
            evento.key.keysym.sym == SDLK_F5 && this->savePendente)
        {
            this->salvar_progresso();
            if (this->estado == EstadoJogo::INICIAL)
            {
                this->abrir_inicio();
            }
            continue;
        }
        if (this->estado == EstadoJogo::INICIAL)
        {
            const AcaoMenu acao = this->menu.tratar_evento(evento);
            if (acao == AcaoMenu::NOVO_JOGO)
            {
                this->iniciar_partida(false);
            }
            else if (acao == AcaoMenu::CONTINUAR)
            {
                this->iniciar_partida(true);
            }
            else if (acao == AcaoMenu::SAIR)
            {
                this->estado = EstadoJogo::SAINDO;
            }
            continue;
        }
        if (this->estado == EstadoJogo::VITORIA)
        {
            if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                (evento.key.keysym.sym == SDLK_RETURN || evento.key.keysym.sym == SDLK_ESCAPE))
            {
                this->abrir_inicio();
            }
            continue;
        }

        // A mira acompanha o mouse mesmo com o jogo pausado.
        if (evento.type == SDL_MOUSEMOTION)
        {
            this->mouseTelaX = static_cast<float>(evento.motion.x);
            this->mouseTelaY = static_cast<float>(evento.motion.y);
        }

        if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
            evento.key.keysym.sym == SDLK_m)
        {
            this->mostrarMinimapa = !this->mostrarMinimapa;
            this->atirando = false;
            continue;
        }
        if (this->mostrarMinimapa)
        {
            if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                evento.key.keysym.sym == SDLK_ESCAPE)
            {
                this->mostrarMinimapa = false;
            }
            continue;
        }

        if (this->estado == EstadoJogo::PAUSADO)
        {
            const AcaoMenu acao = this->menu.tratar_evento(evento);

            if (acao == AcaoMenu::CONTINUAR)
            {
                this->retomar();
            }
            else if (acao == AcaoMenu::REINICIAR)
            {
                this->campanha.reiniciar_fase(this->jogador);
                this->preparar_entrada();
                this->retomar();
            }
            else if (acao == AcaoMenu::MENU_INICIAL)
            {
                this->abrir_inicio();
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
            else if (evento.key.keysym.sym == SDLK_F2)
            {
                this->mostrarHitboxes = !this->mostrarHitboxes;
            }
            else if (evento.key.keysym.sym == SDLK_f)
            {
                this->campanha.get_faseAtual().get_salaAtual().coletar_arma(this->jogador);
            }
            else if (evento.key.keysym.sym == SDLK_q)
            {
                this->jogador.get_armas().alternar();
            }
            else if (evento.key.keysym.sym == SDLK_g)
            {
                this->campanha.get_faseAtual().get_salaAtual().soltar_arma(this->jogador);
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
    if (this->campanha.concluida() || this->mostrarMinimapa)
    {
        return;
    }
    this->mover_jogador(dt);

    // A camera segue o jogador, entao precisa estar atualizada antes de
    // converter a posicao do mouse em coordenadas de mundo.
    this->atualizar_camera();
    this->atualizar_mira();

    this->esperaTiro -= dt;
    this->tempoTituloFase = std::max(0.0f, this->tempoTituloFase - dt);

    if (this->atirando && this->esperaTiro <= 0.0f)
    {
        this->campanha.get_faseAtual().get_salaAtual().disparar(this->jogador);
        const auto *arma = this->jogador.get_armaEquipada();
        this->esperaTiro = arma ? arma->get_intervalo() : INTERVALO_TIRO;
    }

    this->campanha.get_faseAtual().atualizar(dt, this->jogador);
    if (this->jogador.morto())
    {
        this->campanha.reiniciar_fase(this->jogador);
        this->preparar_entrada();
        return;
    }
    const int concluidasAntes = this->campanha.get_fasesConcluidas();
    if (this->campanha.atualizar_transicao(this->jogador))
    {
        if (this->campanha.get_fasesConcluidas() != concluidasAntes)
        {
            this->preparar_entrada();
            this->salvar_progresso();
        }
        else
        {
            // Uma passagem continua nao interrompe o tiro nem reinicia a cadencia.
            this->atualizar_camera();
            this->atualizar_mira();
        }
        if (this->campanha.concluida())
        {
            this->estado = EstadoJogo::VITORIA;
        }
    }
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
    this->campanha.get_faseAtual().get_salaAtual().limitar_jogador(this->jogador);
}

void Jogo::atualizar_mira()
{
    SDL_FPoint alvo;
    if (!this->campanha.get_faseAtual().get_salaAtual().buscar_alvoMaisProximo(this->jogador, alvo))
    {
        alvo = this->camera.para_mundo(this->mouseTelaX, this->mouseTelaY);
    }
    this->jogador.mirar_em(alvo.x, alvo.y);
}

void Jogo::desenhar()
{
    SDL_SetRenderDrawColor(this->renderizador, 18, 22, 30, 255);
    SDL_RenderClear(this->renderizador);

    if (this->estado == EstadoJogo::INICIAL)
    {
        this->menu.desenhar(this->renderizador);
    }
    else if (this->estado == EstadoJogo::VITORIA)
    {
        Texto::desenhar_centralizado(this->renderizador, "CAMPANHA CONCLUIDA!", 480, 200, 4, COR_TEXTO);
        Texto::desenhar_centralizado(this->renderizador, "VOCE COMPLETOU AS " + std::to_string(Campanha::TOTAL_FASES) + " FASES", 480, 270, 2, COR_TEXTO);
        Texto::desenhar_centralizado(this->renderizador, "ENTER: MENU INICIAL", 480, 320, 2, COR_TEXTO);
    }
    else
    {
        this->campanha.get_faseAtual().desenhar(this->renderizador, this->camera,
                                               this->mostrarHitboxes);

        this->jogador.desenhar(this->renderizador, this->camera, this->mostrarHitboxes,
                               &this->spriteJogador);
        if (this->estado != EstadoJogo::PAUSADO)
        {
            Hud::desenhar(this->renderizador, this->jogador);
        }
        if (this->estado == EstadoJogo::JOGANDO && this->tempoTituloFase > 0.0f)
        {
            // Titulo temporario da fase, que some gradualmente no ultimo segundo.
            const auto alfa = static_cast<Uint8>(255 * std::min(1.0f, this->tempoTituloFase));
            const auto &fase = this->campanha.get_faseAtual();
            Texto::desenhar_centralizado(this->renderizador, fase.get_nome(), 480, 150, 3,
                                         {255, 236, 170, alfa});
            Texto::desenhar_centralizado(this->renderizador, fase.get_subtitulo(), 480, 186, 2,
                                         {220, 204, 240, alfa});
        }

        if (this->estado == EstadoJogo::PAUSADO)
        {
            this->menu.desenhar(this->renderizador);
        }
    }
    if (this->estado != EstadoJogo::JOGANDO && !this->mensagemSave.empty())
    {
        SDL_SetRenderDrawColor(this->renderizador, 18, 22, 30, 240);
        const SDL_Rect aviso = {0, ALTURA_TELA - 62, LARGURA_TELA, 26};
        SDL_RenderFillRect(this->renderizador, &aviso);
        Texto::desenhar_centralizado(this->renderizador, this->mensagemSave, 480,
                                     ALTURA_TELA - 56, 2, COR_TEXTO);
    }

    if (this->mostrarMinimapa &&
        (this->estado == EstadoJogo::JOGANDO || this->estado == EstadoJogo::PAUSADO))
    {
        Minimapa::desenhar(this->renderizador, this->campanha.get_faseAtual(), this->jogador);
    }
    SDL_RenderPresent(this->renderizador);
}

void Jogo::atualizar_camera()
{
    this->camera.centralizar_em(this->jogador.get_centroX(), this->jogador.get_centroY());
    const auto vista = this->campanha.get_faseAtual().get_limitesVisuais();
    this->camera.limitar_a(vista);
}

void Jogo::pausar()
{
    const auto &sala = this->campanha.get_faseAtual().get_salaAtual();
    const std::string resumo = std::string(this->campanha.get_faseAtual().get_nome()) + "\n" +
        sala.get_configuracao().nome;
    this->menu.configurar_pausa(resumo);
    this->estado = EstadoJogo::PAUSADO;
    this->atirando = false; // solta o gatilho para nao voltar atirando sozinho
}

void Jogo::retomar()
{
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::finalizar()
{
    this->spriteJogador.liberar();
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
    return this->campanha.get_faseAtual().get_salaAtual().get_inimigosVivos();
}

void Jogo::abrir_inicio()
{
    ProgressoSalvo salvo;
    std::string detalhe;
    const bool valido = this->progresso.carregar(salvo, detalhe);
    this->menu.configurar_inicial(valido && salvo.fasesConcluidas < Campanha::TOTAL_FASES, detalhe);
    this->estado = EstadoJogo::INICIAL;
    this->atirando = false;
    if (!this->savePendente)
    {
        this->mensagemSave.clear();
    }
}

void Jogo::iniciar_partida(bool continuar)
{
    ProgressoSalvo salvo;
    if (continuar)
    {
        std::string detalhe;
        if (!this->progresso.carregar(salvo, detalhe) ||
            salvo.fasesConcluidas == Campanha::TOTAL_FASES)
        {
            this->abrir_inicio();
            return;
        }
    }
    this->campanha.iniciar(salvo.fasesConcluidas, this->jogador, do_save(salvo.armas));
    this->savePendente = false;
    this->mensagemSave.clear();
    if (!continuar)
    {
        this->salvar_progresso();
    }
    this->preparar_entrada();
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::salvar_progresso()
{
    // O checkpoint guarda as armas com que a proxima fase comeca.
    const ProgressoSalvo salvo = {this->campanha.get_fasesConcluidas(),
                                  para_save(this->campanha.get_armasInicioFase())};
    this->savePendente = !this->progresso.salvar(salvo, this->mensagemSave);
}

void Jogo::preparar_entrada()
{
    this->mostrarMinimapa = false;
    this->tempoTituloFase = DURACAO_TITULO_FASE;
    this->atirando = false;
    this->esperaTiro = 0.0f;
    this->atualizar_camera();
    this->atualizar_mira();
}
