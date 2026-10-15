#include "interface/Menu.h"

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
const char *OPCOES_PAUSA[Menu::NUM_OPCOES] = {"CONTINUAR", "MENU INICIAL", "SAIR"};
const char *OPCOES_INICIAIS[Menu::NUM_OPCOES] = {"NOVO JOGO", "CONTINUAR", "SAIR"};

constexpr int LARGURA_PAINEL = 700;
constexpr int ALTURA_PAINEL = 370;
constexpr int ALTURA_OPCAO = 44;
constexpr int TOPO_OPCOES = 100; // relativo ao topo do painel
constexpr int ESCALA_TITULO = 5;
constexpr int ESCALA_OPCAO = 3;
constexpr int ESCALA_DICA = 2;
} // namespace

Menu::Menu()
{
    this->reiniciar();
}

Menu::~Menu()
{
}

void Menu::reiniciar()
{
    this->selecionado = 0;
}

int Menu::get_selecionado() const
{
    return this->selecionado;
}

void Menu::mover_selecao(int passo)
{
    // Soma NUM_OPCOES antes do resto para o indice nunca ficar negativo.
    this->selecionado = (this->selecionado + passo + NUM_OPCOES) % NUM_OPCOES;
}

AcaoMenu Menu::acao_da_opcao(int indice) const
{
    if (this->inicial)
    {
        if (indice == 0)
        {
            return AcaoMenu::NOVO_JOGO;
        }
        if (indice == 1)
        {
            return this->podeContinuar ? AcaoMenu::CONTINUAR : AcaoMenu::NENHUMA;
        }
    }
    else
    {
        if (indice == 0)
        {
            return AcaoMenu::CONTINUAR;
        }
        if (indice == 1)
        {
            return AcaoMenu::MENU_INICIAL;
        }
    }
    return AcaoMenu::SAIR;
}

SDL_Rect Menu::retangulo_painel() const
{
    SDL_Rect painel;
    painel.w = LARGURA_PAINEL;
    painel.h = ALTURA_PAINEL;
    painel.x = (LARGURA_TELA - painel.w) / 2;
    painel.y = (ALTURA_TELA - painel.h) / 2;
    return painel;
}

SDL_Rect Menu::retangulo_opcao(int indice) const
{
    const SDL_Rect painel = this->retangulo_painel();

    SDL_Rect opcao;
    opcao.w = painel.w - 60;
    opcao.h = ALTURA_OPCAO;
    opcao.x = painel.x + 30;
    opcao.y = painel.y + TOPO_OPCOES + indice * (ALTURA_OPCAO + 10);
    return opcao;
}

AcaoMenu Menu::tratar_evento(const SDL_Event &evento)
{
    if (evento.type == SDL_KEYDOWN && evento.key.repeat == 0)
    {
        switch (evento.key.keysym.sym)
        {
        case SDLK_UP:
        case SDLK_w:
            this->mover_selecao(-1);
            break;

        case SDLK_DOWN:
        case SDLK_s:
            this->mover_selecao(1);
            break;

        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            return this->acao_da_opcao(this->selecionado);

        case SDLK_ESCAPE:
            return this->inicial ? AcaoMenu::NENHUMA : AcaoMenu::CONTINUAR;

        default:
            break;
        }
    }
    else if (evento.type == SDL_MOUSEMOTION)
    {
        const SDL_Point cursor = {evento.motion.x, evento.motion.y};
        for (int i = 0; i < NUM_OPCOES; i++)
        {
            SDL_Rect opcao = this->retangulo_opcao(i);
            if (SDL_PointInRect(&cursor, &opcao))
            {
                this->selecionado = i;
            }
        }
    }
    else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT)
    {
        const SDL_Point cursor = {evento.button.x, evento.button.y};
        for (int i = 0; i < NUM_OPCOES; i++)
        {
            SDL_Rect opcao = this->retangulo_opcao(i);
            if (SDL_PointInRect(&cursor, &opcao))
            {
                this->selecionado = i;
                return this->acao_da_opcao(i);
            }
        }
    }

    return AcaoMenu::NENHUMA;
}

void Menu::desenhar(SDL_Renderer *renderizador) const
{
    if (renderizador == nullptr)
    {
        return;
    }

    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);

    // Veu escuro por cima do jogo congelado.
    SDL_SetRenderDrawColor(renderizador, COR_VEU.r, COR_VEU.g, COR_VEU.b, COR_VEU.a);
    SDL_Rect tela = {0, 0, LARGURA_TELA, ALTURA_TELA};
    SDL_RenderFillRect(renderizador, &tela);

    const SDL_Rect painel = this->retangulo_painel();

    SDL_SetRenderDrawColor(renderizador, COR_PAINEL.r, COR_PAINEL.g, COR_PAINEL.b, COR_PAINEL.a);
    SDL_RenderFillRect(renderizador, &painel);

    SDL_SetRenderDrawColor(renderizador, COR_PAINEL_BORDA.r, COR_PAINEL_BORDA.g, COR_PAINEL_BORDA.b,
                           COR_PAINEL_BORDA.a);
    SDL_RenderDrawRect(renderizador, &painel);

    const int centroX = painel.x + painel.w / 2;

    Texto::desenhar_centralizado(renderizador, this->inicial ? "CODIGOS DO BEM" : "PAUSADO",
                                 centroX, painel.y + 36, this->inicial ? 4 : ESCALA_TITULO,
                                 COR_TEXTO);

    for (int i = 0; i < NUM_OPCOES; i++)
    {
        const SDL_Rect area = this->retangulo_opcao(i);
        const bool ativa = (i == this->selecionado);

        if (ativa)
        {
            SDL_SetRenderDrawColor(renderizador, COR_REALCE.r, COR_REALCE.g, COR_REALCE.b,
                                   COR_REALCE.a);
            SDL_RenderFillRect(renderizador, &area);

            SDL_SetRenderDrawColor(renderizador, COR_PAINEL_BORDA.r, COR_PAINEL_BORDA.g,
                                   COR_PAINEL_BORDA.b, COR_PAINEL_BORDA.a);
            SDL_Rect marcador = {area.x, area.y, 4, area.h};
            SDL_RenderFillRect(renderizador, &marcador);
        }

        const bool desabilitada = this->inicial && i == 1 && !this->podeContinuar;
        const SDL_Color cor = desabilitada ? COR_TEXTO_APAGADO :
            (ativa ? COR_OPCAO_ATIVA : COR_OPCAO);
        const int textoY = area.y + (area.h - Texto::altura(ESCALA_OPCAO)) / 2;
        Texto::desenhar_centralizado(renderizador,
                                     this->inicial ? OPCOES_INICIAIS[i] : OPCOES_PAUSA[i],
                                     centroX, textoY, ESCALA_OPCAO, cor);
    }

    if (this->inicial)
    {
        Texto::desenhar_centralizado(renderizador, this->detalhe, centroX,
                                     painel.y + 278, 2, COR_TEXTO);
        Texto::desenhar_centralizado(renderizador, "NOVO JOGO SUBSTITUI O PROGRESSO SALVO", centroX,
                                     painel.y + 308, 2, COR_TEXTO_APAGADO);
    }
    else
    {
        Texto::desenhar_centralizado(renderizador, "MENU INICIAL: RETORNO PELO CHECKPOINT", centroX,
                                     painel.y + 290, 2, COR_TEXTO_APAGADO);
    }

    Texto::desenhar_centralizado(renderizador, "SETAS NAVEGAR - ENTER CONFIRMAR", centroX,
                                 painel.y + painel.h - 32, ESCALA_DICA, COR_TEXTO_APAGADO);
}

void Menu::configurar_inicial(bool podeContinuar, const std::string &detalhe)
{
    this->inicial = true;
    this->podeContinuar = podeContinuar;
    this->detalhe = detalhe;
    this->selecionado = podeContinuar ? 1 : 0;
}

void Menu::configurar_pausa()
{
    this->inicial = false;
    this->reiniciar();
}
