#include "nucleo/Jogo.h"
#include "graficos/CatalogoSprites.h"

#include <algorithm>
#include <array>
#include <cmath>
#include <cstdio>
#include <iomanip>
#include <sstream>
#include <vector>

#include "audio/Som.h"
#include "config/Config.h"
#include "graficos/Desenho.h"
#include "interface/FundoMenu.h"
#include "interface/Hud.h"
#include "interface/Minimapa.h"
#include "interface/Texto.h"
#include "nucleo/Plataforma.h"

namespace
{
static_assert(static_cast<int>(TipoArma::PULSO) + 1 == QUANTIDADE_TIPOS_ARMA,
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

std::filesystem::path caminho_slot(int slot)
{
    return diretorio_jogo() / "saves" / ("progresso_" + std::to_string(slot) + ".json");
}

std::filesystem::path caminho_configuracoes()
{
    return diretorio_jogo() / "saves" / "configuracoes.txt";
}

// Milissegundos de sintese de audio por quadro durante o carregamento.
constexpr int ORCAMENTO_CARREGAMENTO_MS = 8;

std::string formatar_tempo(float segundos)
{
    std::ostringstream texto;
    texto << std::fixed << std::setprecision(1) << segundos << " S";
    return texto.str();
}

} // namespace

Jogo::Jogo()
    : janela(nullptr), renderizador(nullptr), estado(EstadoJogo::CARREGANDO),
      arquivoConfiguracoes(caminho_configuracoes()), progresso(caminho_slot(1)), tempoAnterior(0),
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
    // Toques chegam como SDL_FINGER*: no jogo viram joystick e botoes e, nas
    // outras telas, o proprio Jogo os converte em cliques (veja tratar_eventos).
    SDL_SetHint(SDL_HINT_TOUCH_MOUSE_EVENTS, "0");
    if (SDL_Init(SDL_INIT_VIDEO) != 0)
    {
        std::fprintf(stderr, "Erro ao iniciar o SDL: %s\n", SDL_GetError());
        return false;
    }

    // ALLOW_HIGHDPI: em telas densas (celulares, Retina) o buffer usa os pixels
    // fisicos. Sem ela o navegador do celular dava ao canvas so os pixels CSS
    // (ex.: 640x360), o jogo de 960x540 era reduzido e o texto fino sumia.
    this->janela = SDL_CreateWindow(
        TITULO_JANELA, SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, LARGURA_TELA, ALTURA_TELA,
        SDL_WINDOW_SHOWN | SDL_WINDOW_RESIZABLE | SDL_WINDOW_ALLOW_HIGHDPI);
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

    if (!this->spriteJogador.carregar(this->renderizador, AssetsEmbutidos::SPRITE_JOGADOR, 3))
    {
        std::fprintf(stderr, "Erro ao carregar o sprite do jogador: %s\n",
                     this->spriteJogador.get_erro().c_str());
        return false;
    }
    if (!this->spriteGameOver.carregar(this->renderizador, AssetsEmbutidos::SPRITE_GAME_OVER))
    {
        std::fprintf(stderr, "Erro ao carregar o sprite de fim de jogo: %s\n",
                     this->spriteGameOver.get_erro().c_str());
        return false;
    }

    if (!CatalogoSprites::carregar(this->renderizador))
    {
        std::fprintf(stderr, "Erro ao carregar o catalogo de sprites\n");
        return false;
    }
    // Sem audio o jogo segue mudo; a tela de carregamento continua igual.
    Som::inicializar();
    this->configuracoes = this->arquivoConfiguracoes.carregar();
    this->menu.set_configuracoes(this->configuracoes);
    this->aplicar_configuracoes();

    this->carregamento.reiniciar();
    this->estado = EstadoJogo::CARREGANDO;
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
        this->passo();
    }
}

void Jogo::passo()
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
    if (this->estado != EstadoJogo::JOGANDO || this->mostrarMinimapa)
    {
        // Dedos que estavam no joystick ou no gatilho nao seguem valendo
        // depois de pausa, mapa ou troca de tela.
        this->toque.soltar_tudo();
    }

    if (this->estado == EstadoJogo::CARREGANDO)
    {
        this->atualizar_carregamento(dt);
    }
    else if (this->estado == EstadoJogo::INICIAL || this->estado == EstadoJogo::PAUSADO)
    {
        this->menu.atualizar(dt);
    }

    if (this->estado == EstadoJogo::JOGANDO)
    {
        this->atualizar(dt);
    }
    else if (this->estado == EstadoJogo::GAME_OVER)
    {
        this->atualizar_gameOver(dt);
    }
    else if (this->estado == EstadoJogo::DERROTA)
    {
        this->derrota.atualizar(dt);
        if (this->derrota.terminou())
        {
            this->concluir_derrota();
        }
    }
    else if (this->estado == EstadoJogo::VITORIA)
    {
        const Uint8 *teclas = SDL_GetKeyboardState(nullptr);
        const SDL_TouchID tela = SDL_GetTouchDevice(0);
        const bool acelerar = teclas[SDL_SCANCODE_SPACE] ||
                              (SDL_GetMouseState(nullptr, nullptr) & SDL_BUTTON_LMASK) ||
                              (tela != 0 && SDL_GetNumTouchFingers(tela) > 0);
        const int navegar = (teclas[SDL_SCANCODE_DOWN] ? 1 : 0) - (teclas[SDL_SCANCODE_UP] ? 1 : 0);
        this->vitoria.atualizar(dt, acelerar, navegar);
        if (this->vitoria.terminou())
        {
            this->abrir_inicio();
        }
    }

    this->atualizar_musica();
    this->desenhar();
}

void Jogo::atualizar_carregamento(float dt)
{
    const bool estavaPronta = this->carregamento.pronta();
    const float progressoReal = Som::carregar_etapa(ORCAMENTO_CARREGAMENTO_MS);
    if (this->carregamento.atualizar(dt, progressoReal) > 0)
    {
        Som::tocar(EfeitoSonoro::CARREGAMENTO_LINHA);
    }
    if (!estavaPronta && this->carregamento.pronta())
    {
        Som::tocar(EfeitoSonoro::CARREGAMENTO_PRONTO);
    }
    if (this->carregamento.terminou())
    {
        this->abrir_inicio();
    }
}

void Jogo::atualizar_musica()
{
    Musica musica = Musica::MENU;
    switch (this->estado)
    {
    case EstadoJogo::VITORIA:
        musica = Musica::FINAL;
        break;
    case EstadoJogo::CARREGANDO:
    case EstadoJogo::DERROTA:
    case EstadoJogo::GAME_OVER:
    case EstadoJogo::SAINDO:
        musica = Musica::NENHUMA;
        break;
    case EstadoJogo::JOGANDO:
    case EstadoJogo::PAUSADO:
    {
        // A trilha do chefe toca enquanto a luta contra ele estiver em andamento.
        const Sala &sala = this->campanha.get_faseAtual().get_salaAtual();
        const bool lutaChefe =
            sala.get_configuracao().boss && sala.get_estado() == EstadoSala::EM_COMBATE;
        musica = lutaChefe ? Musica::CHEFE : Musica::FASE;
        break;
    }
    default:
        break;
    }
    Som::tocar_musica(musica);
    Som::set_abafado(this->estado == EstadoJogo::PAUSADO || this->mostrarMinimapa);
}

void Jogo::aplicar_configuracoes()
{
    const float maximo = static_cast<float>(Configuracoes::VOLUME_MAXIMO);
    Som::set_volumes(this->configuracoes.volumeMusica / maximo,
                     this->configuracoes.volumeEfeitos / maximo);
    if (Plataforma::PERMITE_TELA_CHEIA && this->janela != nullptr)
    {
        const bool atual = (SDL_GetWindowFlags(this->janela) & SDL_WINDOW_FULLSCREEN) != 0;
        if (atual != this->configuracoes.telaCheia)
        {
            SDL_SetWindowFullscreen(
                this->janela, this->configuracoes.telaCheia ? SDL_WINDOW_FULLSCREEN_DESKTOP : 0);
        }
    }
}

void Jogo::salvar_configuracoes()
{
    // Falhar aqui so perde a preferencia; o jogo continua com os valores atuais.
    if (this->arquivoConfiguracoes.salvar(this->configuracoes))
    {
        Plataforma::persistir_saves();
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
        this->atualizar_modoEntrada(evento);
        if (evento.type == SDL_FINGERDOWN || evento.type == SDL_FINGERUP ||
            evento.type == SDL_FINGERMOTION)
        {
            if (this->estado == EstadoJogo::JOGANDO && !this->mostrarMinimapa)
            {
                this->tratar_toqueJogo(evento);
                continue;
            }
            // Fora da partida um toque vale um clique no mesmo ponto da tela.
            const SDL_TouchFingerEvent dedo = evento.tfinger;
            const int x = static_cast<int>(dedo.x * LARGURA_TELA);
            const int y = static_cast<int>(dedo.y * ALTURA_TELA);
            SDL_zero(evento);
            if (dedo.type == SDL_FINGERMOTION)
            {
                evento.type = SDL_MOUSEMOTION;
                evento.motion.which = SDL_TOUCH_MOUSEID;
                evento.motion.x = x;
                evento.motion.y = y;
            }
            else
            {
                evento.type = dedo.type == SDL_FINGERDOWN ? SDL_MOUSEBUTTONDOWN : SDL_MOUSEBUTTONUP;
                evento.button.which = SDL_TOUCH_MOUSEID;
                evento.button.button = SDL_BUTTON_LEFT;
                evento.button.state = dedo.type == SDL_FINGERDOWN ? SDL_PRESSED : SDL_RELEASED;
                evento.button.clicks = 1;
                evento.button.x = x;
                evento.button.y = y;
            }
        }
        const bool clique =
            evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT;
        if (this->estado == EstadoJogo::CARREGANDO)
        {
            if ((evento.type == SDL_KEYDOWN && evento.key.repeat == 0) ||
                evento.type == SDL_MOUSEBUTTONDOWN)
            {
                this->carregamento.pular();
            }
            continue;
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
        if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
            (this->estado == EstadoJogo::JOGANDO || this->estado == EstadoJogo::PAUSADO))
        {
            this->registrar_cheat(evento.key.keysym.sym);
        }
        if (this->estado == EstadoJogo::INICIAL)
        {
            const AcaoMenu acao = this->menu.tratar_evento(evento);
            if (acao == AcaoMenu::NOVO_JOGO)
            {
                this->abrir_selecaoSlots(true);
            }
            else if (acao == AcaoMenu::CONTINUAR)
            {
                this->abrir_selecaoSlots(false);
            }
            else if (acao == AcaoMenu::SLOT_1 || acao == AcaoMenu::SLOT_2 ||
                     acao == AcaoMenu::SLOT_3)
            {
                const int slot = acao == AcaoMenu::SLOT_1 ? 1 : (acao == AcaoMenu::SLOT_2 ? 2 : 3);
                this->iniciar_partida(!this->selecionandoNovoJogo, slot);
            }
            else if (acao == AcaoMenu::VOLTAR)
            {
                this->abrir_inicio(true);
            }
            else if (acao == AcaoMenu::CONFIGURACOES_ALTERADAS)
            {
                this->configuracoes = this->menu.get_configuracoes();
                this->aplicar_configuracoes();
                this->salvar_configuracoes();
                Som::tocar(EfeitoSonoro::MENU_AJUSTE);
            }
            else if (acao == AcaoMenu::SAIR)
            {
                this->estado = EstadoJogo::SAINDO;
            }
            continue;
        }
        if (this->estado == EstadoJogo::RESUMO_FASE)
        {
            // Um toque ou clique logo apos a fase acabar ainda era um tiro.
            constexpr Uint32 ESPERA_CLIQUE_RESUMO_MS = 1000;
            const bool cliqueValido =
                clique && SDL_GetTicks() - this->inicioResumo >= ESPERA_CLIQUE_RESUMO_MS;
            if (cliqueValido ||
                (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                 (evento.key.keysym.sym == SDLK_RETURN || evento.key.keysym.sym == SDLK_KP_ENTER ||
                  evento.key.keysym.sym == SDLK_SPACE)))
            {
                if (this->campanha.concluida())
                {
                    this->estado = EstadoJogo::VITORIA;
                    this->vitoria.iniciar(this->pontuacaoTotal);
                    Som::tocar(EfeitoSonoro::VITORIA);
                }
                else
                {
                    this->preparar_entrada();
                    this->estado = EstadoJogo::JOGANDO;
                }
            }
            continue;
        }
        if (this->estado == EstadoJogo::GAME_OVER)
        {
            const bool confirmar =
                clique ||
                (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                 (evento.key.keysym.sym == SDLK_RETURN || evento.key.keysym.sym == SDLK_KP_ENTER));
            if (confirmar && this->continueDisponivel)
            {
                this->aceitar_continue();
            }
            continue;
        }
        if (this->estado == EstadoJogo::VITORIA)
        {
            if (clique)
            {
                // Sai da celebracao ou, nos creditos, volta ao menu. Durante o
                // texto, segurar o dedo ou o botao apenas acelera a rolagem.
                this->vitoria.avancar();
            }
            else if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0)
            {
                if (evento.key.keysym.sym == SDLK_ESCAPE)
                {
                    this->vitoria.pular();
                }
                else if (evento.key.keysym.sym == SDLK_RETURN ||
                         evento.key.keysym.sym == SDLK_KP_ENTER)
                {
                    this->vitoria.avancar();
                }
            }
            continue;
        }
        if (this->estado == EstadoJogo::DERROTA)
        {
            if ((evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                 (evento.key.keysym.sym == SDLK_RETURN || evento.key.keysym.sym == SDLK_KP_ENTER ||
                  evento.key.keysym.sym == SDLK_SPACE)) ||
                (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT))
            {
                this->derrota.pular();
            }
            continue;
        }

        // A mira acompanha o mouse mesmo com o jogo pausado.
        if (evento.type == SDL_MOUSEMOTION)
        {
            this->mouseTelaX = static_cast<float>(evento.motion.x);
            this->mouseTelaY = static_cast<float>(evento.motion.y);
        }

        if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 && evento.key.keysym.sym == SDLK_m)
        {
            this->mostrarMinimapa = !this->mostrarMinimapa;
            this->atirando = false;
            continue;
        }
        if (this->mostrarMinimapa)
        {
            if (clique || (evento.type == SDL_KEYDOWN && evento.key.repeat == 0 &&
                           evento.key.keysym.sym == SDLK_ESCAPE))
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
                this->configuracoes.mostrarHitboxes = !this->configuracoes.mostrarHitboxes;
                this->menu.set_configuracoes(this->configuracoes);
                this->salvar_configuracoes();
            }
            else if (evento.key.keysym.sym == SDLK_f)
            {
                this->campanha.get_faseAtual().get_salaAtual().coletar_arma(this->jogador);
            }
            else if (evento.key.keysym.sym == SDLK_q)
            {
                if (this->jogador.get_armas().get_quantidade() > 1)
                {
                    Som::tocar(EfeitoSonoro::ARMA_TROCA);
                }
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

void Jogo::atualizar_modoEntrada(const SDL_Event &evento)
{
    // Os controles de toque aparecem no primeiro toque e somem quando o
    // jogador volta ao teclado ou ao mouse (notebook com tela de toque).
    bool toque = this->modoToque;
    if (evento.type == SDL_FINGERDOWN)
    {
        toque = true;
    }
    else if (evento.type == SDL_KEYDOWN ||
             (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.which != SDL_TOUCH_MOUSEID) ||
             (evento.type == SDL_MOUSEMOTION && evento.motion.which != SDL_TOUCH_MOUSEID))
    {
        toque = false;
    }
    if (toque == this->modoToque)
    {
        return;
    }
    this->modoToque = toque;
    this->toque.soltar_tudo();
    this->menu.set_modoToque(toque);
    this->vitoria.set_modoToque(toque);
}

void Jogo::tratar_toqueJogo(const SDL_Event &evento)
{
    Sala &sala = this->campanha.get_faseAtual().get_salaAtual();
    switch (this->toque.tratar_evento(evento))
    {
    case AcaoToque::PAUSAR:
        this->pausar();
        break;
    case AcaoToque::MINIMAPA:
        this->mostrarMinimapa = true;
        this->atirando = false;
        break;
    case AcaoToque::PEGAR_ARMA:
        sala.coletar_arma(this->jogador);
        break;
    case AcaoToque::TROCAR_ARMA:
        if (this->jogador.get_armas().get_quantidade() > 1)
        {
            Som::tocar(EfeitoSonoro::ARMA_TROCA);
        }
        this->jogador.get_armas().alternar();
        break;
    case AcaoToque::SOLTAR_ARMA:
        sala.soltar_arma(this->jogador);
        break;
    case AcaoToque::NENHUMA:
        break;
    }
}

void Jogo::atualizar(float dt)
{
    if (this->campanha.concluida() || this->mostrarMinimapa)
    {
        return;
    }
    this->tempoFase += dt;
    this->tempoMensagemCheat = std::max(0.0f, this->tempoMensagemCheat - dt);
    this->mover_jogador(dt);

    // A camera segue o jogador, entao precisa estar atualizada antes de
    // converter a posicao do mouse em coordenadas de mundo.
    this->atualizar_camera();
    this->atualizar_mira();

    this->esperaTiro -= dt;
    this->tempoTituloFase = std::max(0.0f, this->tempoTituloFase - dt);

    if ((this->atirando || this->toque.get_atirando()) && this->esperaTiro <= 0.0f)
    {
        this->campanha.get_faseAtual().get_salaAtual().disparar(this->jogador);
        const auto *arma = this->jogador.get_armaEquipada();
        this->esperaTiro = arma ? arma->get_intervalo() : INTERVALO_SOCO;
    }

    this->campanha.get_faseAtual().atualizar(dt, this->jogador);
    this->atualizar_pontuacaoInimigos();
    if (this->jogador.morto())
    {
        this->iniciar_derrota();
        return;
    }
    if (this->campanha.atualizar_transicao(this->jogador))
    {
        if (this->campanha.get_faseAtual().get_concluida())
        {
            this->concluir_fase();
        }
        else
        {
            // Uma passagem continua nao interrompe o tiro nem reinicia a cadencia.
            this->atualizar_camera();
            this->atualizar_mira();
        }
    }
}

void Jogo::atualizar_gameOver(float dt)
{
    this->tempoGameOver = std::max(0.0f, this->tempoGameOver - dt);
    if (this->tempoGameOver <= 0.0f)
    {
        this->encerrar_gameOver();
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

    float toqueX = 0.0f;
    float toqueY = 0.0f;
    this->toque.get_direcao(toqueX, toqueY);
    if (toqueX != 0.0f || toqueY != 0.0f)
    {
        this->jogador.mover_analogico(toqueX, toqueY, dt);
    }
    else
    {
        this->jogador.mover(dirX, dirY, dt);
    }
    this->campanha.get_faseAtual().get_salaAtual().limitar_jogador(this->jogador);
}

void Jogo::atualizar_mira()
{
    SDL_FPoint alvo;
    if (this->campanha.get_faseAtual().get_salaAtual().buscar_alvoMaisProximo(this->jogador, alvo))
    {
        // A mira trava no inimigo mais proximo.
    }
    else if (this->modoToque)
    {
        // Sem mouse, a mira acompanha a ultima direcao do joystick.
        float dirX = 0.0f;
        float dirY = 0.0f;
        this->toque.get_ultimaDirecao(dirX, dirY);
        alvo = {this->jogador.get_centroX() + dirX * 160.0f,
                this->jogador.get_centroY() + dirY * 160.0f};
    }
    else
    {
        alvo = this->camera.para_mundo(this->mouseTelaX, this->mouseTelaY);
    }
    this->jogador.mirar_em(alvo.x, alvo.y);
}

void Jogo::desenhar()
{
    SDL_SetRenderDrawColor(this->renderizador, 18, 22, 30, 255);
    SDL_RenderClear(this->renderizador);

    if (this->estado == EstadoJogo::CARREGANDO)
    {
        this->carregamento.desenhar(this->renderizador, this->spriteJogador);
    }
    else if (this->estado == EstadoJogo::INICIAL)
    {
        this->menu.desenhar(this->renderizador, this->spriteJogador);
    }
    else if (this->estado == EstadoJogo::RESUMO_FASE)
    {
        this->desenhar_resumoFase();
    }
    else if (this->estado == EstadoJogo::GAME_OVER)
    {
        this->desenhar_gameOver();
    }
    else if (this->estado == EstadoJogo::VITORIA)
    {
        this->vitoria.desenhar(this->renderizador, this->spriteJogador);
    }
    else
    {
        this->campanha.get_faseAtual().desenhar(this->renderizador, this->camera,
                                                this->configuracoes.mostrarHitboxes);

        this->jogador.desenhar(this->renderizador, this->camera,
                               this->configuracoes.mostrarHitboxes, &this->spriteJogador);
        if (this->estado == EstadoJogo::DERROTA)
        {
            this->derrota.desenhar(this->renderizador, this->modoToque);
        }
        else if (this->estado != EstadoJogo::PAUSADO)
        {
            Hud::desenhar(this->renderizador, this->jogador, this->spriteJogador, this->vidas,
                          this->pontosComunsFase + this->pontosBossFase, this->tempoFase);
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

        if (this->estado == EstadoJogo::JOGANDO && this->modoToque && !this->mostrarMinimapa)
        {
            const Sala &sala = this->campanha.get_faseAtual().get_salaAtual();
            this->toque.desenhar(this->renderizador, sala.tem_armaProxima(this->jogador));
        }
        if (this->estado == EstadoJogo::PAUSADO)
        {
            this->menu.desenhar(this->renderizador, this->spriteJogador);
        }
        if (this->tempoMensagemCheat > 0.0f)
        {
            Texto::desenhar_centralizado(this->renderizador,
                                         "CHEAT 6767 ATIVO - TIROS INIMIGOS IGNORADOS", 480, 155, 2,
                                         {130, 255, 180, 255});
        }
    }
    // Manual e Sobre ocupam a tela inteira; o aviso de save cobriria o botao VOLTAR.
    const bool painelInteiro =
        (this->estado == EstadoJogo::INICIAL || this->estado == EstadoJogo::PAUSADO) &&
        (this->menu.get_tela() == TelaMenu::MANUAL || this->menu.get_tela() == TelaMenu::SOBRE);
    const bool animacao = this->estado == EstadoJogo::CARREGANDO ||
                          this->estado == EstadoJogo::DERROTA ||
                          this->estado == EstadoJogo::VITORIA;
    if (this->estado != EstadoJogo::JOGANDO && !animacao && !painelInteiro &&
        !this->mensagemSave.empty())
    {
        SDL_SetRenderDrawColor(this->renderizador, 18, 22, 30, 240);
        const SDL_Rect aviso = {0, ALTURA_TELA - 62, LARGURA_TELA, 26};
        SDL_RenderFillRect(this->renderizador, &aviso);
        Texto::desenhar_centralizado(this->renderizador, this->mensagemSave, 480, ALTURA_TELA - 56,
                                     2, COR_TEXTO);
    }

    if (this->mostrarMinimapa &&
        (this->estado == EstadoJogo::JOGANDO || this->estado == EstadoJogo::PAUSADO))
    {
        Minimapa::desenhar(this->renderizador, this->campanha.get_faseAtual(), this->jogador,
                           this->modoToque);
    }
    SDL_RenderPresent(this->renderizador);
}

void Jogo::desenhar_resumoFase() const
{
    const SDL_Rect painel = {145, 55, 670, 430};
    SDL_SetRenderDrawColor(this->renderizador, 15, 23, 36, 255);
    SDL_RenderFillRect(this->renderizador, &painel);
    SDL_SetRenderDrawColor(this->renderizador, 92, 145, 178, 255);
    SDL_RenderDrawRect(this->renderizador, &painel);
    Texto::desenhar_centralizado(this->renderizador,
                                 "FASE " + std::to_string(resumoFase.numero) + " CONCLUIDA", 480,
                                 82, 4, {235, 244, 255, 255});
    Texto::desenhar_centralizado(this->renderizador, "TEMPO: " + formatar_tempo(resumoFase.tempo),
                                 480, 145, 2, {207, 220, 236, 255});
    Texto::desenhar(this->renderizador, "INIMIGOS COMUNS", 245, 205, 2, COR_TEXTO);
    Texto::desenhar(this->renderizador, std::to_string(resumoFase.pontosComuns), 650, 205, 2,
                    {255, 222, 110, 255});
    Texto::desenhar(this->renderizador, "BOSS", 245, 245, 2, COR_TEXTO);
    Texto::desenhar(this->renderizador, std::to_string(resumoFase.pontosBoss), 650, 245, 2,
                    {255, 222, 110, 255});
    Texto::desenhar(this->renderizador, "BONUS DE RAPIDEZ", 245, 285, 2, COR_TEXTO);
    Texto::desenhar(this->renderizador, std::to_string(resumoFase.pontosRapidez), 650, 285, 2,
                    {255, 222, 110, 255});
    SDL_SetRenderDrawColor(this->renderizador, 92, 145, 178, 180);
    SDL_RenderDrawLine(this->renderizador, 235, 330, 725, 330);
    Texto::desenhar(this->renderizador, "TOTAL DA FASE", 245, 350, 2, COR_TEXTO);
    Texto::desenhar(this->renderizador, std::to_string(resumoFase.total), 650, 350, 2,
                    {255, 222, 110, 255});
    Texto::desenhar_centralizado(this->renderizador,
                                 "TOTAL DA CAMPANHA: " + std::to_string(pontuacaoTotal), 480, 394,
                                 2, {130, 255, 180, 255});
    Texto::desenhar_centralizado(
        this->renderizador,
        std::string(modoToque ? "TOQUE" : "ENTER") +
            (campanha.concluida() ? ": VER RESULTADO FINAL" : ": PROXIMA FASE"),
        480, 450, 2, COR_TEXTO_APAGADO);
}

void Jogo::desenhar_gameOver() const
{
    SDL_SetRenderDrawColor(this->renderizador, 8, 8, 14, 255);
    SDL_RenderClear(this->renderizador);
    Texto::desenhar_centralizado(this->renderizador, "GAME OVER", 480, 52, 6, {225, 45, 55, 255});
    this->spriteGameOver.desenhar(this->renderizador, {385.0f, 104.0f, 190.0f, 180.0f});
    const int segundos = static_cast<int>(std::ceil(tempoGameOver));
    Texto::desenhar_centralizado(this->renderizador, std::to_string(segundos), 480, 296, 7,
                                 {255, 222, 110, 255});
    if (continueDisponivel)
    {
        Texto::desenhar_centralizado(this->renderizador, "CONTINUE?", 480, 370, 4,
                                     {235, 244, 255, 255});
        Texto::desenhar_centralizado(this->renderizador,
                                     modoToque ? "TOQUE: USAR O CONTINUE UNICO"
                                               : "ENTER: USAR O CONTINUE UNICO",
                                     480, 430, 2, {130, 255, 180, 255});
    }
    else
    {
        Texto::desenhar_centralizado(this->renderizador, "SEM CONTINUE", 480, 380, 4,
                                     {225, 45, 55, 255});
        Texto::desenhar_centralizado(this->renderizador, "O SLOT SERA APAGADO", 480, 438, 2,
                                     COR_TEXTO_APAGADO);
    }
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
    Som::finalizar();
    this->spriteJogador.liberar();
    this->spriteGameOver.liberar();
    CatalogoSprites::liberar();
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

void Jogo::abrir_inicio(bool telaJogar)
{
    bool podeContinuar = false;
    int slotsOcupados = 0;
    for (int slot = 1; slot <= QUANTIDADE_SLOTS_SAVE; ++slot)
    {
        ArquivoProgresso arquivo(caminho_slot(slot));
        ProgressoSalvo salvo;
        std::string detalhe;
        if (arquivo.carregar(salvo, detalhe))
        {
            ++slotsOcupados;
            podeContinuar = podeContinuar || salvo.fasesConcluidas < Campanha::TOTAL_FASES;
        }
    }
    const std::string detalhe = slotsOcupados == 0
                                    ? "NENHUM JOGO SALVO"
                                    : std::to_string(slotsOcupados) + " SLOT(S) OCUPADO(S)";
    this->menu.configurar_inicial(podeContinuar, detalhe, telaJogar);
    this->estado = EstadoJogo::INICIAL;
    this->atirando = false;
    this->mostrarMinimapa = false;
    if (!this->savePendente)
    {
        this->mensagemSave.clear();
    }
}

void Jogo::abrir_selecaoSlots(bool novoJogo)
{
    std::array<std::string, QUANTIDADE_SLOTS_SAVE> detalhes;
    std::array<bool, QUANTIDADE_SLOTS_SAVE> disponiveis = {};
    for (int indice = 0; indice < QUANTIDADE_SLOTS_SAVE; ++indice)
    {
        ArquivoProgresso arquivo(caminho_slot(indice + 1));
        ProgressoSalvo salvo;
        std::string mensagem;
        const bool valido = arquivo.carregar(salvo, mensagem);
        disponiveis[indice] = valido && salvo.fasesConcluidas < Campanha::TOTAL_FASES;
        if (!valido)
        {
            detalhes[indice] = mensagem == "NENHUM JOGO SALVO" ? "VAZIO" : mensagem;
        }
        else if (salvo.fasesConcluidas == Campanha::TOTAL_FASES)
        {
            detalhes[indice] =
                "CAMPANHA CONCLUIDA - PONTOS " + std::to_string(salvo.pontuacaoTotal);
        }
        else
        {
            detalhes[indice] = "FASE " + std::to_string(salvo.fasesConcluidas + 1) + "/" +
                               std::to_string(Campanha::TOTAL_FASES) + " - VIDAS " +
                               std::to_string(salvo.vidas) + " - PONTOS " +
                               std::to_string(salvo.pontuacaoTotal) + " - CONTINUE " +
                               (salvo.continueDisponivel ? "SIM" : "NAO");
        }
    }
    this->selecionandoNovoJogo = novoJogo;
    this->menu.configurar_slots(novoJogo, detalhes, disponiveis);
}

void Jogo::iniciar_partida(bool continuar, int slot)
{
    if (slot < 1 || slot > QUANTIDADE_SLOTS_SAVE)
    {
        return;
    }
    this->slotAtivo = slot;
    this->progresso = ArquivoProgresso(caminho_slot(slot));
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
    this->vidas = salvo.vidas;
    this->continueDisponivel = salvo.continueDisponivel;
    this->pontuacaoTotal = salvo.pontuacaoTotal;
    this->pontosComunsFase = 0;
    this->pontosBossFase = 0;
    this->pontosComunsObservados = 0;
    this->pontosBossObservados = 0;
    this->tempoFase = 0.0f;
    this->cheatTiros = false;
    this->progressoCheat = 0;
    this->tempoMensagemCheat = 0.0f;
    this->campanha.iniciar(salvo.fasesConcluidas, this->jogador, do_save(salvo.armas));
    this->savePendente = false;
    this->mensagemSave.clear();
    if (!continuar)
    {
        this->salvar_progresso();
    }
    this->preparar_entrada();
    if (this->vidas == 0)
    {
        this->iniciar_gameOver();
    }
    else
    {
        this->estado = EstadoJogo::JOGANDO;
    }
}

void Jogo::salvar_progresso()
{
    // O checkpoint guarda as armas com que a proxima fase comeca.
    ProgressoSalvo salvo;
    salvo.fasesConcluidas = this->campanha.get_fasesConcluidas();
    salvo.armas = para_save(this->campanha.get_armasInicioFase());
    salvo.vidas = this->vidas;
    salvo.continueDisponivel = this->continueDisponivel;
    salvo.pontuacaoTotal = this->pontuacaoTotal;
    this->savePendente = !this->progresso.salvar(salvo, this->mensagemSave);
    Plataforma::persistir_saves();
}

void Jogo::concluir_fase()
{
    this->resumoFase.numero = this->campanha.get_fasesConcluidas() + 1;
    this->resumoFase.pontosComuns = this->pontosComunsFase;
    this->resumoFase.pontosBoss = this->pontosBossFase;
    this->resumoFase.tempo = std::max(0.1f, this->tempoFase);
    this->resumoFase.pontosRapidez = static_cast<int>(
        std::lround(PONTOS_RAPIDEZ_BASE * TEMPO_REFERENCIA_FASE / this->resumoFase.tempo));
    Som::tocar(EfeitoSonoro::FASE_CONCLUIDA);
    this->resumoFase.total = this->resumoFase.pontosComuns + this->resumoFase.pontosBoss +
                             this->resumoFase.pontosRapidez;
    this->pontuacaoTotal += this->resumoFase.total;
    this->campanha.avancar_fase(this->jogador);
    this->pontosComunsFase = 0;
    this->pontosBossFase = 0;
    this->pontosComunsObservados = 0;
    this->pontosBossObservados = 0;
    this->tempoFase = 0.0f;
    this->salvar_progresso();
    this->atirando = false;
    this->inicioResumo = SDL_GetTicks();
    this->estado = EstadoJogo::RESUMO_FASE;
}

void Jogo::iniciar_derrota()
{
    // Perde a vida e grava ja; a fase so e recriada quando a animacao terminar,
    // para a invasao acontecer sobre a cena congelada do momento da queda.
    Som::tocar(EfeitoSonoro::MORTE_JOGADOR);
    this->vidas = std::max(0, this->vidas - 1);
    this->salvar_progresso();
    this->atirando = false;
    this->mostrarMinimapa = false;
    this->derrota.iniciar(this->vidas, static_cast<int>(SDL_GetTicks() % 100000));
    this->estado = EstadoJogo::DERROTA;
}

void Jogo::concluir_derrota()
{
    this->campanha.reiniciar_fase(this->jogador);
    this->preparar_entrada();
    if (this->vidas == 0)
    {
        this->iniciar_gameOver();
    }
    else
    {
        this->estado = EstadoJogo::JOGANDO;
    }
}

void Jogo::iniciar_gameOver()
{
    Som::tocar(EfeitoSonoro::GAME_OVER);
    this->tempoGameOver = 10.0f;
    this->mostrarMinimapa = false;
    this->atirando = false;
    this->estado = EstadoJogo::GAME_OVER;
}

void Jogo::aceitar_continue()
{
    if (!this->continueDisponivel)
    {
        return;
    }
    Som::tocar(EfeitoSonoro::CONTINUE);
    this->continueDisponivel = false;
    this->vidas = VIDAS_INICIAIS;
    this->campanha.reiniciar_fase(this->jogador);
    this->tempoFase = 0.0f;
    this->preparar_entrada();
    this->salvar_progresso();
    this->estado = EstadoJogo::JOGANDO;
}

void Jogo::encerrar_gameOver()
{
    std::string mensagem;
    const bool apagado = this->progresso.apagar(mensagem);
    Plataforma::persistir_saves();
    this->savePendente = false;
    this->abrir_inicio();
    if (!apagado)
    {
        this->mensagemSave = mensagem;
    }
}

void Jogo::registrar_cheat(SDL_Keycode tecla)
{
    int digito = -1;
    if (tecla == SDLK_6 || tecla == SDLK_KP_6)
    {
        digito = 6;
    }
    else if (tecla == SDLK_7 || tecla == SDLK_KP_7)
    {
        digito = 7;
    }
    if (digito < 0)
    {
        this->progressoCheat = 0;
        return;
    }
    constexpr int CODIGO[] = {6, 7, 6, 7};
    if (digito == CODIGO[this->progressoCheat])
    {
        ++this->progressoCheat;
    }
    else
    {
        this->progressoCheat = digito == 6 ? 1 : 0;
    }
    if (this->progressoCheat == 4)
    {
        this->cheatTiros = true;
        Som::tocar(EfeitoSonoro::CHEAT);
        this->jogador.set_ignorarTirosInimigos(true);
        this->tempoMensagemCheat = 3.0f;
        this->progressoCheat = 0;
    }
}

void Jogo::atualizar_pontuacaoInimigos()
{
    const auto &fase = this->campanha.get_faseAtual();
    const int comunsAtuais = fase.get_pontosComuns();
    const int bossAtuais = fase.get_pontosBoss();
    if (comunsAtuais > this->pontosComunsObservados)
    {
        this->pontosComunsFase += comunsAtuais - this->pontosComunsObservados;
    }
    if (bossAtuais > this->pontosBossObservados)
    {
        this->pontosBossFase += bossAtuais - this->pontosBossObservados;
    }
    this->pontosComunsObservados = comunsAtuais;
    this->pontosBossObservados = bossAtuais;
}

void Jogo::preparar_entrada()
{
    this->mostrarMinimapa = false;
    this->tempoTituloFase = DURACAO_TITULO_FASE;
    this->atirando = false;
    this->esperaTiro = 0.0f;
    this->pontosComunsObservados = this->campanha.get_faseAtual().get_pontosComuns();
    this->pontosBossObservados = this->campanha.get_faseAtual().get_pontosBoss();
    this->jogador.set_ignorarTirosInimigos(this->cheatTiros);
    this->atualizar_camera();
    this->atualizar_mira();
}
