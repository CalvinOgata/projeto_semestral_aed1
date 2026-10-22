#include "sistemas/Combate.h"

#include <algorithm>
#include <cmath>

#include "config/Config.h"

bool Combate::buscar_alvoMaisProximo(const Jogador &jogador, SDL_FPoint &alvo) const
{
    bool encontrado = false;
    float menorDistancia = 0.0f;
    for (const Inimigo &inimigo : this->inimigos)
    {
        if (!inimigo.get_vivo())
            continue;

        const float dx = inimigo.get_centroX() - jogador.get_centroX();
        const float dy = inimigo.get_centroY() - jogador.get_centroY();
        const float distancia = dx * dx + dy * dy;
        if (!encontrado || distancia < menorDistancia)
        {
            menorDistancia = distancia;
            alvo = {inimigo.get_centroX(), inimigo.get_centroY()};
            encontrado = true;
        }
    }
    return encontrado;
}

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

void Combate::reiniciar(const SDL_FRect &area, int quantidade, int dificuldade, int brutos, bool boss,
                        int tema)
{
    this->limites = area;
    this->inimigos.limpar();
    this->obstaculos.limpar();
    coracoes.limpar();
    this->limpar_projeteis();
    const int virus = std::max(0, quantidade);
    quantidade = virus + std::max(0, brutos) + (boss ? 1 : 0);
    dificuldade = std::max(1, dificuldade);
    // Distribui os inimigos na metade direita, longe da entrada a esquerda.
    const int colunas = std::max(1, static_cast<int>(std::ceil(std::sqrt(quantidade))));
    const int linhas = std::max(1, (quantidade + colunas - 1) / colunas);
    for (int i = 0; i < quantidade; ++i)
    {
        const float x = area.x + area.w * (0.5f + 0.4f * (i % colunas + 1) / (colunas + 1));
        const float y = area.y + area.h * (i / colunas + 1) / (linhas + 1);
        const auto tipo = i < virus ? TipoInimigo::VIRUS :
            (i < virus + brutos ? TipoInimigo::BRUTO : TipoInimigo::BOSS);
        const auto dimensoes = tipo == TipoInimigo::VIRUS ? DIMENSOES_INIMIGO_COMUM :
            (tipo == TipoInimigo::BRUTO ? DIMENSOES_GUARDA_COSTAS : DIMENSOES_BOSS);
        const int vida = tipo == TipoInimigo::VIRUS ? VIDA_INIMIGO :
            (tipo == TipoInimigo::BRUTO ? 12 : 65);
        Inimigo inimigo(x, y, vida + dificuldade - 1,
                       VELOCIDADE_INIMIGO * (tipo == TipoInimigo::VIRUS ? 1.0f : 0.65f) *
                       (1.0f + 0.12f * (dificuldade - 1)), dimensoes, tipo, tema);
        inimigo.limitar_a(area);
        this->inimigos.inserir(inimigo);
    }
}

void Combate::adicionar_obstaculo(const Obstaculo &obstaculo)
{
    this->obstaculos.inserir(obstaculo);
    this->afastar_inimigos_de_obstaculos(true);
}

void Combate::afastar_inimigos_de_obstaculos(bool pelaArte)
{
    for (Inimigo &inimigo : this->inimigos)
    {
        for (const Obstaculo &obstaculo : this->obstaculos)
        {
            const SDL_FPoint deslocamento = obstaculo.separar(
                pelaArte ? inimigo.get_retanguloVisual() : inimigo.get_retangulo());
            inimigo.deslocar(deslocamento.x, deslocamento.y);
        }
        inimigo.limitar_a(this->limites);
    }
}

void Combate::afastar_jogador(Jogador &jogador) const
{
    for (const Obstaculo &obstaculo : this->obstaculos)
    {
        const SDL_FPoint deslocamento = obstaculo.separar(jogador.get_retangulo());
        if (deslocamento.x != 0 || deslocamento.y != 0)
        {
            jogador.set_posicao(jogador.get_x() + deslocamento.x, jogador.get_y() + deslocamento.y);
        }
    }
}

bool Combate::tiro_bloqueado(const SDL_FRect &area) const
{
    for (const Obstaculo &obstaculo : this->obstaculos)
    {
        const SDL_FRect peca = obstaculo.get_retangulo();
        if (obstaculo.get_intacto() && SDL_HasIntersectionF(&area, &peca))
        {
            return true;
        }
    }
    return false;
}

void Combate::limpar_projeteis()
{
    this->projeteis.limpar();
    tirosInimigos.limpar();
    this->rastros.limpar();
}

void Combate::disparar(Jogador &jogador)
{
    Armas *arma = jogador.get_armaEquipada();

    if (arma == nullptr || !arma->atirar())
    {
        return; // sem arma na fila ou sem municao
    }

    if (arma->get_tipo() == TipoArma::LASER || arma->get_tipo() == TipoArma::ESPADA)
    {
        atacar_instantaneo(jogador, *arma);
        return;
    }
    const SDL_FPoint direcao = jogador.get_direcaoMira();
    const float angulo = std::atan2(direcao.y, direcao.x);
    const int quantidade = arma->get_quantidadeDisparos();
    for (int i = 0; i < quantidade; ++i)
    {
        float desvio = 0;
        if (arma->get_tipo() == TipoArma::SHOTGUN)
        {
            desvio = (i - 1) * static_cast<float>(M_PI) / 12;
        }
        if (arma->get_tipo() == TipoArma::DUPLA)
        {
            desvio = i * static_cast<float>(M_PI);
        }
        const float dx = std::cos(angulo + desvio), dy = std::sin(angulo + desvio);
        // Origem no corpo evita nascer alem da parede ao encostar nela.
        projeteis.inserir(Projetil(jogador.get_centroX(), jogador.get_centroY(), dx, dy,
                                  arma->get_dano(), arma->get_tipo()));
    }
}

void Combate::atacar_instantaneo(const Jogador &jogador, const Armas &arma)
{
    const bool espada = arma.get_tipo() == TipoArma::ESPADA;
    const auto direcao = jogador.get_direcaoMira();
    const float angulo = std::atan2(direcao.y, direcao.x);
    const SDL_FPoint origem = {jogador.get_centroX(), jogador.get_centroY()};
    // Nove segmentos formam o leque de 90 graus da espada.
    const int raios = espada ? 9 : 1;
    std::vector<bool> atingidos(inimigos.get_quantidade(), false);
    std::vector<bool> pecasAtingidas(obstaculos.get_quantidade(), false);
    for (int i = 0; i < raios; ++i)
    {
        const float a = angulo + (espada ? (i - 4) * static_cast<float>(M_PI) / 16 : 0);
        float x1 = origem.x, y1 = origem.y;
        float x2 = x1 + std::cos(a) * (espada ? 80 : 1600);
        float y2 = y1 + std::sin(a) * (espada ? 80 : 1600);
        SDL_IntersectFRectAndLine(&limites, &x1, &y1, &x2, &y2);
        // O primeiro obstaculo no caminho leva o dano e interrompe o raio.
        int pecaMaisProxima = -1;
        float menorDistancia = 0;
        for (std::size_t j = 0; j < obstaculos.get_quantidade(); ++j)
        {
            if (!obstaculos.obter(j).get_intacto())
            {
                continue;
            }
            auto area = obstaculos.obter(j).get_retangulo();
            float ax = x1, ay = y1, bx = x2, by = y2;
            if (!SDL_IntersectFRectAndLine(&area, &ax, &ay, &bx, &by))
            {
                continue;
            }
            const float distancia = std::hypot(ax - x1, ay - y1);
            if (pecaMaisProxima < 0 || distancia < menorDistancia)
            {
                pecaMaisProxima = static_cast<int>(j);
                menorDistancia = distancia;
                x2 = ax;
                y2 = ay;
            }
        }
        if (pecaMaisProxima >= 0 && !pecasAtingidas[pecaMaisProxima])
        {
            obstaculos.obter(pecaMaisProxima).levar_dano(arma.get_dano());
            pecasAtingidas[pecaMaisProxima] = true;
        }
        rastros.inserir({{x1, y1}, {x2, y2}, 0.12f, arma.get_tipo()});
        for (std::size_t j = 0; j < inimigos.get_quantidade(); ++j)
        {
            auto &inimigo = inimigos.obter(j);
            if (!inimigo.get_vivo() || atingidos[j])
            {
                continue;
            }
            auto area = inimigo.get_retangulo();
            float ax = x1, ay = y1, bx = x2, by = y2;
            if (SDL_IntersectFRectAndLine(&area, &ax, &ay, &bx, &by))
            {
                inimigo.levar_dano(arma.get_dano());
                atingidos[j] = true;
            }
        }
    }
}

void Combate::atualizar_projeteis(float dt, const Jogador &jogador)
{
    for (Projetil &projetil : projeteis)
        projetil.atualizar(dt, limites, {jogador.get_centroX(), jogador.get_centroY()});
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
    this->afastar_inimigos_de_obstaculos();
}

void Combate::resolver_colisoes()
{
    for (Projetil &projetil : this->projeteis)
    {
        if (!projetil.get_ativo())
        {
            continue;
        }

        for (Obstaculo &obstaculo : this->obstaculos)
        {
            if (obstaculo.get_intacto() && projetil.colidir_obstaculo(obstaculo.get_retangulo()))
            {
                obstaculo.levar_dano(projetil.get_dano());
            }
        }
        if (!projetil.get_ativo())
        {
            continue;
        }

        const SDL_FRect areaProjetil = projetil.get_retangulo();

        for (std::size_t indice = 0; indice < inimigos.get_quantidade(); ++indice)
        {
            Inimigo &inimigo = inimigos.obter(indice);
            if (!inimigo.get_vivo())
            {
                continue;
            }

            const SDL_FRect areaInimigo = inimigo.get_retangulo();

            if (SDL_HasIntersectionF(&areaProjetil, &areaInimigo) && projetil.atingir(indice))
            {
                inimigo.levar_dano(projetil.get_dano());
                if (!projetil.get_ativo())
                {
                    break;
                }
            }
        }
    }
}

void Combate::atualizar(float dt, Jogador &jogador)
{
    // Passos pequenos evitam atravessar inimigos ou paredes entre quadros.
    float restante = std::max(0.0f, dt);
    while (restante > 0)
    {
        const float passo = std::min(restante, 1.0f / 240.0f);
        atualizar_projeteis(passo, jogador);
        atualizar_inimigos(passo, jogador);
        resolver_colisoes();
        jogador.atualizar_defesa(passo);
        atualizar_hostis(passo, jogador);
        restante -= passo;
    }
    for (auto &rastro : rastros)
    {
        rastro.tempo -= dt;
    }
    rastros.remover_se([](const Rastro &r) { return r.tempo <= 0; });
    atualizar_coracoes(dt, jogador);
    this->limpar_inativos();
}

void Combate::limpar_inativos()
{
    this->projeteis.remover_se([](const Projetil &p) { return !p.get_ativo(); });
    // Preserva indices dos inimigos para registrar acertos do boomerang por trajeto.
}

void Combate::desenhar(SDL_Renderer *renderizador, const Camera &camera, bool mostrarHitboxes) const
{
    for (const Obstaculo &obstaculo : this->obstaculos)
    {
        obstaculo.desenhar(renderizador, camera, mostrarHitboxes);
    }
    for (const auto &tiro : tirosInimigos)
    {
        const auto pos = camera.para_tela(tiro.x, tiro.y);
        const SDL_FRect ret{pos.x - 4, pos.y - 4, 8, 8};
        SDL_SetRenderDrawColor(renderizador, 255, 115, 80, 255);
        SDL_RenderFillRectF(renderizador, &ret);
    }
    for (const auto &coracao : coracoes)
    {
        if (coracao.tempo < 2 && static_cast<int>(coracao.tempo * 8) % 2 == 0) continue;
        const auto pos = camera.para_tela(coracao.x, coracao.y);
        const char *forma[] = {".XX.XX.", "XXXXXXX", "XXXXXXX", ".XXXXX.", "..XXX..", "...X..."};
        for (int y = 0; y < 6; ++y)
            for (int x = 0; x < 7; ++x)
                if (forma[y][x] == 'X')
                {
                    SDL_SetRenderDrawColor(renderizador, 255, y < 2 ? 135 : 65, 105, 255);
                    const SDL_FRect ret{pos.x + x * 3 - 10, pos.y + y * 3 - 9, 3, 3};
                    SDL_RenderFillRectF(renderizador, &ret);
                }
    }
    for (const auto &rastro : rastros)
    {
        const auto a = camera.para_tela(rastro.inicio.x, rastro.inicio.y);
        const auto b = camera.para_tela(rastro.fim.x, rastro.fim.y);
        SDL_SetRenderDrawColor(renderizador, 120, 255, rastro.tipo == TipoArma::LASER ? 160 : 255, 255);
        SDL_RenderDrawLineF(renderizador, a.x, a.y, b.x, b.y);
    }
    for (const Inimigo &inimigo : this->inimigos)
    {
        inimigo.desenhar(renderizador, camera, mostrarHitboxes);
    }

    for (const Projetil &projetil : this->projeteis)
    {
        projetil.desenhar(renderizador, camera);
    }
}

void Combate::atualizar_hostis(float dt, Jogador &jogador)
{
    for (auto &inimigo : inimigos)
    {
        if (!inimigo.preparar_tiro(dt)) continue;
        const float dx = jogador.get_centroX() - inimigo.get_centroX();
        const float dy = jogador.get_centroY() - inimigo.get_centroY();
        const float tamanho = std::max(0.001f, std::hypot(dx, dy));
        tirosInimigos.inserir({inimigo.get_centroX(), inimigo.get_centroY(),
                              dx / tamanho, dy / tamanho, inimigo.get_danoTiro(), true});
    }
    auto corpo = jogador.get_retangulo();
    for (auto &tiro : tirosInimigos)
    {
        tiro.x += tiro.dx * 230 * dt;
        tiro.y += tiro.dy * 230 * dt;
        const SDL_FRect ret{tiro.x - 4, tiro.y - 4, 8, 8};
        if (this->tiro_bloqueado(ret))
        {
            tiro.ativo = false; // a cobertura absorve sem quebrar
        }
        else if (SDL_HasIntersectionF(&ret, &corpo))
        {
            jogador.receber_dano(tiro.dano);
            tiro.ativo = false;
        }
        if (tiro.x < limites.x || tiro.x > limites.x + limites.w ||
            tiro.y < limites.y || tiro.y > limites.y + limites.h) tiro.ativo = false;
    }
    tirosInimigos.remover_se([](const TiroInimigo &tiro) { return !tiro.ativo; });
}
void Combate::atualizar_coracoes(float dt, Jogador &jogador)
{
    const auto corpo = jogador.get_retangulo();
    for (auto &coracao : coracoes)
    {
        coracao.tempo -= dt;
        const SDL_FRect area{coracao.x - 10, coracao.y - 9, 21, 18};
        if (coracao.tempo > 0 && !jogador.morto() &&
            jogador.get_vida() < jogador.get_vidaMax() && SDL_HasIntersectionF(&corpo, &area))
        {
            jogador.rec_vida(10);
            coracao.tempo = 0;
        }
    }
    coracoes.remover_se([](const Coracao &c) { return c.tempo <= 0; });
    for (auto &inimigo : inimigos)
        if (inimigo.soltar_coracao())
            coracoes.inserir({inimigo.get_centroX(), inimigo.get_centroY(), 10});
}

void Combate::envelhecer_coracoes(float dt)
{
    for (auto &coracao : coracoes) coracao.tempo -= dt;
    coracoes.remover_se([](const Coracao &c) { return c.tempo <= 0; });
}
