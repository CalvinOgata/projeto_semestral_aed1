#include "mundo/Sala.h"

#include <algorithm>
#include <cmath>
#include <random>
#include "interface/Texto.h"
#include <stdexcept>

#include "config/Config.h"
#include "graficos/IconeArma.h"
#include "graficos/Ambiente.h"

namespace
{
// Pecas autorais em uma sala de referencia de 936x516. Evitam a cruz central,
// que liga as portas e recebe portal e baus, e ficam longe das paredes.
struct PecaCobertura
{
    TipoObstaculo tipo;
    float x, y;
    bool vertical;
};

constexpr auto C = TipoObstaculo::CAIXOTE;
constexpr auto B = TipoObstaculo::BARREIRA;
const PecaCobertura COBERTURA_QUADRANTES[] = {
    {C, 234, 129, false}, {C, 274, 129, false}, {C, 702, 387, false}, {C, 662, 387, false},
    {B, 702, 129, false}, {B, 234, 387, false},
};
const PecaCobertura COBERTURA_TRINCHEIRA[] = {
    {B, 300, 120, true}, {B, 300, 396, true}, {B, 640, 120, true}, {B, 640, 396, true},
    {C, 170, 110, false}, {C, 770, 406, false},
};
// Na sala do boss o lado direito fica livre para ele nascer e se mover.
const PecaCobertura COBERTURA_FORTALEZA[] = {
    {B, 234, 150, false}, {B, 234, 366, false}, {B, 380, 130, true}, {B, 380, 386, true},
    {C, 840, 90, false}, {C, 840, 426, false},
};
const PecaCobertura COBERTURA_LABIRINTO[] = {
    {C, 190, 110, false}, {C, 230, 110, false}, {C, 190, 150, false},
    {C, 746, 406, false}, {C, 706, 406, false}, {C, 746, 366, false},
    {B, 600, 124, true}, {B, 336, 392, true}, {C, 380, 130, false}, {C, 556, 386, false},
};
const PecaCobertura COBERTURA_DEPOSITO[] = {
    {C, 110, 90, false}, {C, 150, 90, false}, {C, 826, 90, false},
    {C, 110, 426, false}, {C, 826, 426, false}, {C, 786, 426, false},
};

template <std::size_t N>
void adicionar_pecas(Combate &combate, const PecaCobertura (&pecas)[N], float escalaX, float escalaY)
{
    for (const auto &peca : pecas)
    {
        const float x = peca.x * escalaX, y = peca.y * escalaY;
        if (peca.tipo == TipoObstaculo::CAIXOTE)
        {
            combate.adicionar_obstaculo(Obstaculo(peca.tipo, x, y));
            continue;
        }
        // A barreira vira uma fileira de bloquinhos centrada no ponto autoral.
        for (int i = 0; i < BLOCOS_POR_BARREIRA; ++i)
        {
            const float desvio = (i - (BLOCOS_POR_BARREIRA - 1) / 2.0f) * LADO_BLOCO_BARREIRA;
            combate.adicionar_obstaculo(Obstaculo(peca.tipo, x + (peca.vertical ? 0 : desvio),
                                                  y + (peca.vertical ? desvio : 0)));
        }
    }
}
}

Sala::Sala(const ConfiguracaoSala &configuracao) : configuracao(configuracao)
{
    // Mantem espaco para entrada, portas e entidades sem clamp com limites invertidos.
    if (configuracao.largura < 400 || configuracao.altura < 300 ||
        configuracao.dificuldade < 1 || configuracao.quantidadeInimigos < 0)
    {
        throw std::invalid_argument("Configuracao de sala invalida");
    }
}

void Sala::preparar()
{
    if (!preparada)
    {
        this->combate.reiniciar(this->get_limites(), this->configuracao.quantidadeInimigos,
                               this->configuracao.dificuldade, configuracao.brutos, configuracao.boss,
                               configuracao.tema);
        const float escalaX = configuracao.largura / 936, escalaY = configuracao.altura / 516;
        switch (configuracao.cobertura)
        {
        case 1: adicionar_pecas(combate, COBERTURA_QUADRANTES, escalaX, escalaY); break;
        case 2: adicionar_pecas(combate, COBERTURA_TRINCHEIRA, escalaX, escalaY); break;
        case 3: adicionar_pecas(combate, COBERTURA_FORTALEZA, escalaX, escalaY); break;
        case 4: adicionar_pecas(combate, COBERTURA_LABIRINTO, escalaX, escalaY); break;
        case 5: adicionar_pecas(combate, COBERTURA_DEPOSITO, escalaX, escalaY); break;
        default: break;
        }
        const TipoArma tipos[] = {TipoArma::SHOTGUN, TipoArma::LASER, TipoArma::DUPLA,
            TipoArma::RICOCHETE, TipoArma::ESPADA, TipoArma::ARCO, TipoArma::BOOMERANG};
        for (int i = 0; configuracao.armasIniciais && i < 7; ++i)
            armasChao.inserir({Armas(tipos[i]), 160.0f + (i % 2) * 130.0f,
                              configuracao.altura * (i / 2 + 1) / 5.0f, false});
        for (int i = 0; i < configuracao.baus; ++i)
            baus.inserir({configuracao.largura * (i + 1) / (configuracao.baus + 1),
                         configuracao.altura / 2, false});
        preparada = true;
    }
}

void Sala::ativar()
{
    preparar();
    if (estado == EstadoSala::NAO_VISITADA)
    {
        estado = configuracao.requisito.eliminarInimigos && combate.get_inimigosVivos() > 0
                     ? EstadoSala::EM_COMBATE : EstadoSala::LIBERADA;
    }
}

void Sala::entrar(Jogador &jogador, bool pelaDireita)
{
    ativar();
    jogador.set_posicao(pelaDireita ? this->configuracao.largura - 90 - jogador.get_largura() : 90,
                       (this->configuracao.altura - jogador.get_altura()) / 2.0f);
    this->limitar_jogador(jogador);
    this->atualizar(0.0f, jogador);
}

void Sala::sair()
{
    // Tiros nao ficam suspensos para acertar o jogador/inimigos ao voltar.
    this->combate.limpar_projeteis();
}

void Sala::atualizar(float dt, Jogador &jogador)
{
    if (this->estado == EstadoSala::NAO_VISITADA)
    {
        return;
    }
    this->combate.atualizar(dt, jogador);
    static std::mt19937 sorteio(std::random_device{}());
    const TipoArma tipos[] = {TipoArma::PISTOLA, TipoArma::SHOTGUN, TipoArma::LASER,
        TipoArma::DUPLA, TipoArma::RICOCHETE, TipoArma::ESPADA, TipoArma::ARCO, TipoArma::BOOMERANG};
    for (auto &bau : baus)
    {
        if (!bau.aberto && std::hypot(jogador.get_centroX() - bau.x,
                                     jogador.get_centroY() - bau.y) < 64)
        {
            bau.aberto = true;
            for (int i = 0; i < 2; ++i)
                armasChao.inserir({Armas(tipos[std::uniform_int_distribution<int>(0, 7)(sorteio)]),
                                  bau.x + (i == 0 ? -40 : 40), bau.y + 48, false});
        }
    }
    if (this->estado == EstadoSala::EM_COMBATE)
    {
        const bool eliminados = !this->configuracao.requisito.eliminarInimigos ||
                                this->combate.get_inimigosVivos() == 0;
        if (eliminados)
        {
            this->estado = EstadoSala::LIBERADA;
        }
    }
}

bool Sala::buscar_alvoMaisProximo(const Jogador &jogador, SDL_FPoint &alvo) const
{
    return this->combate.buscar_alvoMaisProximo(jogador, alvo);
}

void Sala::disparar(Jogador &jogador)
{
    this->combate.disparar(jogador);
}

void Sala::limitar_jogador(Jogador &jogador) const
{
    const float x = jogador.get_x(), y = jogador.get_y();
    const float w = jogador.get_largura(), h = jogador.get_altura();
    float melhorX = std::clamp(x, 0.0f, configuracao.largura - w);
    float melhorY = std::clamp(y, 0.0f, configuracao.altura - h);
    float menorDistancia = (x - melhorX) * (x - melhorX) + (y - melhorY) * (y - melhorY);

    // Retangulos de posicoes validas do canto da hitbox. A passagem inclui a
    // faixa em que o corpo ocupa sala e corredor ao mesmo tempo, nos dois sentidos.
    for (int d = 0; d < 4; ++d)
    {
        if (!porta_liberada(static_cast<Direcao>(d))) continue;
        const auto c = get_corredor(static_cast<Direcao>(d));
        float minimoX = c.x, maximoX = c.x + c.w - w;
        float minimoY = c.y, maximoY = c.y + c.h - h;
        // A folga no extremo conserva o ultimo passo ate a troca de coordenadas.
        if (d == 0) { minimoX -= 2 * w; maximoX = 0; }
        if (d == 1) { minimoX -= w; maximoX += 2 * w; }
        if (d == 2) { minimoY -= 2 * h; maximoY = 0; }
        if (d == 3) { minimoY -= h; maximoY += 2 * h; }
        if (minimoX > maximoX || minimoY > maximoY) continue;
        const float candidatoX = std::clamp(x, minimoX, maximoX);
        const float candidatoY = std::clamp(y, minimoY, maximoY);
        const float distancia = (x - candidatoX) * (x - candidatoX) +
                                (y - candidatoY) * (y - candidatoY);
        if (distancia < menorDistancia)
        {
            melhorX = candidatoX;
            melhorY = candidatoY;
            menorDistancia = distancia;
        }
    }
    jogador.set_posicao(melhorX, melhorY);
    this->combate.afastar_jogador(jogador);
}

SDL_FRect Sala::get_limites() const
{
    return {0, 0, this->configuracao.largura, this->configuracao.altura};
}

SDL_FRect Sala::get_porta(bool direita) const
{
    return {direita ? this->configuracao.largura - 18 : 0,
            this->configuracao.altura / 2.0f - 48, 18, 96};
}

bool Sala::tocando_porta(const Jogador &jogador, bool direita) const
{
    const SDL_FRect area = this->get_porta(direita);
    const SDL_FRect corpo = jogador.get_retangulo();
    return corpo.x <= area.x + area.w && corpo.x + corpo.w >= area.x &&
           corpo.y <= area.y + area.h && corpo.y + corpo.h >= area.y;
}

bool Sala::liberada() const
{
    return this->estado == EstadoSala::LIBERADA;
}

EstadoSala Sala::get_estado() const
{
    return this->estado;
}

int Sala::get_inimigosVivos() const
{
    return this->combate.get_inimigosVivos();
}

const ConfiguracaoSala &Sala::get_configuracao() const
{
    return this->configuracao;
}

void Sala::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool temAnterior,
                    bool mostrarHitboxes) const
{
    const SDL_FRect piso = camera.para_tela(this->get_limites());
    (void)temAnterior;
    Ambiente::desenhar_piso(renderizador, piso);
    Ambiente::desenhar_paredes(renderizador, piso);
    for (int d = 0; d < 4; ++d)
    {
        if (configuracao.vizinhos[d] < 0) continue;
        const auto direcao = static_cast<Direcao>(d);
        auto porta = camera.para_tela(get_porta(direcao));
        if (d == 0) { porta.x -= 12; porta.w += 12; }
        if (d == 1) porta.w += 12;
        if (d == 2) { porta.y -= 12; porta.h += 12; }
        if (d == 3) porta.h += 12;
        Ambiente::desenhar_porta(renderizador, porta,
                                porta_liberada(direcao));
    }
    if (configuracao.portalSaida && liberada())
    {
        const auto centro = camera.para_tela(configuracao.largura / 2, configuracao.altura / 2);
        Ambiente::desenhar_portal(renderizador, centro.x, centro.y);
        Texto::desenhar_centralizado(renderizador, "PROXIMA FASE", centro.x, centro.y + 44,
                                    1, {220, 204, 240, 255});
    }
    for (const auto &bau : baus)
    {
        const auto pos = camera.para_tela(bau.x, bau.y);
        auto bloco = [&](float x, float y, float w, float h, SDL_Color cor) {
            SDL_SetRenderDrawColor(renderizador, cor.r, cor.g, cor.b, cor.a);
            const SDL_FRect ret = {pos.x + x, pos.y + y, w, h};
            SDL_RenderFillRectF(renderizador, &ret);
        };
        bloco(-32, 18, 64, 8, {40,38,48,255});
        bloco(-30, -16, 60, 40, {48,27,30,255});
        bloco(-26, -12, 52, 32, {139,65,40,255});
        bloco(-24, 2, 48, 3, {78,35,30,255});
        bloco(-30, bau.aberto ? -30 : -20, 60, 18, {202,117,51,255});
        bloco(-26, bau.aberto ? -27 : -17, 52, 4, {246,183,84,255});
        if (bau.aberto) bloco(-23, -12, 46, 10, {28,23,34,255});
        bloco(-22, -16, 6, 38, {235,181,72,255});
        bloco(16, -16, 6, 38, {235,181,72,255});
        bloco(-7, -5, 14, 14, {255,218,107,255});
        bloco(-2, -1, 4, 7, {66,43,35,255});
    }
    for (const auto &item : armasChao)
    {
        if (item.coletada)
        {
            continue;
        }
        const auto pos = camera.para_tela(item.x, item.y);
        const SDL_FRect area = {pos.x - 18, pos.y - 18, 36, 36};
        IconeArma::desenhar(renderizador, item.arma.get_tipo(), area);
        Texto::desenhar_centralizado(renderizador, item.arma.get_nome(), pos.x, pos.y + 22,
                                    1, {255, 240, 180, 255});
    }
    this->combate.desenhar(renderizador, camera, mostrarHitboxes);
}

bool Sala::coletar_arma(Jogador &jogador)
{
    ArmaChao *maisProxima = nullptr;
    float distancia = 54;
    for (auto &item : armasChao)
    {
        const float atual = std::hypot(item.x - jogador.get_centroX(), item.y - jogador.get_centroY());
        if (!item.coletada && atual < distancia)
        {
            maisProxima = &item;
            distancia = atual;
        }
    }
    if (!maisProxima || !jogador.get_armas().inserir(maisProxima->arma))
    {
        return false;
    }
    maisProxima->coletada = true;
    return true;
}

void Sala::soltar_arma(Jogador &jogador)
{
    Armas arma;
    if (jogador.get_armas().retirar(arma))
        armasChao.inserir({arma, jogador.get_centroX(), jogador.get_centroY(), false});
}

void Sala::entrar(Jogador &jogador, Direcao entrada)
{
    entrar(jogador, entrada == Direcao::DIREITA);
    if (entrada == Direcao::CIMA || entrada == Direcao::BAIXO)
        jogador.set_posicao((configuracao.largura - jogador.get_largura()) / 2,
            entrada == Direcao::CIMA ? 90 : configuracao.altura - 90 - jogador.get_altura());
}
bool Sala::porta_liberada(Direcao direcao) const
{
    const int destino = configuracao.vizinhos[static_cast<int>(direcao)];
    return estado != EstadoSala::EM_COMBATE &&
           (!configuracao.mapaRamificado || destino >= 0);
}
SDL_FRect Sala::get_porta(Direcao direcao) const
{
    if (direcao == Direcao::ESQUERDA || direcao == Direcao::DIREITA)
        return get_porta(direcao == Direcao::DIREITA);
    return {configuracao.largura / 2 - 48,
            direcao == Direcao::CIMA ? 0 : configuracao.altura - 18, 96, 18};
}
bool Sala::tocando_porta(const Jogador &jogador, Direcao direcao) const
{
    const auto porta = get_porta(direcao), corpo = jogador.get_retangulo();
    return SDL_HasIntersectionF(&porta, &corpo);
}
bool Sala::tocando_portal(const Jogador &jogador) const
{
    return configuracao.portalSaida && liberada() &&
        std::hypot(jogador.get_centroX() - configuracao.largura / 2,
                   jogador.get_centroY() - configuracao.altura / 2) < 35;
}

void Sala::envelhecer_coletas(float dt) { combate.envelhecer_coracoes(dt); }

SDL_FRect Sala::get_corredor(Direcao direcao) const
{
    if (direcao == Direcao::ESQUERDA || direcao == Direcao::DIREITA)
        return {direcao == Direcao::ESQUERDA ? -COMPRIMENTO_CORREDOR : configuracao.largura,
                configuracao.altura / 2 - 48, COMPRIMENTO_CORREDOR, 96};
    return {configuracao.largura / 2 - 48,
            direcao == Direcao::CIMA ? -COMPRIMENTO_CORREDOR : configuracao.altura,
            96, COMPRIMENTO_CORREDOR};
}

void Sala::desenhar_corredores(SDL_Renderer *renderizador, const Camera &camera) const
{
    // Passagens sao desenhadas antes da sala para preservar o acabamento do piso.
    for (int d = 0; d < 4; ++d)
    {
        if (configuracao.vizinhos[d] < 0) continue;
        const auto corredor = camera.para_tela(get_corredor(static_cast<Direcao>(d)));
        Ambiente::desenhar_piso(renderizador, corredor);
        Ambiente::desenhar_paredes(renderizador, corredor);
    }
}
