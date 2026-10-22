#ifndef COMBATE_H
#define COMBATE_H

#include "entidades/Inimigo.h"
#include "entidades/Jogador.h"
#include "entidades/Obstaculo.h"
#include "entidades/Projetil.h"
#include "estruturas/ListaSequencial.h"

// Dono das entidades temporarias e das regras de combate.
// As listas cuidam do armazenamento; esta classe decide quando inserir/remover.
class Combate
{
private:
    SDL_FRect limites = {0, 0, 800, 400};
    int dificuldade = 1;
    int tema = 1;
    // Reforcos: cada onda extra surge quando resta no maximo um inimigo vivo.
    int ondasRestantes = 0;
    int reforcoVirus = 0;
    int reforcoBrutos = 0;
    // Marca piscante no ponto de surgimento, para o jogador poder reagir.
    struct Aviso { float x, y, tempo, duracao; TipoInimigo tipo; };
    ListaSequencial<Aviso> avisos;
    // Velocidade estimada do jogador, usada na mira antecipada.
    SDL_FPoint posicaoJogadorAnterior = {0, 0};
    bool temPosicaoAnterior = false;
    SDL_FPoint velocidadeJogador = {0, 0};
    ListaSequencial<Projetil> projeteis;
    ListaSequencial<Inimigo> inimigos;
    // Quebrados continuam na lista, inativos, ate a sala ser recriada.
    ListaSequencial<Obstaculo> obstaculos;
    struct TiroInimigo { float x, y, dx, dy; int dano; bool ativo = true; float velocidade = 230; };
    struct Coracao { float x, y, tempo = 10; };
    ListaSequencial<TiroInimigo> tirosInimigos;
    ListaSequencial<Coracao> coracoes;
    void atualizar_hostis(float dt, Jogador &jogador);
    void atualizar_coracoes(float dt, Jogador &jogador);

    struct Rastro
    {
        SDL_FPoint inicio, fim;
        float tempo;
        TipoArma tipo;
    };
    ListaSequencial<Rastro> rastros;
    void atacar_instantaneo(const Jogador &jogador, const Armas &arma);
    void atualizar_projeteis(float dt, const Jogador &jogador);
    void atualizar_inimigos(float dt, const Jogador &jogador);
    void resolver_colisoes();
    // No posicionamento inicial usa o quadro visual, para nenhum sprite nascer sobre a peca.
    void afastar_inimigos_de_obstaculos(bool pelaArte = false);
    void criar_inimigo(TipoInimigo tipo, float centroX, float centroY);
    void agendar_onda(const Jogador &jogador);
    void atualizar_avisos(float dt);
    void invocar_lacaios(const Inimigo &boss);
    int get_lacaiosVivos() const;
    bool linha_livre(SDL_FPoint origem, SDL_FPoint destino) const;
    void disparar_hostil(SDL_FPoint origem, float angulo, float velocidade, int dano);
    void estimar_velocidadeJogador(float dt, const Jogador &jogador);
    bool tiro_bloqueado(const SDL_FRect &area) const;
    void limpar_inativos();

public:
    bool buscar_alvoMaisProximo(const Jogador &jogador, SDL_FPoint &alvo) const;
    void reiniciar(const SDL_FRect &area, int quantidade, int dificuldade, int brutos = 0,
                   bool boss = false, int tema = 1, int ondasExtras = 0);
    // Sem inimigos vivos, ondas pendentes ou surgimentos anunciados.
    bool concluido() const;
    void limpar_projeteis();
    // Chamado apos reiniciar; empurra inimigos que nasceram sobre a peca.
    void adicionar_obstaculo(const Obstaculo &obstaculo);
    void afastar_jogador(Jogador &jogador) const;
    void envelhecer_coracoes(float dt);
    void disparar(Jogador &jogador);
    void atualizar(float dt, Jogador &jogador);
    void desenhar(SDL_Renderer *renderizador, const Camera &camera,
                  bool mostrarHitboxes = false) const;
    int get_inimigosVivos() const;
};

#endif
