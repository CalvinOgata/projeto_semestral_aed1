#include "mundo/Fase.h"

#include <array>
#include <iterator>
#include <string>
#include "config/Config.h"
#include <stdexcept>

#include "config/CampanhaConfig.h"

namespace
{
// Planta autoral de uma sala. As conexoes sao derivadas exclusivamente dos pais.
struct PlantaSala
{
    const char *nome;
    int coluna, linha;
    int virus, brutos, baus;
    bool boss;
    // 1 quadrantes, 2 trincheira, 3 fortaleza (boss), 4 labirinto, 5 deposito (baus).
    int cobertura;
    int ondasExtras; // reforcos que surgem quando resta no maximo um inimigo
    Afinidade afinidade = Afinidade::NEUTRA;
    bool misturarAfinidades = false;
};

// Fase 1: lobby no topo; A ramifica para B/C/D, C para E e E para o boss F.
const PlantaSala PLANTA_FASE_1[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, 0, 0},
    {"A - PRE-PROCESSADOR", 0, 1, 4, 1, 0, false, 1, 1, Afinidade::AZUL},
    {"B - CACHE DE BUILD", -1, 1, 0, 0, 1, false, 5, 0},
    {"C - ANALISE LEXICA", 1, 1, 5, 2, 0, false, 2, 1, Afinidade::VERMELHA},
    {"D - PARSER", 0, 2, 5, 2, 0, false, 4, 1, Afinidade::VERDE},
    {"E - LINKER", 1, 2, 0, 0, 2, false, 5, 0},
    {"F - WINDOWS UPDATE", 1, 3, 0, 0, 0, true, 3, 0, Afinidade::AZUL},
};

// Fase 2: C leva a E; D leva a F, que ramifica para G e para o boss H.
const PlantaSala PLANTA_FASE_2[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, 0, 0},
    {"A - ROTEADOR", 0, 1, 5, 1, 0, false, 1, 1, Afinidade::VERDE},
    {"B - CACHE DNS", -1, 1, 0, 0, 1, false, 5, 0},
    {"C - CAIXA DE SPAM", 1, 1, 6, 2, 0, false, 2, 1, Afinidade::VERMELHA},
    {"D - PROXY", 0, 2, 5, 2, 0, false, 4, 2, Afinidade::AZUL},
    {"E - BOTNET", 1, 2, 7, 2, 0, false, 2, 1, Afinidade::VERMELHA, true},
    {"F - PING 999", 0, 3, 6, 3, 0, false, 4, 2, Afinidade::VERDE, true},
    {"G - LOOT DROP", 1, 3, 0, 0, 2, false, 5, 0},
    {"H - DATACENTER", 0, 4, 0, 0, 0, true, 3, 0, Afinidade::VERDE},
};

// Fase 3: lobby no topo, ramos B/C e caminho B-D-G-I-J ate o boss.
const PlantaSala PLANTA_FASE_3[] = {
    {"LOBBY", 0, 0, 0, 0, 0, false, 0, 0},
    {"A - KERNEL PANIC", 0, 1, 6, 2, 0, false, 1, 1, Afinidade::AZUL, true},
    {"B - ROOTKIT", -1, 1, 7, 2, 0, false, 4, 2, Afinidade::VERMELHA},
    {"C - RANSOMWARE", 1, 1, 7, 2, 0, false, 2, 2, Afinidade::VERMELHA, true},
    {"D - SPYWARE", -1, 2, 8, 2, 0, false, 2, 2, Afinidade::AZUL, true},
    {"E - WORM", 1, 2, 8, 3, 0, false, 4, 2, Afinidade::VERDE, true},
    {"F - COFRE DE MEMORIA", 2, 1, 0, 0, 1, false, 5, 0},
    {"G - DEADLOCK", 0, 2, 7, 3, 0, false, 1, 2, Afinidade::VERDE, true},
    {"H - SAVE POINT", 2, 2, 0, 0, 2, false, 5, 0},
    {"I - PROCESSOS ZUMBIS", 0, 3, 7, 4, 0, false, 4, 3, Afinidade::VERMELHA, true},
    {"J - WANNACRY PRIME", 0, 4, 3, 0, 0, true, 3, 0, Afinidade::VERMELHA, true},
};

// Fonte unica das ligacoes: portas, corredores e arvore usam estes mesmos pais.
// A raiz (lobby) e a unica sala sem pai.
constexpr int PAIS_FASE_1[] = {-1, 0, 1, 1, 1, 3, 5};
constexpr int PAIS_FASE_2[] = {-1, 0, 1, 1, 1, 3, 4, 6, 6};
constexpr int PAIS_FASE_3[] = {-1, 0, 1, 1, 2, 3, 3, 4, 5, 7, 9};
static_assert(std::size(PAIS_FASE_1) == std::size(PLANTA_FASE_1));
static_assert(std::size(PAIS_FASE_2) == std::size(PLANTA_FASE_2));
static_assert(std::size(PAIS_FASE_3) == std::size(PLANTA_FASE_3));

// Cada fase mistura um tema de computacao com uma referencia de jogos.
struct PlantaFase
{
    const char *nome;
    const char *subtitulo;
    const PlantaSala *salas;
    std::size_t quantidade;
    const int *pais;
};

const PlantaFase PLANTAS[] = {
    {"FASE DOS COMPILADORES", "SPEEDRUN DO BUILD", PLANTA_FASE_1,
     std::size(PLANTA_FASE_1), PAIS_FASE_1},
    {"FASE DA REDE", "MULTIPLAYER COM LAG", PLANTA_FASE_2,
     std::size(PLANTA_FASE_2), PAIS_FASE_2},
    {"FASE DO KERNEL", "GAME OVER EM TELA AZUL", PLANTA_FASE_3,
     std::size(PLANTA_FASE_3), PAIS_FASE_3},
};
static_assert(std::size(PLANTAS) == QUANTIDADE_FASES, "Cada fase precisa de uma planta");

std::vector<std::array<int, 4>> construir_conexoes(const PlantaFase &planta)
{
    if (planta.quantidade == 0 || planta.pais[0] != -1)
    {
        throw std::logic_error("A planta precisa de um lobby como unica raiz");
    }
    std::vector<std::array<int, 4>> conexoes(planta.quantidade, {{-1, -1, -1, -1}});
    for (std::size_t filho = 1; filho < planta.quantidade; ++filho)
    {
        const int pai = planta.pais[filho];
        // Pais anteriores aos filhos garantem conectividade com a raiz e ausencia de ciclos.
        if (pai < 0 || static_cast<std::size_t>(pai) >= filho)
        {
            throw std::logic_error("Pai invalido ou ciclo na planta da fase");
        }
        const auto &sala = planta.salas[filho];
        for (std::size_t anterior = 0; anterior < filho; ++anterior)
        {
            if (sala.coluna == planta.salas[anterior].coluna &&
                sala.linha == planta.salas[anterior].linha)
            {
                throw std::logic_error("Salas sobrepostas na planta da fase");
            }
        }
        const int dx = sala.coluna - planta.salas[pai].coluna;
        const int dy = sala.linha - planta.salas[pai].linha;
        int direcao = -1;
        if (dx == -1 && dy == 0) direcao = static_cast<int>(Direcao::ESQUERDA);
        if (dx == 1 && dy == 0) direcao = static_cast<int>(Direcao::DIREITA);
        if (dx == 0 && dy == -1) direcao = static_cast<int>(Direcao::CIMA);
        if (dx == 0 && dy == 1) direcao = static_cast<int>(Direcao::BAIXO);
        if (direcao < 0)
        {
            throw std::logic_error("Pai e filho precisam compartilhar um corredor reto");
        }
        const int oposta = direcao ^ 1;
        if (conexoes[pai][direcao] != -1 || conexoes[filho][oposta] != -1)
        {
            throw std::logic_error("Duas conexoes ocupam a mesma porta");
        }
        conexoes[pai][direcao] = static_cast<int>(filho);
        conexoes[filho][oposta] = pai;
    }
    return conexoes;
}
}

Fase::Fase(int numero) : numero(numero)
{
    if (numero < 1 || numero > QUANTIDADE_FASES)
    {
        throw std::invalid_argument("Numero de fase invalido");
    }
    const PlantaFase &planta = PLANTAS[numero - 1];
    const auto conexoes = construir_conexoes(planta);
    const float largura = 936, altura = 516;
    int indicePiso = numero == 1 ? 0 : (numero == 2 ? 5 : 12);
    for (std::size_t i = 0; i < planta.quantidade; ++i)
    {
        const PlantaSala &sala = planta.salas[i];
        ConfiguracaoSala config{sala.nome, largura, altura, numero, sala.virus, {true}};
        config.brutos = sala.brutos;
        config.boss = sala.boss;
        config.portalSaida = sala.boss;
        config.baus = sala.baus;
        config.armasIniciais = false;
        config.vizinhos = conexoes[i];
        config.mapaRamificado = true;
        config.tema = numero;
        config.afinidadePrincipal = sala.afinidade;
        config.misturarAfinidades = sala.misturarAfinidades;
        config.cobertura = sala.cobertura;
        config.ondasExtras = sala.ondasExtras;
        config.indicePiso = sala.baus > 0 ? 21 : indicePiso++;
        salas.inserir(Sala(config));
        origens.inserir({sala.coluna * (largura + Sala::COMPRIMENTO_CORREDOR),
                        sala.linha * (altura + Sala::COMPRIMENTO_CORREDOR)});
    }
    // A arvore nasce somente com o lobby. As outras salas entram quando suas
    // portas forem liberadas pela gameplay.
    arvoreProgresso.definir_raiz(0);
}

void Fase::reiniciar(Jogador &jogador, const FilaArmas &armas)
{
    // Reiniciar executa explicitamente a operacao primitiva de limpeza. O
    // construtor chamado em seguida define novamente apenas a raiz.
    this->arvoreProgresso.limpar();
    *this = Fase(this->numero);
    jogador = Jogador();
    // Inventario vazio receberia uma fase sem como lutar; mantem a arma inicial.
    if (!armas.vazio())
    {
        jogador.get_armas() = armas;
    }
    this->get_salaAtual().entrar(jogador);
}

bool Fase::atualizar_transicao(Jogador &jogador)
{
    Sala &atual = this->get_salaAtual();
    if (this->concluida || !atual.liberada())
    {
        return false;
    }
    preparar_adjacentes();
    if (atual.tocando_portal(jogador))
    {
        atual.sair();
        concluida = true;
        return true;
    }
    for (int d = 0; d < 4; ++d)
    {
        const auto direcao = static_cast<Direcao>(d);
        const int destino = get_vizinho(indiceAtual, direcao);
        if (destino < 0 || !atual.porta_liberada(direcao)) continue;
        if (!consultar_transicaoArvore(indiceAtual, static_cast<std::size_t>(destino))) continue;
        const auto &config = atual.get_configuracao();
        const float comprimento = Sala::COMPRIMENTO_CORREDOR;
        const bool chegou = d == 0 ? jogador.get_x() <= -comprimento - jogador.get_largura() :
                            d == 1 ? jogador.get_x() >= config.largura + comprimento :
                            d == 2 ? jogador.get_y() <= -comprimento - jogador.get_altura() :
                                     jogador.get_y() >= config.altura + comprimento;
        if (!chegou) continue;
        const auto origem = origens.obter(indiceAtual), alvo = origens.obter(destino);
        const float x = jogador.get_x() + origem.x - alvo.x;
        const float y = jogador.get_y() + origem.y - alvo.y;
        atual.sair();
        indiceAtual = static_cast<std::size_t>(destino);
        get_salaAtual().ativar();
        jogador.set_posicao(x, y);
        get_salaAtual().limitar_jogador(jogador);
        preparar_adjacentes();
        return true;
    }
    return false;
}

void Fase::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool hitboxes) const
{
    const auto origem = origens.obter(indiceAtual);
    for (int camada = 0; camada < 2; ++camada)
    {
        for (std::size_t i = 0; i < salas.get_quantidade(); ++i)
        {
            const auto pos = origens.obter(i);
            Camera local;
            local.centralizar_em(camera.get_x() + origem.x - pos.x + LARGURA_TELA / 2.0f,
                                 camera.get_y() + origem.y - pos.y + ALTURA_TELA / 2.0f);
            if (camada == 0)
            {
                salas.obter(i).desenhar_corredores(renderizador, local);
            }
            else
            {
                salas.obter(i).desenhar(renderizador, local, i > 0, hitboxes);
            }
        }
    }
}

SDL_FRect Fase::get_limitesVisuais() const
{
    // A camera considera somente a sala atual e os corredores que realmente
    // saem dela. Assim ramos situados a esquerda/direita em outras partes da
    // fase nao deslocam a sala atual para um canto vazio da tela.
    const auto &config = get_salaAtual().get_configuracao();
    const float margem = 12.0f;
    const float esquerda = get_vizinho(indiceAtual, Direcao::ESQUERDA) >= 0 ?
        -Sala::COMPRIMENTO_CORREDOR - margem : -margem;
    const float direita = get_vizinho(indiceAtual, Direcao::DIREITA) >= 0 ?
        config.largura + Sala::COMPRIMENTO_CORREDOR + margem : config.largura + margem;
    const float cima = get_vizinho(indiceAtual, Direcao::CIMA) >= 0 ?
        -Sala::COMPRIMENTO_CORREDOR - margem : -margem;
    const float baixo = get_vizinho(indiceAtual, Direcao::BAIXO) >= 0 ?
        config.altura + Sala::COMPRIMENTO_CORREDOR + margem : config.altura + margem;
    return {esquerda, cima, direita - esquerda, baixo - cima};
}

Sala &Fase::get_salaAtual()
{
    return this->salas.obter(this->indiceAtual);
}

const Sala &Fase::get_salaAtual() const
{
    return this->salas.obter(this->indiceAtual);
}

const char *Fase::get_nome() const
{
    return PLANTAS[numero - 1].nome;
}

const char *Fase::get_subtitulo() const
{
    return PLANTAS[numero - 1].subtitulo;
}

std::size_t Fase::get_indiceAtual() const
{
    return this->indiceAtual;
}

bool Fase::get_concluida() const
{
    return this->concluida;
}

std::size_t Fase::get_quantidadeSalas() const
{
    return salas.get_quantidade();
}

const Sala &Fase::get_sala(std::size_t indice) const
{
    return salas.obter(indice);
}

SDL_FPoint Fase::get_origemSala(std::size_t indice) const
{
    return origens.obter(indice);
}

int Fase::get_vizinho(std::size_t indice, Direcao direcao) const
{
    const auto &config = get_sala(indice).get_configuracao();
    if (config.mapaRamificado)
    {
        return config.vizinhos[static_cast<int>(direcao)];
    }
    if (direcao == Direcao::ESQUERDA && indice > 0)
    {
        return static_cast<int>(indice) - 1;
    }
    if (direcao == Direcao::DIREITA && indice + 1 < get_quantidadeSalas())
    {
        return static_cast<int>(indice) + 1;
    }
    return -1;
}

int Fase::get_paiSalaDescoberta(std::size_t indice) const
{
    const auto *pai = arvoreProgresso.obter_pai(indice);
    return pai == nullptr ? -1 : static_cast<int>(*pai);
}

std::vector<std::size_t> Fase::percorrer_salasDescobertas() const
{
    return arvoreProgresso.percorrer_pre_ordem();
}

std::size_t Fase::contar_salasDescobertas() const
{
    return arvoreProgresso.contar_nos();
}

int Fase::get_pontosComuns() const
{
    int total = 0;
    for (const auto &sala : salas)
    {
        total += sala.get_pontosComuns();
    }
    return total;
}

int Fase::get_pontosBoss() const
{
    int total = 0;
    for (const auto &sala : salas)
    {
        total += sala.get_pontosBoss();
    }
    return total;
}

bool Fase::consultar_transicaoArvore(std::size_t origem, std::size_t destino) const
{
    // Avanco para uma sala liberada: consulta direta de filho.
    if (arvoreProgresso.obter_filho(origem, destino) != nullptr)
    {
        return true;
    }
    // Retorno pelo caminho de descoberta: consulta do pai.
    const auto *pai = arvoreProgresso.obter_pai(origem);
    if (pai != nullptr && *pai == destino)
    {
        return true;
    }
    // Destino conhecido nao basta: somente uma aresta pai-filho permite passagem.
    return false;
}

void Fase::atualizar(float dt, Jogador &jogador)
{
    for (std::size_t i = 0; i < salas.get_quantidade(); ++i)
    {
        if (i == indiceAtual) salas.obter(i).atualizar(dt, jogador);
        else salas.obter(i).envelhecer_coletas(dt);
    }
    preparar_adjacentes();
}

void Fase::preparar_adjacentes()
{
    if (!get_salaAtual().liberada()) return;
    const PlantaFase &planta = PLANTAS[numero - 1];
    for (std::size_t indiceFilho = 1; indiceFilho < planta.quantidade; ++indiceFilho)
    {
        if (planta.pais[indiceFilho] != static_cast<int>(indiceAtual))
        {
            continue;
        }
        // A insercao continua acontecendo em tempo real, mas somente o pai
        // canonico pode executa-la. Assim a ordem de exploracao nao altera a
        // forma final da arvore.
        if (!arvoreProgresso.contem(indiceFilho))
        {
            arvoreProgresso.inserir_filho(indiceAtual, indiceFilho);
        }
        salas.obter(indiceFilho).preparar();
    }
}
