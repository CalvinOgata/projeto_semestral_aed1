#ifndef FASE_H
#define FASE_H

#include <vector>

#include "estruturas/Arvore.h"
#include "estruturas/ListaSequencial.h"
#include "mundo/Sala.h"

// Salas e conexoes direcionais. A lista armazena; Fase decide a navegacao.
class Fase
{
private:
    int numero;
    ListaSequencial<Sala> salas;
    std::size_t indiceAtual = 0;
    bool concluida = false;
    ListaSequencial<SDL_FPoint> origens;
    // A planta fisica permanece fixa; esta arvore registra apenas as salas
    // liberadas durante a tentativa atual.
    Arvore<std::size_t> arvoreProgresso;
    void preparar_adjacentes();
    bool consultar_transicaoArvore(std::size_t origem, std::size_t destino) const;

public:
    explicit Fase(int numero = 1);
    void desenhar(SDL_Renderer *renderizador, const Camera &camera, bool hitboxes) const;
    SDL_FRect get_limitesVisuais() const;
    // Recria a fase e o jogador, que recebe o inventario informado.
    void reiniciar(Jogador &jogador, const FilaArmas &armas);
    void atualizar(float dt, Jogador &jogador);
    // Verifica contato com portas liberadas uma vez por atualizacao.
    bool atualizar_transicao(Jogador &jogador);
    Sala &get_salaAtual();
    const Sala &get_salaAtual() const;
    const char *get_nome() const;
    const char *get_subtitulo() const;
    std::size_t get_indiceAtual() const;
    bool get_concluida() const;
    std::size_t get_quantidadeSalas() const;
    const Sala &get_sala(std::size_t indice) const;
    // Copia da origem usada para desenhar a sala e atravessar seus corredores.
    SDL_FPoint get_origemSala(std::size_t indice) const;
    int get_vizinho(std::size_t indice, Direcao direcao) const;
    int get_paiSalaDescoberta(std::size_t indice) const;
    std::vector<std::size_t> percorrer_salasDescobertas() const;
    std::size_t contar_salasDescobertas() const;
    int get_pontosComuns() const;
    int get_pontosBoss() const;
};

#endif
