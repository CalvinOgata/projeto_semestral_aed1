#include "mundo/Fase.h"

#include <cmath>
#include <string>

#include "config/Config.h"
#include "interface/Texto.h"

Fase::Fase()
{
    // Fase autoral inicial. Cada registro pode ter tamanho, dificuldade e requisito proprios.
    this->salas.inserir(Sala({"SETOR DE BOOT", 800, 400, 1, 4, {true, 0}}));
    this->salas.inserir(Sala({"REDE INFECTADA", 1120, 640, 2, 7, {true, 0}}));
    this->salas.inserir(Sala({"NUCLEO EM QUARENTENA", 1440, 800, 3, 10, {true, 20}}));
}

void Fase::reiniciar(Jogador &jogador)
{
    *this = Fase();
    jogador = Jogador();
    this->get_salaAtual().entrar(jogador);
}

bool Fase::interagir(Jogador &jogador)
{
    Sala &atual = this->get_salaAtual();
    if (this->concluida || !atual.liberada())
    {
        return false;
    }
    const bool direita = atual.perto_porta(jogador, true);
    const bool esquerda = this->indiceAtual > 0 && atual.perto_porta(jogador, false);
    if (!direita && !esquerda)
    {
        return false;
    }
    atual.sair();
    if (direita && this->indiceAtual + 1 == this->salas.get_quantidade())
    {
        this->concluida = true;
        return true;
    }
    if (direita)
    {
        ++this->indiceAtual;
    }
    else
    {
        --this->indiceAtual;
    }
    this->get_salaAtual().entrar(jogador, esquerda);
    return true;
}

Sala &Fase::get_salaAtual()
{
    return this->salas.obter(this->indiceAtual);
}

const Sala &Fase::get_salaAtual() const
{
    return this->salas.obter(this->indiceAtual);
}

std::size_t Fase::get_indiceAtual() const
{
    return this->indiceAtual;
}

bool Fase::get_concluida() const
{
    return this->concluida;
}

void Fase::desenhar_painel(SDL_Renderer *renderizador, const Jogador &jogador) const
{
    const Sala &sala = this->get_salaAtual();
    const auto &config = sala.get_configuracao();
    SDL_SetRenderDrawColor(renderizador, 18, 22, 30, 235);
    const SDL_Rect painel = {0, 0, LARGURA_TELA, 62};
    SDL_RenderFillRect(renderizador, &painel);
    const std::string titulo = "SALA " + std::to_string(this->indiceAtual + 1) + "/" +
        std::to_string(this->salas.get_quantidade()) + " - " + config.nome +
        " | DIFICULDADE " + std::to_string(config.dificuldade);
    Texto::desenhar(renderizador, titulo, 16, 10, 2, COR_TEXTO);
    const auto *arma = jogador.get_armas().frente();
    std::string objetivo = "INIMIGOS: " + std::to_string(sala.get_inimigosVivos()) +
        " | MUNICAO: " + std::to_string(arma ? arma->get_municao() : 0);
    if (sala.get_tempoRestante() > 0)
    {
        objetivo += " | TEMPO MINIMO: " +
            std::to_string(static_cast<int>(std::ceil(sala.get_tempoRestante()))) + "S";
    }
    Texto::desenhar(renderizador, objetivo, 16, 36, 2, COR_TEXTO);
    std::string ajuda = "ELIMINE OS INIMIGOS PARA LIBERAR AS PORTAS | R: REINICIAR";
    if (sala.get_inimigosVivos() == 0 && !sala.liberada())
    {
        ajuda = "AGUARDE O FIM DA QUARENTENA PARA LIBERAR AS PORTAS";
    }
    if (sala.liberada())
    {
        ajuda = "SALA LIBERADA - PORTA DIREITA: AVANCAR | ESQUERDA: VOLTAR";
        if (sala.perto_porta(jogador, true))
        {
            ajuda = this->indiceAtual + 1 == this->salas.get_quantidade() ?
                "E: SAIR E CONCLUIR A FASE" : "E: ENTRAR NA PROXIMA SALA";
        }
        else if (this->indiceAtual > 0 && sala.perto_porta(jogador, false))
        {
            ajuda = "E: VOLTAR A SALA ANTERIOR";
        }
    }
    if (this->concluida)
    {
        ajuda = "FASE CONCLUIDA! R: JOGAR NOVAMENTE";
    }
    const SDL_Rect rodape = {0, ALTURA_TELA - 34, LARGURA_TELA, 34};
    SDL_SetRenderDrawColor(renderizador, 18, 22, 30, 235);
    SDL_RenderFillRect(renderizador, &rodape);
    Texto::desenhar_centralizado(renderizador, ajuda, LARGURA_TELA / 2,
                                 ALTURA_TELA - 24, 2, COR_TEXTO);
}
