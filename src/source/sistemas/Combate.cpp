#include "sistemas/Combate.h"

#include <cmath>

#include "config/Config.h"

int Combate::get_inimigosVivos() const
{
    int vivos = 0;

    for (const Inimigo &inimigo : this->inimigos)
    {
        if (inimigo.get_vivo())
        {
            vivos++;
        }
    }
    return vivos;
}

void Combate::reiniciar(const Jogador &jogador)
{
    this->inimigos.limpar();
    this->projeteis.limpar();

    // Nascem espalhados num circulo em volta do jogador.
    for (int i = 0; i < QUANTIDADE_INIMIGOS; i++)
    {
        const float angulo = (2.0f * static_cast<float>(M_PI) * i) / QUANTIDADE_INIMIGOS;

        const float centroX = jogador.get_centroX() + std::cos(angulo) * RAIO_SURGIMENTO;
        const float centroY = jogador.get_centroY() + std::sin(angulo) * RAIO_SURGIMENTO;

        this->inimigos.inserir(
            Inimigo(centroX - LADO_INIMIGO / 2.0f, centroY - LADO_INIMIGO / 2.0f));
    }
}

void Combate::disparar(Jogador &jogador)
{
    Armas *arma = jogador.get_armaEquipada();

    if (arma == nullptr || !arma->atirar())
    {
        return; // sem arma na fila ou sem municao
    }

    const SDL_FPoint boca = jogador.get_bocaArma();
    const SDL_FPoint direcao = jogador.get_direcaoMira();

    this->projeteis.inserir(Projetil(boca.x, boca.y, direcao.x, direcao.y, arma->get_dano()));
}

void Combate::atualizar_projeteis(float dt)
{
    for (Projetil &projetil : this->projeteis)
    {
        projetil.atualizar(dt);
    }
}

void Combate::atualizar_inimigos(float dt, const Jogador &jogador)
{
    const float alvoX = jogador.get_centroX();
    const float alvoY = jogador.get_centroY();

    for (Inimigo &inimigo : this->inimigos)
    {
        inimigo.perseguir(alvoX, alvoY, dt);
    }

    // Sem isso os cinco se empilham num quadrado so ao chegar no jogador.
    for (std::size_t a = 0; a < this->inimigos.get_quantidade(); a++)
    {
        for (std::size_t b = a + 1; b < this->inimigos.get_quantidade(); b++)
        {
            this->inimigos.obter(a).afastar_de(this->inimigos.obter(b));
        }
    }
}

void Combate::resolver_colisoes()
{
    for (Projetil &projetil : this->projeteis)
    {
        if (!projetil.get_ativo())
        {
            continue;
        }

        const SDL_FRect areaProjetil = projetil.get_retangulo();

        for (Inimigo &inimigo : this->inimigos)
        {
            if (!inimigo.get_vivo())
            {
                continue;
            }

            const SDL_FRect areaInimigo = inimigo.get_retangulo();

            if (SDL_HasIntersectionF(&areaProjetil, &areaInimigo))
            {
                inimigo.levar_dano(projetil.get_dano());
                projetil.desativar();
                break; // cada tiro acerta um inimigo so
            }
        }
    }
}

void Combate::atualizar(float dt, const Jogador &jogador)
{
    this->atualizar_projeteis(dt);
    this->atualizar_inimigos(dt, jogador);
    this->resolver_colisoes();
    this->limpar_inativos();
}

void Combate::limpar_inativos()
{
    this->projeteis.remover_se([](const Projetil &p) { return !p.get_ativo(); });
    this->inimigos.remover_se([](const Inimigo &i) { return !i.get_vivo(); });
}

void Combate::desenhar(SDL_Renderer *renderizador, const Camera &camera) const
{
    for (const Inimigo &inimigo : this->inimigos)
    {
        inimigo.desenhar(renderizador, camera);
    }

    for (const Projetil &projetil : this->projeteis)
    {
        projetil.desenhar(renderizador, camera);
    }
}
