#include "sistemas/Combate.h"

#include <algorithm>
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

void Combate::reiniciar(const SDL_FRect &area, int quantidade, int dificuldade)
{
    this->limites = area;
    this->inimigos.limpar();
    this->limpar_projeteis();
    quantidade = std::max(0, quantidade);
    dificuldade = std::max(1, dificuldade);
    // Distribui os inimigos na metade direita, longe da entrada a esquerda.
    const int colunas = std::max(1, static_cast<int>(std::ceil(std::sqrt(quantidade))));
    const int linhas = std::max(1, (quantidade + colunas - 1) / colunas);
    for (int i = 0; i < quantidade; ++i)
    {
        const float x = area.x + area.w * (0.5f + 0.4f * (i % colunas + 1) / (colunas + 1));
        const float y = area.y + area.h * (i / colunas + 1) / (linhas + 1);
        Inimigo inimigo(x, y, VIDA_INIMIGO + dificuldade - 1,
                       VELOCIDADE_INIMIGO * (1.0f + 0.12f * (dificuldade - 1)));
        inimigo.limitar_a(area);
        this->inimigos.inserir(inimigo);
    }
}

void Combate::limpar_projeteis()
{
    this->projeteis.limpar();
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
        const SDL_FRect area = projetil.get_retangulo();
        if (area.x < this->limites.x || area.y < this->limites.y ||
            area.x + area.w > this->limites.x + this->limites.w ||
            area.y + area.h > this->limites.y + this->limites.h)
        {
            projetil.desativar();
        }
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
    for (Inimigo &inimigo : this->inimigos)
    {
        inimigo.limitar_a(this->limites);
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

void Combate::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitboxes) const
{
    for (const Inimigo &inimigo : this->inimigos)
    {
        inimigo.desenhar(renderizador, camera, mostrarHitboxes);
    }

    for (const Projetil &projetil : this->projeteis)
    {
        projetil.desenhar(renderizador, camera);
    }
}
