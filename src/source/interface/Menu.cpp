#include "interface/Menu.h"

#include <algorithm>
#include <cmath>
#include <vector>

#include "audio/Som.h"
#include "config/Config.h"
#include "entidades/Inimigo.h"
#include "graficos/Sprite.h"
#include "graficos/SpriteInimigo.h"
#include "interface/FundoMenu.h"
#include "interface/Texto.h"

namespace
{
const char *OPCOES_PRINCIPAIS[] = {"JOGAR", "MANUAL", "CONFIGURACOES", "SOBRE", "SAIR"};
const char *OPCOES_JOGAR[] = {"NOVO JOGO", "CONTINUAR", "VOLTAR"};
const char *OPCOES_PAUSA[] = {"CONTINUAR", "MANUAL", "REINICIAR", "MENU INICIAL", "SAIR"};

constexpr int ALTURA_OPCAO = 40;
constexpr int ALTURA_OPCAO_SLOT = 64;
constexpr int ALTURA_LINHA_CONFIG = 46;

constexpr SDL_Color COR_CIANO = {90, 220, 255, 255};
constexpr SDL_Color COR_DOURADO = {255, 210, 100, 255};
constexpr SDL_Color COR_VERDE_OK = {110, 250, 150, 255};

// Integrantes da equipe, na ordem em que aparecem em "SOBRE".
const char *INTEGRANTES[] = {"CALVIN DA SERRA OGATA DE PAULA", "FELIPE BARATELA PIMENTA",
                             "JOAO PEDRO TOMAZ DOS SANTOS", "PEDRO TESTA LOURENCO"};

struct Tecnologia
{
    const char *nome;
    const char *uso;
    SDL_Color cor;
};

const Tecnologia TECNOLOGIAS[] = {
    {"C++17", "LINGUAGEM DO JOGO INTEIRO", {90, 160, 255, 255}},
    {"SDL2", "JANELA, DESENHO, TECLADO, MOUSE E AUDIO", {120, 230, 160, 255}},
    {"LIBPNG + ZLIB", "SPRITES PNG EMBUTIDOS NO EXECUTAVEL", {255, 190, 90, 255}},
    {"EMSCRIPTEN", "VERSAO PARA NAVEGADOR (WEBASSEMBLY)", {200, 120, 255, 255}},
    {"MAKE + CMAKE", "COMPILACAO DO JOGO E DAS BIBLIOTECAS", {255, 120, 140, 255}},
    {"DOCKER + MINGW-W64", "EXECUTAVEIS PARA LINUX E WINDOWS", {90, 200, 255, 255}},
    {"GITHUB ACTIONS", "BUILDS E RELEASES AUTOMATICOS", {230, 230, 240, 255}},
    {"CLANG-FORMAT", "PADRAO DE FORMATACAO DO CODIGO", {160, 220, 120, 255}},
};

struct Comando
{
    std::vector<const char *> teclas;
    const char *acao;
    const char *detalhe;
};

const Comando COMANDOS[] = {
    {{"W", "A", "S", "D"}, "MOVER", "AS SETAS DO TECLADO TAMBEM FUNCIONAM"},
    {{"MOUSE ESQ"}, "ATIRAR", "SEGURE PARA DISPARAR. A MIRA TRAVA NO INIMIGO MAIS PROXIMO"},
    {{"MOUSE"}, "MIRAR", "USADO QUANDO NAO HA INIMIGOS NA SALA"},
    {{"F"}, "PEGAR ARMA", "PEGA A ARMA DO CHAO MAIS PROXIMA - CABEM 3 NO INVENTARIO"},
    {{"Q"}, "TROCAR ARMA", "MANDA A ARMA EQUIPADA PARA O FIM DA FILA"},
    {{"G"}, "SOLTAR ARMA", "DEIXA A ARMA EQUIPADA NO CHAO COM A MUNICAO QUE SOBROU"},
    {{"M"}, "MINIMAPA", "MOSTRA AS SALAS JA DESCOBERTAS DA FASE"},
    {{"ESC"}, "PAUSAR", "CONTINUAR, REINICIAR A FASE OU VOLTAR AO MENU"},
    {{"F2", "F5"}, "EXTRAS", "F2 MOSTRA AS HITBOXES - F5 TENTA SALVAR DE NOVO SE FALHAR"},
};

struct Secao
{
    const char *titulo;
    const char *texto;
    SDL_Color cor;
};

const Secao SECOES_COMO_JOGAR[] = {
    {"OBJETIVO",
     "LIMPE CADA SALA PARA DESTRANCAR AS PORTAS, DERROTE O CHEFE E ENTRE NO PORTAL. "
     "SAO 3 FASES CONTRA OS SISTEMAS MALIGNOS.",
     {255, 210, 100, 255}},
    {"AFINIDADES",
     "ARMAS E INIMIGOS SAO VERMELHOS, AZUIS OU VERDES. ARMA DA MESMA COR DO INIMIGO CAUSA "
     "DANO EM DOBRO.",
     {255, 110, 120, 255}},
    {"VIDA E ESCUDO",
     "O ESCUDO ABSORVE OS TIROS E VOLTA APOS 3 S SEM DANO. CORACOES DOS INIMIGOS CURAM "
     "10 DE VIDA, MAS SOMEM EM 10 S.",
     {110, 200, 255, 255}},
    {"ARMAS E BAUS",
     "BAUS ABREM QUANDO VOCE CHEGA PERTO. ESPADA E BUMERANGUE NAO GASTAM MUNICAO. SEM "
     "ARMAS, O TUX LUTA NO SOCO.",
     {150, 240, 140, 255}},
    {"VIDAS E SAVES",
     "SAO 3 VIDAS E UM UNICO CONTINUE. O JOGO SALVA AO COMECAR E A CADA FASE CONCLUIDA, "
     "EM 3 SLOTS.",
     {210, 150, 255, 255}},
};

Uint8 canal(float valor)
{
    return static_cast<Uint8>(std::clamp(valor, 0.0f, 255.0f));
}

// Tecla desenhada como um "keycap" com sombra. Devolve a largura usada.
int desenhar_tecla(SDL_Renderer *r, const std::string &rotulo, int x, int y)
{
    const int largura = Texto::largura(rotulo, 2) + 14;
    FundoMenu::preencher(r, {x, y + 3, largura, 24}, {10, 10, 20, 255});
    FundoMenu::preencher(r, {x, y, largura, 24}, {58, 64, 96, 255});
    FundoMenu::preencher(r, {x + 2, y + 2, largura - 4, 17}, {82, 90, 132, 255});
    Texto::desenhar(r, rotulo, x + 7, y + 4, 2, {240, 244, 255, 255});
    return largura;
}
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

void Menu::atualizar(float dt)
{
    this->tempo += dt;
}

bool Menu::tela_slots() const
{
    return tela == TelaMenu::SLOTS_NOVO || tela == TelaMenu::SLOTS_CONTINUAR;
}

bool Menu::tela_painelInteiro() const
{
    return tela == TelaMenu::MANUAL || tela == TelaMenu::SOBRE;
}

TelaMenu Menu::get_tela() const
{
    return this->tela;
}

int Menu::get_selecionado() const
{
    return this->selecionado;
}

void Menu::set_configuracoes(const Configuracoes &configuracoes)
{
    this->configuracoes = configuracoes;
}

const Configuracoes &Menu::get_configuracoes() const
{
    return this->configuracoes;
}

int Menu::get_quantidadeOpcoes() const
{
    switch (tela)
    {
    case TelaMenu::PRINCIPAL:
        return NUM_OPCOES_PRINCIPAIS;
    case TelaMenu::JOGAR:
        return NUM_OPCOES_JOGAR;
    case TelaMenu::SLOTS_NOVO:
    case TelaMenu::SLOTS_CONTINUAR:
        return NUM_OPCOES_SLOTS;
    case TelaMenu::CONFIGURACOES:
        return Plataforma::PERMITE_TELA_CHEIA ? 5 : 4;
    case TelaMenu::MANUAL:
    case TelaMenu::SOBRE:
        return 1;
    case TelaMenu::PAUSA:
        return NUM_OPCOES_PAUSA;
    }
    return 1;
}

Menu::ItemConfig Menu::item_config(int indice) const
{
    if (!Plataforma::PERMITE_TELA_CHEIA && indice >= 2)
    {
        ++indice; // pula TELA_CHEIA
    }
    return static_cast<ItemConfig>(indice);
}

bool Menu::opcao_desabilitada(int indice) const
{
    if (tela == TelaMenu::JOGAR)
    {
        return indice == 1 && !this->podeContinuar;
    }
    if (tela == TelaMenu::SLOTS_CONTINUAR)
    {
        return indice < QUANTIDADE_SLOTS_SAVE && !slotsDisponiveis[indice];
    }
    return false;
}

void Menu::mover_selecao(int passo)
{
    const int quantidade = this->get_quantidadeOpcoes();
    const int anterior = this->selecionado;
    // Pula opcoes desabilitadas; ha sempre ao menos "VOLTAR" habilitada.
    for (int tentativa = 0; tentativa < quantidade; ++tentativa)
    {
        this->selecionado = ((this->selecionado + passo) % quantidade + quantidade) % quantidade;
        if (!this->opcao_desabilitada(this->selecionado))
        {
            break;
        }
    }
    if (this->selecionado != anterior)
    {
        Som::tocar(EfeitoSonoro::MENU_MOVER);
    }
}

void Menu::abrir_tela(TelaMenu nova, int selecao)
{
    this->tela = nova;
    this->selecionado = selecao;
    if (nova == TelaMenu::MANUAL)
    {
        this->abaManual = 0;
    }
}

void Menu::trocar_aba(int passo)
{
    this->abaManual = (this->abaManual + passo + 2) % 2;
    Som::tocar(EfeitoSonoro::MENU_AJUSTE);
}

AcaoMenu Menu::ajustar_config(int indice, int passo)
{
    Configuracoes &c = this->configuracoes;
    switch (this->item_config(indice))
    {
    case ItemConfig::VOLUME_MUSICA:
        c.volumeMusica = std::clamp(c.volumeMusica + passo, 0, Configuracoes::VOLUME_MAXIMO);
        break;
    case ItemConfig::VOLUME_EFEITOS:
        c.volumeEfeitos = std::clamp(c.volumeEfeitos + passo, 0, Configuracoes::VOLUME_MAXIMO);
        break;
    case ItemConfig::TELA_CHEIA:
        c.telaCheia = !c.telaCheia;
        break;
    case ItemConfig::HITBOXES:
        c.mostrarHitboxes = !c.mostrarHitboxes;
        break;
    case ItemConfig::VOLTAR:
        return AcaoMenu::NENHUMA;
    }
    // O jogo aplica o volume antes deste som, entao ele ja sai na altura nova.
    return AcaoMenu::CONFIGURACOES_ALTERADAS;
}

AcaoMenu Menu::ativar_opcao(int indice)
{
    if (this->opcao_desabilitada(indice))
    {
        return AcaoMenu::NENHUMA;
    }
    switch (tela)
    {
    case TelaMenu::PRINCIPAL:
        Som::tocar(EfeitoSonoro::MENU_CONFIRMAR);
        if (indice == 0)
        {
            this->abrir_tela(TelaMenu::JOGAR, this->podeContinuar ? 1 : 0);
        }
        else if (indice == 1)
        {
            this->manualDaPausa = false;
            this->abrir_tela(TelaMenu::MANUAL, 0);
        }
        else if (indice == 2)
        {
            this->abrir_tela(TelaMenu::CONFIGURACOES, 0);
        }
        else if (indice == 3)
        {
            this->abrir_tela(TelaMenu::SOBRE, 0);
        }
        else
        {
            return AcaoMenu::SAIR;
        }
        return AcaoMenu::NENHUMA;
    case TelaMenu::JOGAR:
        if (indice == 2)
        {
            Som::tocar(EfeitoSonoro::MENU_VOLTAR);
            this->abrir_tela(TelaMenu::PRINCIPAL, 0);
            return AcaoMenu::NENHUMA;
        }
        Som::tocar(EfeitoSonoro::MENU_CONFIRMAR);
        return indice == 0 ? AcaoMenu::NOVO_JOGO : AcaoMenu::CONTINUAR;
    case TelaMenu::SLOTS_NOVO:
    case TelaMenu::SLOTS_CONTINUAR:
        if (indice == QUANTIDADE_SLOTS_SAVE)
        {
            Som::tocar(EfeitoSonoro::MENU_VOLTAR);
            return AcaoMenu::VOLTAR;
        }
        Som::tocar(EfeitoSonoro::MENU_CONFIRMAR);
        return indice == 0 ? AcaoMenu::SLOT_1 : (indice == 1 ? AcaoMenu::SLOT_2 : AcaoMenu::SLOT_3);
    case TelaMenu::MANUAL:
        Som::tocar(EfeitoSonoro::MENU_VOLTAR);
        this->abrir_tela(this->manualDaPausa ? TelaMenu::PAUSA : TelaMenu::PRINCIPAL, 1);
        return AcaoMenu::NENHUMA;
    case TelaMenu::SOBRE:
        Som::tocar(EfeitoSonoro::MENU_VOLTAR);
        this->abrir_tela(TelaMenu::PRINCIPAL, 3);
        return AcaoMenu::NENHUMA;
    case TelaMenu::CONFIGURACOES:
        if (this->item_config(indice) == ItemConfig::VOLTAR)
        {
            Som::tocar(EfeitoSonoro::MENU_VOLTAR);
            this->abrir_tela(TelaMenu::PRINCIPAL, 2);
            return AcaoMenu::NENHUMA;
        }
        // Enter alterna as opcoes de sim/nao e sobe o volume em ciclo.
        if (this->item_config(indice) == ItemConfig::VOLUME_MUSICA ||
            this->item_config(indice) == ItemConfig::VOLUME_EFEITOS)
        {
            const int atual = this->item_config(indice) == ItemConfig::VOLUME_MUSICA
                                  ? this->configuracoes.volumeMusica
                                  : this->configuracoes.volumeEfeitos;
            return this->ajustar_config(
                indice, atual == Configuracoes::VOLUME_MAXIMO ? -Configuracoes::VOLUME_MAXIMO : 1);
        }
        return this->ajustar_config(indice, 1);
    case TelaMenu::PAUSA:
        if (indice == 0)
        {
            Som::tocar(EfeitoSonoro::MENU_VOLTAR);
            return AcaoMenu::CONTINUAR;
        }
        Som::tocar(EfeitoSonoro::MENU_CONFIRMAR);
        if (indice == 1)
        {
            this->manualDaPausa = true;
            this->abrir_tela(TelaMenu::MANUAL, 0);
            return AcaoMenu::NENHUMA;
        }
        if (indice == 2)
        {
            return AcaoMenu::REINICIAR;
        }
        if (indice == 3)
        {
            return AcaoMenu::MENU_INICIAL;
        }
        return AcaoMenu::SAIR;
    }
    return AcaoMenu::NENHUMA;
}

SDL_Rect Menu::retangulo_painel() const
{
    switch (tela)
    {
    case TelaMenu::JOGAR:
        return {250, 140, 460, 300};
    case TelaMenu::SLOTS_NOVO:
    case TelaMenu::SLOTS_CONTINUAR:
        return {100, 96, 760, 380};
    case TelaMenu::MANUAL:
    case TelaMenu::SOBRE:
        return {36, 20, 888, 500};
    case TelaMenu::CONFIGURACOES:
        return {190, 100, 580, 360};
    case TelaMenu::PAUSA:
    {
        const int altura = 160 + NUM_OPCOES_PAUSA * 54;
        return {230, (ALTURA_TELA - altura) / 2, 500, altura};
    }
    case TelaMenu::PRINCIPAL:
        break;
    }
    return {330, 190, 300, 270};
}

SDL_Rect Menu::retangulo_opcao(int indice) const
{
    const SDL_Rect painel = this->retangulo_painel();
    switch (tela)
    {
    case TelaMenu::PRINCIPAL:
        return {330, 196 + indice * 52, 300, ALTURA_OPCAO + 4};
    case TelaMenu::JOGAR:
        return {painel.x + 50, painel.y + 70 + indice * 54, painel.w - 100, ALTURA_OPCAO + 4};
    case TelaMenu::SLOTS_NOVO:
    case TelaMenu::SLOTS_CONTINUAR:
        if (indice == QUANTIDADE_SLOTS_SAVE)
        {
            return {painel.x + 230, painel.y + 318, painel.w - 460, ALTURA_OPCAO};
        }
        return {painel.x + 40, painel.y + 70 + indice * (ALTURA_OPCAO_SLOT + 16), painel.w - 80,
                ALTURA_OPCAO_SLOT};
    case TelaMenu::MANUAL:
    case TelaMenu::SOBRE:
        return {painel.x + painel.w / 2 - 100, painel.y + painel.h - 50, 200, 36};
    case TelaMenu::CONFIGURACOES:
        if (this->item_config(indice) == ItemConfig::VOLTAR)
        {
            return {painel.x + painel.w / 2 - 100, painel.y + painel.h - 64, 200, 40};
        }
        return {painel.x + 30, painel.y + 74 + indice * (ALTURA_LINHA_CONFIG + 8), painel.w - 60,
                ALTURA_LINHA_CONFIG};
    case TelaMenu::PAUSA:
        return {painel.x + 40, painel.y + 136 + indice * 54, painel.w - 80, ALTURA_OPCAO + 4};
    }
    return {0, 0, 0, 0};
}

SDL_Rect Menu::retangulo_aba(int indice) const
{
    const SDL_Rect painel = this->retangulo_painel();
    return {painel.x + painel.w / 2 - 210 + indice * 216, painel.y + 54, 204, 32};
}

AcaoMenu Menu::tratar_evento(const SDL_Event &evento)
{
    if (evento.type == SDL_KEYDOWN)
    {
        const SDL_Keycode tecla = evento.key.keysym.sym;
        const bool repeticao = evento.key.repeat != 0;
        // Setas laterais repetem ao segurar, para ajustar volume rapido.
        if (tecla == SDLK_LEFT || tecla == SDLK_a || tecla == SDLK_RIGHT || tecla == SDLK_d)
        {
            const int passo = (tecla == SDLK_LEFT || tecla == SDLK_a) ? -1 : 1;
            if (tela == TelaMenu::CONFIGURACOES &&
                this->item_config(this->selecionado) != ItemConfig::VOLTAR)
            {
                return this->ajustar_config(this->selecionado, passo);
            }
            if (tela == TelaMenu::MANUAL && !repeticao)
            {
                this->trocar_aba(passo);
            }
            return AcaoMenu::NENHUMA;
        }
        if (repeticao && tecla != SDLK_UP && tecla != SDLK_DOWN && tecla != SDLK_w &&
            tecla != SDLK_s)
        {
            return AcaoMenu::NENHUMA;
        }
        switch (tecla)
        {
        case SDLK_UP:
        case SDLK_w:
            this->mover_selecao(-1);
            break;
        case SDLK_DOWN:
        case SDLK_s:
            this->mover_selecao(1);
            break;
        case SDLK_TAB:
            if (tela == TelaMenu::MANUAL)
            {
                this->trocar_aba(1);
            }
            break;
        case SDLK_RETURN:
        case SDLK_KP_ENTER:
        case SDLK_SPACE:
            return this->ativar_opcao(this->selecionado);
        case SDLK_ESCAPE:
            switch (tela)
            {
            case TelaMenu::PRINCIPAL:
                return AcaoMenu::NENHUMA;
            case TelaMenu::PAUSA:
                return this->ativar_opcao(0);
            case TelaMenu::SLOTS_NOVO:
            case TelaMenu::SLOTS_CONTINUAR:
                return this->ativar_opcao(QUANTIDADE_SLOTS_SAVE);
            case TelaMenu::JOGAR:
                return this->ativar_opcao(2);
            default:
                return this->ativar_opcao(this->get_quantidadeOpcoes() - 1);
            }
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
            if (SDL_PointInRect(&cursor, &opcao) && i != this->selecionado &&
                !this->opcao_desabilitada(i))
            {
                this->selecionado = i;
                Som::tocar(EfeitoSonoro::MENU_MOVER);
            }
        }
    }
    else if (evento.type == SDL_MOUSEBUTTONDOWN && evento.button.button == SDL_BUTTON_LEFT)
    {
        const SDL_Point cursor = {evento.button.x, evento.button.y};
        if (tela == TelaMenu::MANUAL)
        {
            for (int aba = 0; aba < 2; ++aba)
            {
                const SDL_Rect area = this->retangulo_aba(aba);
                if (SDL_PointInRect(&cursor, &area) && aba != this->abaManual)
                {
                    this->trocar_aba(1);
                    return AcaoMenu::NENHUMA;
                }
            }
        }
        for (int i = 0; i < this->get_quantidadeOpcoes(); ++i)
        {
            const SDL_Rect opcao = this->retangulo_opcao(i);
            if (!SDL_PointInRect(&cursor, &opcao))
            {
                continue;
            }
            this->selecionado = i;
            const ItemConfig item = this->item_config(i);
            if (tela == TelaMenu::CONFIGURACOES &&
                (item == ItemConfig::VOLUME_MUSICA || item == ItemConfig::VOLUME_EFEITOS))
            {
                // Clique na metade esquerda do controle abaixa, na direita aumenta.
                const int meio = opcao.x + opcao.w - 150;
                return this->ajustar_config(i, cursor.x < meio ? -1 : 1);
            }
            return this->ativar_opcao(i);
        }
    }
    return AcaoMenu::NENHUMA;
}

void Menu::desenhar_botao(SDL_Renderer *r, const SDL_Rect &area, const std::string &texto,
                          bool ativa, bool desabilitada, int escala) const
{
    const float pulso = 0.5f + 0.5f * std::sin(this->tempo * 6.0f);
    SDL_Rect caixa = area;
    if (ativa)
    {
        FundoMenu::preencher(r, {caixa.x - 4, caixa.y - 4, caixa.w + 8, caixa.h + 8},
                             {90, 220, 255, canal(30 + 30 * pulso)});
        FundoMenu::preencher(r, caixa, {40, 30, 90, 235});
        FundoMenu::preencher(r, {caixa.x, caixa.y, caixa.w, caixa.h / 2}, {70, 50, 140, 120});
        SDL_SetRenderDrawColor(r, COR_CIANO.r, COR_CIANO.g, COR_CIANO.b, canal(170 + 85 * pulso));
        SDL_RenderDrawRect(r, &caixa);
        FundoMenu::preencher(r, {caixa.x, caixa.y, 4, caixa.h}, {255, 80, 200, 255});
        FundoMenu::preencher(r, {caixa.x + caixa.w - 4, caixa.y, 4, caixa.h}, {255, 80, 200, 255});
    }
    else
    {
        FundoMenu::preencher(r, caixa, {16, 18, 40, 205});
        SDL_SetRenderDrawColor(r, 70, 80, 130, 200);
        SDL_RenderDrawRect(r, &caixa);
    }
    const SDL_Color cor =
        desabilitada ? SDL_Color{90, 92, 110, 255}
                     : (ativa ? SDL_Color{255, 255, 255, 255} : SDL_Color{180, 190, 215, 255});
    const int centroX = caixa.x + caixa.w / 2;
    const int textoY = caixa.y + (caixa.h - Texto::altura(escala)) / 2;
    Texto::desenhar_centralizado(r, texto, centroX, textoY, escala, cor);
    if (ativa && !texto.empty())
    {
        // Setas que "respiram" dos dois lados do texto.
        const int meia = Texto::largura(texto, escala) / 2;
        const int recuo = 14 + static_cast<int>(pulso * 4);
        Texto::desenhar(r, ">", centroX - meia - recuo - Texto::largura(">", escala), textoY,
                        escala, COR_DOURADO);
        Texto::desenhar(r, "<", centroX + meia + recuo, textoY, escala, COR_DOURADO);
    }
}

void Menu::desenhar(SDL_Renderer *renderizador, const Sprite &tux) const
{
    if (renderizador == nullptr)
    {
        return;
    }
    SDL_SetRenderDrawBlendMode(renderizador, SDL_BLENDMODE_BLEND);

    // Na pausa (inclusive no manual aberto por ela) a partida continua visivel ao fundo.
    if (tela == TelaMenu::PAUSA || (tela == TelaMenu::MANUAL && this->manualDaPausa))
    {
        FundoMenu::preencher(renderizador, {0, 0, LARGURA_TELA, ALTURA_TELA}, COR_VEU);
    }
    else
    {
        FundoMenu::desenhar(renderizador, this->tempo);
    }

    if (tela == TelaMenu::PRINCIPAL)
    {
        this->desenhar_principal(renderizador, tux);
        return;
    }

    const SDL_Rect painel = this->retangulo_painel();
    const int centroX = painel.x + painel.w / 2;
    if (tela == TelaMenu::JOGAR || tela_slots())
    {
        FundoMenu::desenhar_logo(renderizador, LARGURA_TELA / 2, 22, 6, this->tempo);
    }
    FundoMenu::desenhar_painel(renderizador, painel, this->tempo);

    switch (tela)
    {
    case TelaMenu::MANUAL:
        this->desenhar_manual(renderizador, painel);
        break;
    case TelaMenu::SOBRE:
        this->desenhar_sobre(renderizador, painel);
        break;
    case TelaMenu::CONFIGURACOES:
        this->desenhar_configuracoes(renderizador, painel);
        break;
    case TelaMenu::PAUSA:
    {
        Texto::desenhar_centralizado(renderizador, "PAUSADO", centroX, painel.y + 24, 5, COR_TEXTO);
        const auto quebra = this->detalhe.find('\n');
        Texto::desenhar_centralizado(renderizador, this->detalhe.substr(0, quebra), centroX,
                                     painel.y + 80, 2, COR_DOURADO);
        if (quebra != std::string::npos)
        {
            Texto::desenhar_centralizado(renderizador, this->detalhe.substr(quebra + 1), centroX,
                                         painel.y + 102, 2, COR_TEXTO_APAGADO);
        }
        for (int i = 0; i < NUM_OPCOES_PAUSA; ++i)
        {
            this->desenhar_botao(renderizador, this->retangulo_opcao(i), OPCOES_PAUSA[i],
                                 i == this->selecionado, false, 3);
        }
        break;
    }
    case TelaMenu::JOGAR:
        Texto::desenhar_centralizado(renderizador, "JOGAR", centroX, painel.y + 22, 4, COR_TEXTO);
        for (int i = 0; i < NUM_OPCOES_JOGAR; ++i)
        {
            this->desenhar_botao(renderizador, this->retangulo_opcao(i), OPCOES_JOGAR[i],
                                 i == this->selecionado, this->opcao_desabilitada(i), 3);
        }
        Texto::desenhar_centralizado(renderizador, this->detalhe, centroX, painel.y + 248, 2,
                                     COR_DOURADO);
        Texto::desenhar_centralizado(renderizador, "3 SLOTS DE PROGRESSO INDEPENDENTES", centroX,
                                     painel.y + 272, 1, COR_TEXTO_APAGADO);
        break;
    case TelaMenu::SLOTS_NOVO:
    case TelaMenu::SLOTS_CONTINUAR:
    {
        const char *titulo = tela == TelaMenu::SLOTS_NOVO ? "NOVO JOGO - ESCOLHA O SLOT"
                                                          : "CONTINUAR - ESCOLHA O SLOT";
        Texto::desenhar_centralizado(renderizador, titulo, centroX, painel.y + 24, 3, COR_TEXTO);
        for (int i = 0; i < QUANTIDADE_SLOTS_SAVE; ++i)
        {
            const SDL_Rect area = this->retangulo_opcao(i);
            const bool ativa = i == this->selecionado;
            const bool desabilitada = this->opcao_desabilitada(i);
            this->desenhar_botao(renderizador, area, "", ativa, desabilitada, 2);
            const SDL_Color cor = desabilitada ? SDL_Color{90, 92, 110, 255}
                                               : (ativa ? SDL_Color{255, 255, 255, 255}
                                                        : SDL_Color{180, 190, 215, 255});
            // Disquete desenhado como icone do slot.
            const SDL_Rect disco = {area.x + 18, area.y + 14, 34, 36};
            FundoMenu::preencher(renderizador, disco,
                                 desabilitada ? SDL_Color{60, 62, 80, 255}
                                              : SDL_Color{70, 110, 220, 255});
            FundoMenu::preencher(renderizador, {disco.x + 7, disco.y, 20, 12},
                                 {200, 210, 225, 255});
            FundoMenu::preencher(renderizador, {disco.x + 20, disco.y + 2, 4, 8},
                                 {40, 50, 80, 255});
            FundoMenu::preencher(renderizador, {disco.x + 5, disco.y + 20, 24, 14},
                                 {235, 235, 240, 255});
            Texto::desenhar_centralizado(renderizador, "SLOT " + std::to_string(i + 1), centroX,
                                         area.y + 10, 3, cor);
            Texto::desenhar_centralizado(renderizador, detalhesSlots[i], centroX, area.y + 42, 1,
                                         ativa ? COR_DOURADO : cor);
        }
        this->desenhar_botao(renderizador, this->retangulo_opcao(QUANTIDADE_SLOTS_SAVE), "VOLTAR",
                             this->selecionado == QUANTIDADE_SLOTS_SAVE, false, 2);
        break;
    }
    case TelaMenu::PRINCIPAL:
        break;
    }
}

void Menu::desenhar_principal(SDL_Renderer *r, const Sprite &tux) const
{
    FundoMenu::desenhar_logo(r, LARGURA_TELA / 2, 34, 11, this->tempo);
    Texto::desenhar_centralizado(r, "MAKE COMPUTERS GREAT AGAIN", LARGURA_TELA / 2, 128, 3,
                                 {210, 196, 255, 255});
    Texto::desenhar_centralizado(r, "O CODIGO BOM CONTRA OS SISTEMAS MALIGNOS", LARGURA_TELA / 2,
                                 160, 1, {160, 150, 210, 255});

    // O heroi de um lado e as ameacas do outro, flutuando sobre a grade.
    const float pulo = std::abs(std::sin(this->tempo * 2.2f)) * 10.0f;
    FundoMenu::preencher(r, {116, 420 + static_cast<int>(pulo * 0.3f), 88, 8}, {0, 0, 0, 120});
    tux.desenhar(r, {100.0f, 296.0f - pulo, 120.0f, 152.0f});

    struct Ameaca
    {
        TipoInimigo tipo;
        Afinidade afinidade;
        float x, y, lado, fase;
    };
    const Ameaca ameacas[] = {
        {TipoInimigo::BRUTO, Afinidade::AZUL, 760, 300, 96, 0.0f},
        {TipoInimigo::VIRUS, Afinidade::VERMELHA, 700, 236, 56, 1.3f},
        {TipoInimigo::VIRUS, Afinidade::VERDE, 860, 250, 56, 2.4f},
        {TipoInimigo::VIRUS, Afinidade::VERMELHA, 870, 380, 48, 3.1f},
    };
    for (const Ameaca &a : ameacas)
    {
        const float oscila = std::sin(this->tempo * 1.8f + a.fase) * 8.0f;
        const float deriva = std::cos(this->tempo * 0.9f + a.fase) * 6.0f;
        SpriteInimigo::desenhar(r, a.tipo, a.afinidade,
                                {a.x + deriva, a.y + oscila, a.lado, a.lado});
    }

    for (int i = 0; i < NUM_OPCOES_PRINCIPAIS; ++i)
    {
        this->desenhar_botao(r, this->retangulo_opcao(i), OPCOES_PRINCIPAIS[i],
                             i == this->selecionado, false, 3);
    }
}

void Menu::desenhar_manual(SDL_Renderer *r, const SDL_Rect &painel) const
{
    const int centroX = painel.x + painel.w / 2;
    Texto::desenhar_centralizado(r, "MANUAL DO JOGADOR", centroX, painel.y + 18, 3, COR_TEXTO);
    const char *abas[] = {"CONTROLES", "COMO JOGAR"};
    for (int i = 0; i < 2; ++i)
    {
        const SDL_Rect area = this->retangulo_aba(i);
        const bool ativa = i == this->abaManual;
        FundoMenu::preencher(r, area,
                             ativa ? SDL_Color{50, 40, 110, 240} : SDL_Color{16, 18, 40, 200});
        SDL_SetRenderDrawColor(r, ativa ? 90 : 70, ativa ? 220 : 80, ativa ? 255 : 130, 220);
        SDL_RenderDrawRect(r, &area);
        if (ativa)
        {
            FundoMenu::preencher(r, {area.x, area.y + area.h - 3, area.w, 3}, {255, 80, 200, 255});
        }
        Texto::desenhar_centralizado(r, abas[i], area.x + area.w / 2, area.y + 9, 2,
                                     ativa ? SDL_Color{255, 255, 255, 255}
                                           : SDL_Color{150, 160, 190, 255});
    }
    Texto::desenhar(r, "<", this->retangulo_aba(0).x - 26, painel.y + 63, 2, COR_TEXTO_APAGADO);
    Texto::desenhar(r, ">", this->retangulo_aba(1).x + this->retangulo_aba(1).w + 14, painel.y + 63,
                    2, COR_TEXTO_APAGADO);

    if (this->abaManual == 0)
    {
        int y = painel.y + 100;
        for (const Comando &comando : COMANDOS)
        {
            int x = painel.x + 36;
            for (const char *tecla : comando.teclas)
            {
                x += desenhar_tecla(r, tecla, x, y) + 6;
            }
            const int colunaTexto = painel.x + 260;
            Texto::desenhar(r, comando.acao, colunaTexto, y + 1, 2, COR_DOURADO);
            Texto::desenhar(r, comando.detalhe, colunaTexto, y + 19, 1, {180, 188, 214, 255});
            y += 38;
        }
    }
    else
    {
        int y = painel.y + 102;
        for (const Secao &secao : SECOES_COMO_JOGAR)
        {
            FundoMenu::preencher(r, {painel.x + 36, y + 2, 10, 10}, secao.cor);
            Texto::desenhar(r, secao.titulo, painel.x + 56, y, 2, secao.cor);
            if (&secao == &SECOES_COMO_JOGAR[1])
            {
                // Amostras das tres afinidades ao lado do titulo.
                const int amostraX = painel.x + 56 + Texto::largura(secao.titulo, 2) + 16;
                const SDL_Color cores[] = {
                    {235, 70, 80, 255}, {70, 140, 255, 255}, {80, 220, 110, 255}};
                for (int i = 0; i < 3; ++i)
                {
                    FundoMenu::preencher(r, {amostraX + i * 22, y, 16, 14}, cores[i]);
                }
            }
            y += 20;
            for (const std::string &linha : Texto::quebrar_linhas(secao.texto, painel.w - 100, 2))
            {
                Texto::desenhar(r, linha, painel.x + 56, y, 2, {205, 212, 232, 255});
                y += 18;
            }
            y += 10;
        }
    }
    this->desenhar_botao(r, this->retangulo_opcao(0), "VOLTAR", true, false, 2);
    Texto::desenhar(r, "SETAS LATERAIS TROCAM A ABA", painel.x + 20, painel.y + painel.h - 26, 1,
                    COR_TEXTO_APAGADO);
}

void Menu::desenhar_configuracoes(SDL_Renderer *r, const SDL_Rect &painel) const
{
    const int centroX = painel.x + painel.w / 2;
    Texto::desenhar_centralizado(r, "CONFIGURACOES", centroX, painel.y + 22, 4, COR_TEXTO);
    for (int i = 0; i < this->get_quantidadeOpcoes(); ++i)
    {
        const SDL_Rect area = this->retangulo_opcao(i);
        const bool ativa = i == this->selecionado;
        const ItemConfig item = this->item_config(i);
        if (item == ItemConfig::VOLTAR)
        {
            this->desenhar_botao(r, area, "VOLTAR", ativa, false, 2);
            continue;
        }
        this->desenhar_botao(r, area, "", ativa, false, 2);
        const char *rotulos[] = {"VOLUME DA MUSICA", "VOLUME DOS EFEITOS", "TELA CHEIA",
                                 "MOSTRAR HITBOXES"};
        const int textoY = area.y + (area.h - Texto::altura(2)) / 2;
        Texto::desenhar(r, rotulos[static_cast<int>(item)], area.x + 20, textoY, 2,
                        ativa ? SDL_Color{255, 255, 255, 255} : SDL_Color{180, 190, 215, 255});

        const int controleX = area.x + area.w - 200;
        Texto::desenhar(r, "<", controleX - 18, textoY, 2, ativa ? COR_DOURADO : COR_TEXTO_APAGADO);
        Texto::desenhar(r, ">", area.x + area.w - 22, textoY, 2,
                        ativa ? COR_DOURADO : COR_TEXTO_APAGADO);
        if (item == ItemConfig::VOLUME_MUSICA || item == ItemConfig::VOLUME_EFEITOS)
        {
            const int valor = item == ItemConfig::VOLUME_MUSICA ? this->configuracoes.volumeMusica
                                                                : this->configuracoes.volumeEfeitos;
            for (int barra = 0; barra < Configuracoes::VOLUME_MAXIMO; ++barra)
            {
                const int altura = 8 + barra * 2;
                const SDL_Rect bloco = {controleX + barra * 15, area.y + area.h - 12 - altura, 11,
                                        altura};
                const float t = static_cast<float>(barra) / (Configuracoes::VOLUME_MAXIMO - 1);
                FundoMenu::preencher(
                    r, bloco,
                    barra < valor ? SDL_Color{canal(80 + 175 * t), canal(230 - 150 * t), 255, 255}
                                  : SDL_Color{40, 44, 76, 255});
            }
        }
        else
        {
            const bool ligado = item == ItemConfig::TELA_CHEIA
                                    ? this->configuracoes.telaCheia
                                    : this->configuracoes.mostrarHitboxes;
            const SDL_Rect chave = {controleX + 30, area.y + 11, 96, area.h - 22};
            FundoMenu::preencher(r, chave,
                                 ligado ? SDL_Color{40, 150, 90, 255} : SDL_Color{70, 40, 60, 255});
            const SDL_Rect botao = {ligado ? chave.x + chave.w - 30 : chave.x + 2, chave.y + 2, 28,
                                    chave.h - 4};
            FundoMenu::preencher(r, botao, {230, 235, 245, 255});
            Texto::desenhar(r, ligado ? "SIM" : "NAO", ligado ? chave.x + 12 : chave.x + 44,
                            chave.y + (chave.h - Texto::altura(2)) / 2, 2,
                            ligado ? COR_VERDE_OK : SDL_Color{255, 140, 150, 255});
        }
    }
}

void Menu::desenhar_sobre(SDL_Renderer *r, const SDL_Rect &painel) const
{
    const int centroX = painel.x + painel.w / 2;
    Texto::desenhar_centralizado(r, "SOBRE", centroX, painel.y + 16, 3, COR_TEXTO);
    Texto::desenhar_centralizado(r, "M.C.G.A - MAKE COMPUTERS GREAT AGAIN", centroX, painel.y + 46,
                                 2, COR_DOURADO);
    Texto::desenhar_centralizado(r,
                                 "UM ROGUELIKE EM QUE O TUX E O CODIGO BOM ENFRENTAM VIRUS, "
                                 "MALWARES E SISTEMAS FECHADOS",
                                 centroX, painel.y + 70, 1, {200, 200, 230, 255});
    Texto::desenhar_centralizado(r, "PROJETO SEMESTRAL DE ALGORITMOS E ESTRUTURAS DE DADOS 1",
                                 centroX, painel.y + 84, 1, COR_TEXTO_APAGADO);

    // Coluna das tecnologias.
    const int esquerda = painel.x + 30;
    Texto::desenhar(r, "TECNOLOGIAS", esquerda, painel.y + 108, 2, COR_CIANO);
    int y = painel.y + 132;
    for (const Tecnologia &tecnologia : TECNOLOGIAS)
    {
        FundoMenu::preencher(r, {esquerda, y, 420, 34}, {20, 22, 48, 200});
        FundoMenu::preencher(r, {esquerda, y, 4, 34}, tecnologia.cor);
        Texto::desenhar(r, tecnologia.nome, esquerda + 14, y + 4, 2, tecnologia.cor);
        Texto::desenhar(r, tecnologia.uso, esquerda + 14, y + 22, 1, {190, 196, 220, 255});
        y += 38;
    }

    // Coluna da equipe e creditos.
    const int direita = painel.x + 480;
    Texto::desenhar(r, "EQUIPE", direita, painel.y + 108, 2, COR_CIANO);
    y = painel.y + 136;
    for (const char *nome : INTEGRANTES)
    {
        const float brilho = 0.5f + 0.5f * std::sin(this->tempo * 2.0f + y * 0.05f);
        FundoMenu::preencher(r, {direita, y + 3, 8, 8}, {255, canal(120 + 100 * brilho), 200, 255});
        Texto::desenhar(r, nome, direita + 18, y, 2, {235, 238, 250, 255});
        y += 28;
    }
    y += 14;
    Texto::desenhar(r, "CREDITOS", direita, y, 2, COR_CIANO);
    y += 26;
    const char *creditos[] = {
        "TUX, O MASCOTE DO LINUX, FOI CRIADO POR", "LARRY EWING COM O GIMP.",     "",
        "SONS E MUSICAS SAO SINTETIZADOS PELO",    "PROPRIO JOGO, NOTA A NOTA.",  "",
        "FONTE BITMAP E PIXEL ART DOS INIMIGOS",   "DESENHADAS DIRETO NO CODIGO."};
    for (const char *linha : creditos)
    {
        Texto::desenhar(r, linha, direita, y, 1, {190, 196, 220, 255});
        y += 12;
    }
    this->desenhar_botao(r, this->retangulo_opcao(0), "VOLTAR", true, false, 2);
}

void Menu::configurar_inicial(bool podeContinuar, const std::string &detalhe, bool telaJogar)
{
    this->podeContinuar = podeContinuar;
    this->detalhe = detalhe;
    if (telaJogar)
    {
        this->abrir_tela(TelaMenu::JOGAR, podeContinuar ? 1 : 0);
    }
    else
    {
        this->abrir_tela(TelaMenu::PRINCIPAL, 0);
    }
}

void Menu::configurar_pausa(const std::string &detalhe)
{
    this->tela = TelaMenu::PAUSA;
    this->detalhe = detalhe;
    this->reiniciar();
}

void Menu::configurar_slots(bool novoJogo,
                            const std::array<std::string, QUANTIDADE_SLOTS_SAVE> &detalhes,
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
