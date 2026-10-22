#include "sistemas/Combate.h"

#include <algorithm>
#include <cmath>
#include <random>
#include <vector>

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
                        int tema, int ondasExtras)
{
    this->limites = area;
    this->inimigos.limpar();
    this->obstaculos.limpar();
    this->avisos.limpar();
    coracoes.limpar();
    this->limpar_projeteis();
    this->dificuldade = std::max(1, dificuldade);
    this->tema = tema;
    this->temPosicaoAnterior = false;
    const int virus = std::max(0, quantidade);
    brutos = std::max(0, brutos);
    // Reforcos menores que a primeira onda, mas com pelo menos dois virus.
    this->ondasRestantes = std::max(0, ondasExtras);
    this->reforcoVirus = std::max(2, (virus * 2 + 2) / 3);
    this->reforcoBrutos = (brutos + 1) / 2;
    quantidade = virus + brutos + (boss ? 1 : 0);
    // Distribui os inimigos na metade direita, longe da entrada a esquerda.
    const int colunas = std::max(1, static_cast<int>(std::ceil(std::sqrt(quantidade))));
    const int linhas = std::max(1, (quantidade + colunas - 1) / colunas);
    for (int i = 0; i < quantidade; ++i)
    {
        const float x = area.x + area.w * (0.5f + 0.4f * (i % colunas + 1) / (colunas + 1));
        const float y = area.y + area.h * (i / colunas + 1) / (linhas + 1);
        const auto tipo = i < virus ? TipoInimigo::VIRUS :
            (i < virus + brutos ? TipoInimigo::BRUTO : TipoInimigo::BOSS);
        // Mantem a posicao historica: (x, y) era o canto da hitbox.
        const auto &dimensoes = tipo == TipoInimigo::VIRUS ? DIMENSOES_INIMIGO_COMUM :
            (tipo == TipoInimigo::BRUTO ? DIMENSOES_GUARDA_COSTAS : DIMENSOES_BOSS);
        this->criar_inimigo(tipo, x + dimensoes.larguraHitbox / 2.0f, y + dimensoes.alturaHitbox / 2.0f);
    }
}

void Combate::criar_inimigo(TipoInimigo tipo, float centroX, float centroY)
{
    const auto dimensoes = tipo == TipoInimigo::VIRUS ? DIMENSOES_INIMIGO_COMUM :
        (tipo == TipoInimigo::BRUTO ? DIMENSOES_GUARDA_COSTAS : DIMENSOES_BOSS);
    // Vida cresce mais nos inimigos fortes para as fases finais exigirem mais.
    const float fator = static_cast<float>(dificuldade - 1);
    const int vida = tipo == TipoInimigo::VIRUS ? VIDA_INIMIGO + dificuldade - 1 :
        static_cast<int>(std::lround(tipo == TipoInimigo::BRUTO ? 12 * (1 + 0.25f * fator) :
                                                                  65 * (1 + 0.3f * fator)));
    Inimigo inimigo(centroX - dimensoes.larguraHitbox / 2.0f, centroY - dimensoes.alturaHitbox / 2.0f,
                   vida, VELOCIDADE_INIMIGO * (tipo == TipoInimigo::VIRUS ? 1.0f : 0.65f) *
                   (1.0f + 0.12f * fator), dimensoes, tipo, tema, dificuldade);
    inimigo.limitar_a(limites);
    this->inimigos.inserir(inimigo);
}

bool Combate::concluido() const
{
    return this->get_inimigosVivos() == 0 && this->ondasRestantes == 0 && this->avisos.vazio();
}

void Combate::agendar_onda(const Jogador &jogador)
{
    static std::mt19937 sorteio(std::random_device{}());
    // Pontos candidatos longe do jogador e sem cobertura em cima.
    std::vector<SDL_FPoint> pontos;
    for (int coluna = 0; coluna < 7; ++coluna)
    {
        for (int linha = 0; linha < 4; ++linha)
        {
            const SDL_FPoint p = {limites.x + 90 + coluna * (limites.w - 180) / 6,
                                  limites.y + 80 + linha * (limites.h - 160) / 3};
            const SDL_FRect area = {p.x - 32, p.y - 32, 64, 64};
            if (std::hypot(p.x - jogador.get_centroX(), p.y - jogador.get_centroY()) >= 260 &&
                !this->tiro_bloqueado(area))
            {
                pontos.push_back(p);
            }
        }
    }
    if (pontos.empty())
    {
        pontos.push_back({limites.x + limites.w - 90, limites.y + limites.h / 2});
    }
    std::shuffle(pontos.begin(), pontos.end(), sorteio);
    const int total = reforcoVirus + reforcoBrutos;
    for (int i = 0; i < total; ++i)
    {
        // Se faltarem pontos, repete com um pequeno desvio.
        const SDL_FPoint p = pontos[i % pontos.size()];
        const float desvio = static_cast<float>(i / pontos.size()) * 36;
        avisos.inserir({p.x + desvio, p.y, 1.0f, 1.0f,
                        i < reforcoVirus ? TipoInimigo::VIRUS : TipoInimigo::BRUTO});
    }
}

void Combate::atualizar_avisos(float dt)
{
    for (auto &aviso : avisos)
    {
        aviso.tempo -= dt;
        if (aviso.tempo > 0)
        {
            continue;
        }
        this->criar_inimigo(aviso.tipo, aviso.x, aviso.y);
        Inimigo &novo = this->inimigos.obter(this->inimigos.get_quantidade() - 1);
        for (const Obstaculo &obstaculo : this->obstaculos)
        {
            const SDL_FPoint deslocamento = obstaculo.separar(novo.get_retanguloVisual());
            novo.deslocar(deslocamento.x, deslocamento.y);
        }
        novo.limitar_a(this->limites);
    }
    avisos.remover_se([](const Aviso &aviso) { return aviso.tempo <= 0; });
}

int Combate::get_lacaiosVivos() const
{
    int vivos = 0;
    for (const Inimigo &inimigo : this->inimigos)
    {
        vivos += inimigo.get_vivo() && inimigo.get_tipo() == TipoInimigo::VIRUS;
    }
    return vivos + static_cast<int>(this->avisos.get_quantidade());
}

void Combate::invocar_lacaios(const Inimigo &boss)
{
    // Dois virus surgem aos lados do boss, dentro da sala.
    for (int lado = -1; lado <= 1; lado += 2)
    {
        const float x = std::clamp(boss.get_centroX() + lado * 100.0f, limites.x + 40,
                                   limites.x + limites.w - 40);
        const float y = std::clamp(boss.get_centroY(), limites.y + 40, limites.y + limites.h - 40);
        avisos.inserir({x, y, 0.8f, 0.8f, TipoInimigo::VIRUS});
    }
}

bool Combate::linha_livre(SDL_FPoint origem, SDL_FPoint destino) const
{
    for (const Obstaculo &obstaculo : this->obstaculos)
    {
        if (!obstaculo.get_intacto())
        {
            continue;
        }
        // Peca ampliada pela metade do tiro (8x8): a linha so e livre se o tiro passa.
        SDL_FRect area = obstaculo.get_retangulo();
        area = {area.x - 4, area.y - 4, area.w + 8, area.h + 8};
        float x1 = origem.x, y1 = origem.y, x2 = destino.x, y2 = destino.y;
        if (SDL_IntersectFRectAndLine(&area, &x1, &y1, &x2, &y2))
        {
            return false;
        }
    }
    return true;
}

void Combate::disparar_hostil(SDL_FPoint origem, float angulo, float velocidade, int dano)
{
    tirosInimigos.inserir({origem.x, origem.y, std::cos(angulo), std::sin(angulo), dano, true,
                           velocidade});
}

void Combate::estimar_velocidadeJogador(float dt, const Jogador &jogador)
{
    const SDL_FPoint atual = {jogador.get_centroX(), jogador.get_centroY()};
    if (temPosicaoAnterior && dt > 0)
    {
        SDL_FPoint medida = {(atual.x - posicaoJogadorAnterior.x) / dt,
                             (atual.y - posicaoJogadorAnterior.y) / dt};
        // Saltos (troca de sala ou reinicio) nao contam como movimento.
        if (std::hypot(medida.x, medida.y) > 600)
        {
            medida = {0, 0};
        }
        velocidadeJogador = {velocidadeJogador.x * 0.6f + medida.x * 0.4f,
                             velocidadeJogador.y * 0.6f + medida.y * 0.4f};
    }
    posicaoJogadorAnterior = atual;
    temPosicaoAnterior = true;
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
    // Cada ataque danifica so a peca mais proxima entre os raios que a tocam.
    int pecaAtingida = -1;
    float distanciaPeca = 0;
    for (int i = 0; i < raios; ++i)
    {
        const float a = angulo + (espada ? (i - 4) * static_cast<float>(M_PI) / 16 : 0);
        float x1 = origem.x, y1 = origem.y;
        float x2 = x1 + std::cos(a) * (espada ? 80 : 1600);
        float y2 = y1 + std::sin(a) * (espada ? 80 : 1600);
        SDL_IntersectFRectAndLine(&limites, &x1, &y1, &x2, &y2);
        // O primeiro obstaculo no caminho interrompe o raio.
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
        if (pecaMaisProxima >= 0 && (pecaAtingida < 0 || menorDistancia < distanciaPeca))
        {
            pecaAtingida = pecaMaisProxima;
            distanciaPeca = menorDistancia;
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
    if (pecaAtingida >= 0)
    {
        obstaculos.obter(static_cast<std::size_t>(pecaAtingida)).levar_dano(arma.get_dano());
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

        // Um contato atinge so um bloco, mesmo sobre a emenda entre dois.
        for (Obstaculo &obstaculo : this->obstaculos)
        {
            if (obstaculo.get_intacto() && projetil.colidir_obstaculo(obstaculo.get_retangulo()))
            {
                obstaculo.levar_dano(projetil.get_dano());
                break;
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
    this->estimar_velocidadeJogador(dt, jogador);
    // A visao e recalculada por quadro; inimigos sem linha livre contornam a cobertura.
    const SDL_FPoint alvo = {jogador.get_centroX(), jogador.get_centroY()};
    for (Inimigo &inimigo : this->inimigos)
    {
        if (inimigo.get_vivo())
        {
            inimigo.set_visaoLivre(this->linha_livre({inimigo.get_centroX(), inimigo.get_centroY()}, alvo));
        }
    }
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
    if (this->ondasRestantes > 0 && this->avisos.vazio() && this->get_inimigosVivos() <= 1)
    {
        this->agendar_onda(jogador);
        --this->ondasRestantes;
    }
    this->atualizar_avisos(dt);
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
    for (const auto &aviso : avisos)
    {
        // Moldura que se fecha sobre o ponto ate o inimigo aparecer.
        const float progresso = 1 - aviso.tempo / aviso.duracao;
        const auto pos = camera.para_tela(aviso.x, aviso.y);
        const float meio = (aviso.tipo == TipoInimigo::VIRUS ? 24.0f : 32.0f) * (1.6f - 0.6f * progresso);
        const Uint8 alfa = static_cast<Uint8>(170 + 85 * (static_cast<int>(progresso * 10) % 2));
        const SDL_FRect fundo = {pos.x - meio, pos.y - meio, 2 * meio, 2 * meio};
        SDL_SetRenderDrawColor(renderizador, 255, 40, 100, static_cast<Uint8>(40 + 50 * progresso));
        SDL_RenderFillRectF(renderizador, &fundo);
        SDL_SetRenderDrawColor(renderizador, 255, 90, 140, alfa);
        for (int borda = 0; borda < 3; ++borda)
        {
            const SDL_FRect moldura = {pos.x - meio + borda, pos.y - meio + borda,
                                       2 * (meio - borda), 2 * (meio - borda)};
            SDL_RenderDrawRectF(renderizador, &moldura);
        }
        SDL_RenderDrawLineF(renderizador, pos.x - 8, pos.y - 8, pos.x + 8, pos.y + 8);
        SDL_RenderDrawLineF(renderizador, pos.x - 8, pos.y + 8, pos.x + 8, pos.y - 8);
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
    const float velocidadeTiro = 230 * (1 + 0.08f * (dificuldade - 1));
    for (auto &inimigo : inimigos)
    {
        if (!inimigo.preparar_tiro(dt)) continue;
        const int ataque = inimigo.registrar_ataque();
        const auto tipo = inimigo.get_tipo();
        const SDL_FPoint origem = {inimigo.get_centroX(), inimigo.get_centroY()};
        // Mira antecipada: preve onde o jogador estara quando o tiro chegar.
        const float antecipacao = tipo == TipoInimigo::VIRUS ? 0.3f :
            (tipo == TipoInimigo::BRUTO ? 0.6f : 0.75f);
        const float tempo = std::hypot(jogador.get_centroX() - origem.x,
                                       jogador.get_centroY() - origem.y) / velocidadeTiro;
        const float alvoX = jogador.get_centroX() + velocidadeJogador.x * tempo * antecipacao;
        const float alvoY = jogador.get_centroY() + velocidadeJogador.y * tempo * antecipacao;
        const float angulo = std::atan2(alvoY - origem.y, alvoX - origem.x);
        const int dano = inimigo.get_danoTiro();
        if (tipo == TipoInimigo::BOSS)
        {
            // Padrao do boss: invoca lacaios, anel giratorio ou leque mirado.
            if (ataque % 5 == 4 && this->get_lacaiosVivos() < 4)
            {
                this->invocar_lacaios(inimigo);
            }
            else if (ataque % 4 == 3)
            {
                for (int i = 0; i < 12; ++i)
                    disparar_hostil(origem, i * static_cast<float>(M_PI) / 6 + ataque * 0.13f,
                                    velocidadeTiro * 0.75f, dano);
            }
            else
            {
                for (int i = -1; i <= 1; ++i)
                    disparar_hostil(origem, angulo + i * 0.21f, velocidadeTiro, dano);
            }
        }
        else if (tipo == TipoInimigo::BRUTO && ataque % 3 == 2)
        {
            // A cada tres ataques o bruta-monte abre um leque para cortar a fuga.
            for (int i = -1; i <= 1; ++i)
                disparar_hostil(origem, angulo + i * 0.3f, velocidadeTiro, dano);
        }
        else
        {
            disparar_hostil(origem, angulo, velocidadeTiro, dano);
        }
    }
    auto corpo = jogador.get_retangulo();
    for (auto &tiro : tirosInimigos)
    {
        tiro.x += tiro.dx * tiro.velocidade * dt;
        tiro.y += tiro.dy * tiro.velocidade * dt;
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
