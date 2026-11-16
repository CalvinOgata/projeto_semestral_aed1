#include "interface/Menu.h"

#include "config/Config.h"
#include "interface/Texto.h"

namespace
{
const char *OPCOES_PAUSA[Menu::NUM_OPCOES_PAUSA] = {"CONTINUAR", "REINICIAR", "MENU INICIAL",
                                                     "SAIR"};
const char *OPCOES_INICIAIS[Menu::NUM_OPCOES_INICIAIS] = {"NOVO JOGO", "CONTINUAR", "SAIR"};

constexpr int ALTURA_OPCAO = 44;
constexpr int ALTURA_OPCAO_SLOT = 64;
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

bool Menu::tela_inicial() const
{
    return tela == TelaMenu::INICIAL;
}

bool Menu::tela_slots() const
{
    return tela == TelaMenu::SLOTS_NOVO || tela == TelaMenu::SLOTS_CONTINUAR;
}

int Menu::get_selecionado() const
{
    return this->selecionado;
}

void Menu::mover_selecao(int passo)
{
    const int quantidade = this->get_quantidadeOpcoes();
    this->selecionado = ((this->selecionado + passo) % quantidade + quantidade) % quantidade;
}

int Menu::get_quantidadeOpcoes() const
{
    if (tela_slots())
    {
        return NUM_OPCOES_SLOTS;
    }
    return tela_inicial() ? NUM_OPCOES_INICIAIS : NUM_OPCOES_PAUSA;
}

AcaoMenu Menu::acao_da_opcao(int indice) const
{
    if (tela_slots())
    {
        if (indice == QUANTIDADE_SLOTS_SAVE)
        {
            return AcaoMenu::VOLTAR;
        }
        if (tela == TelaMenu::SLOTS_CONTINUAR && !slotsDisponiveis[indice])
        {
            return AcaoMenu::NENHUMA;
        }
        return indice == 0 ? AcaoMenu::SLOT_1 :
            (indice == 1 ? AcaoMenu::SLOT_2 : AcaoMenu::SLOT_3);
    }
    if (tela_inicial())
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
            return AcaoMenu::REINICIAR;
        }
        if (indice == 2)
        {
            return AcaoMenu::MENU_INICIAL;
        }
    }
    return AcaoMenu::SAIR;
}

SDL_Rect Menu::retangulo_painel() const
{
    SDL_Rect painel;
    painel.w = tela_slots() ? 760 : 700;
    painel.h = tela_slots() ? 450 : 370;
    painel.x = (LARGURA_TELA - painel.w) / 2;
    painel.y = (ALTURA_TELA - painel.h) / 2;
    return painel;
}

SDL_Rect Menu::retangulo_opcao(int indice) const
{
    const SDL_Rect painel = this->retangulo_painel();
    if (tela_slots())
    {
        if (indice == QUANTIDADE_SLOTS_SAVE)
        {
            return {painel.x + 40, painel.y + 342, painel.w - 80, ALTURA_OPCAO};
        }
        return {painel.x + 40, painel.y + 82 + indice * (ALTURA_OPCAO_SLOT + 10),
                painel.w - 80, ALTURA_OPCAO_SLOT};
    }
    return {painel.x + 30, painel.y + (tela_inicial() ? 100 : 132) +
                                      indice * (ALTURA_OPCAO + 10),
            painel.w - 60, ALTURA_OPCAO};
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
            return tela_slots() ? AcaoMenu::VOLTAR :
                (tela_inicial() ? AcaoMenu::NENHUMA : AcaoMenu::CONTINUAR);
        default:
            break;
        }
    }
    else if (evento.type == SDL_MOUSEMOTION)
    {
        const SDL_Point cursor = {evento.motion.x, evento.motion.y};
        for (int i = 0; i < this->get_quantidadeOpcoes(); ++i)
        {
            const SDL_Rect opcao = this->retangulo_opcao(i);
            if (SDL_PointInRect(&cursor, &opcao))
            {
                this->selecionado = i;
            }
        }
    }
    else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT)
    {
        const SDL_Point cursor = {evento.button.x, evento.button.y};
        for (int i = 0; i < this->get_quantidadeOpcoes(); ++i)
        {
            const SDL_Rect opcao = this->retangulo_opcao(i);
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
    SDL_SetRenderDrawColor(renderizador, COR_VEU.r, COR_VEU.g, COR_VEU.b, COR_VEU.a);
    const SDL_Rect telaInteira = {0, 0, LARGURA_TELA, ALTURA_TELA};
    SDL_RenderFillRect(renderizador, &telaInteira);

    const SDL_Rect painel = this->retangulo_painel();
    SDL_SetRenderDrawColor(renderizador, COR_PAINEL.r, COR_PAINEL.g, COR_PAINEL.b, COR_PAINEL.a);
    SDL_RenderFillRect(renderizador, &painel);
    SDL_SetRenderDrawColor(renderizador, COR_PAINEL_BORDA.r, COR_PAINEL_BORDA.g, COR_PAINEL_BORDA.b,
                           COR_PAINEL_BORDA.a);
    SDL_RenderDrawRect(renderizador, &painel);
    const int centroX = painel.x + painel.w / 2;

    if (tela_inicial())
    {
        Texto::desenhar_centralizado(renderizador, "M.C.G.A", centroX, painel.y + 18, 5,
                                    COR_TEXTO);
        Texto::desenhar_centralizado(renderizador, "MAKE COMPUTERS GREAT AGAIN", centroX,
                                    painel.y + 68, 2, COR_TEXTO_APAGADO);
    }
    else
    {
        const char *titulo = tela == TelaMenu::PAUSA ? "PAUSADO" :
            (tela == TelaMenu::SLOTS_NOVO ? "NOVO JOGO - ESCOLHA O SLOT" :
                                           "CONTINUAR - ESCOLHA O SLOT");
        Texto::desenhar_centralizado(renderizador, titulo, centroX, painel.y + 30,
                                    tela_slots() ? 3 : ESCALA_TITULO, COR_TEXTO);
    }

    if (tela == TelaMenu::PAUSA)
    {
        const auto quebra = this->detalhe.find('\n');
        Texto::desenhar_centralizado(renderizador, this->detalhe.substr(0, quebra), centroX,
                                    painel.y + 84, 2, COR_TEXTO);
        if (quebra != std::string::npos)
        {
            Texto::desenhar_centralizado(renderizador, this->detalhe.substr(quebra + 1), centroX,
                                        painel.y + 106, 2, COR_TEXTO_APAGADO);
        }
    }

    for (int i = 0; i < this->get_quantidadeOpcoes(); ++i)
    {
        const SDL_Rect area = this->retangulo_opcao(i);
        const bool ativa = i == this->selecionado;
        const bool desabilitada = (tela_inicial() && i == 1 && !this->podeContinuar) ||
            (tela == TelaMenu::SLOTS_CONTINUAR && i < QUANTIDADE_SLOTS_SAVE &&
             !slotsDisponiveis[i]);
        if (ativa)
        {
            SDL_SetRenderDrawColor(renderizador, COR_REALCE.r, COR_REALCE.g, COR_REALCE.b,
                                   COR_REALCE.a);
            SDL_RenderFillRect(renderizador, &area);
            SDL_SetRenderDrawColor(renderizador, COR_PAINEL_BORDA.r, COR_PAINEL_BORDA.g,
                                   COR_PAINEL_BORDA.b, COR_PAINEL_BORDA.a);
            const SDL_Rect marcador = {area.x, area.y, 4, area.h};
            SDL_RenderFillRect(renderizador, &marcador);
        }
        const SDL_Color cor = desabilitada ? COR_TEXTO_APAGADO :
            (ativa ? COR_OPCAO_ATIVA : COR_OPCAO);
        if (tela_slots())
        {
            if (i == QUANTIDADE_SLOTS_SAVE)
            {
                Texto::desenhar_centralizado(renderizador, "VOLTAR", centroX,
                                            area.y + (area.h - Texto::altura(2)) / 2, 2, cor);
            }
            else
            {
                Texto::desenhar_centralizado(renderizador, "SLOT " + std::to_string(i + 1),
                                            centroX, area.y + 7, 2, cor);
                Texto::desenhar_centralizado(renderizador, detalhesSlots[i], centroX,
                                            area.y + 37, 1, cor);
            }
        }
        else
        {
            const int textoY = area.y + (area.h - Texto::altura(ESCALA_OPCAO)) / 2;
            Texto::desenhar_centralizado(renderizador,
                                        tela_inicial() ? OPCOES_INICIAIS[i] : OPCOES_PAUSA[i],
                                        centroX, textoY, ESCALA_OPCAO, cor);
        }
    }

    if (tela_inicial())
    {
        Texto::desenhar_centralizado(renderizador, this->detalhe, centroX, painel.y + 278, 2,
                                    COR_TEXTO);
        Texto::desenhar_centralizado(renderizador, "3 SLOTS DE PROGRESSO INDEPENDENTES", centroX,
                                    painel.y + 308, 2, COR_TEXTO_APAGADO);
    }
    const char *dica = tela_slots() ? "SETAS NAVEGAR - ENTER CONFIRMAR - ESC VOLTAR" :
                                     "SETAS NAVEGAR - ENTER CONFIRMAR";
    if (tela != TelaMenu::PAUSA)
    {
        Texto::desenhar_centralizado(renderizador, dica, centroX, painel.y + painel.h - 32,
                                    ESCALA_DICA, COR_TEXTO_APAGADO);
    }
}

void Menu::configurar_inicial(bool podeContinuar, const std::string &detalhe)
{
    this->tela = TelaMenu::INICIAL;
    this->podeContinuar = podeContinuar;
    this->detalhe = detalhe;
    this->selecionado = podeContinuar ? 1 : 0;
}

void Menu::configurar_pausa(const std::string &detalhe)
{
    this->tela = TelaMenu::PAUSA;
    this->detalhe = detalhe;
    this->reiniciar();
}

void Menu::configurar_slots(
    bool novoJogo, const std::array<std::string, QUANTIDADE_SLOTS_SAVE> &detalhes,
    const std::array<bool, QUANTIDADE_SLOTS_SAVE> &disponiveis)
{
    this->tela = novoJogo ? TelaMenu::SLOTS_NOVO : TelaMenu::SLOTS_CONTINUAR;
    this->detalhesSlots = detalhes;
    this->slotsDisponiveis = disponiveis;
    this->selecionado = 0;
    if (!novoJogo)
    {
        while (this->selecionado < QUANTIDADE_SLOTS_SAVE &&
               !this->slotsDisponiveis[this->selecionado])
        {
            ++this->selecionado;
        }
    }
}
