#include "interface/TelaDerrota.h"

#include <algorithm>
#include <cmath>
#include <string>
#include <vector>

#include "audio/Som.h"
#include "config/Config.h"
#include "entidades/Inimigo.h"
#include "graficos/SpriteInimigo.h"
#include "interface/FundoMenu.h"
#include "interface/Texto.h"

namespace
{
// Linha do tempo da animacao, em segundos.
constexpr float INICIO_ERROS = 0.8f;
constexpr float INICIO_ESCURO = 2.0f;
constexpr float FIM_ESCURO = 2.8f;
constexpr float INICIO_TITULO = 2.4f;
constexpr float INICIO_FRASE = 2.8f;
constexpr float INICIO_INCENTIVO = 4.2f;
constexpr float INICIO_TROCADILHO = 4.8f;
constexpr float INICIO_VIDAS = 5.4f;
constexpr float INICIO_AVISO = 6.0f;

constexpr int QUANTIDADE_VIRUS = 36;
constexpr int QUANTIDADE_JANELAS = 8;

const char *FRASE_INVASAO = "OS CODIGOS MALICIOSOS TOMARAM CONTA DO SEU SISTEMA";

const char *TROCADILHOS[] = {
    "TODO CODIGO BOM JA FALHOU NOS TESTES UM DIA.",
    "ERRO NAO E FIM DE PROGRAMA. E SO UMA EXCECAO ESPERANDO TRATAMENTO.",
    "CTRL+Z NA DERROTA, CTRL+S NA DETERMINACAO.",
    "UM TUX DE VERDADE NUNCA DA SEGMENTATION FAULT NA CORAGEM.",
    "REINICIE, RECOMPILE E RODE DE NOVO. VOCE CONSEGUE.",
    "O DEBUGGER MAIS PODEROSO QUE EXISTE SE CHAMA PERSISTENCIA.",
    "CAIR FAZ PARTE DO LOOP. DESISTIR E QUE SERIA UM BREAK.",
};
constexpr int QUANTIDADE_TROCADILHOS =
    static_cast<int>(sizeof(TROCADILHOS) / sizeof(TROCADILHOS[0]));

const char *PROGRAMAS[] = {"VIRUS.EXE", "TROJAN.DLL", "RANSOM.BAT", "SPYWARE.SYS",
                           "WORM.COM",  "ROOTKIT.SO", "ADWARE.MSI", "KEYLOGGER.EXE"};

Uint8 canal(float valor)
{
    return static_cast<Uint8>(std::clamp(valor, 0.0f, 255.0f));
}

float sorteio(int semente)
{
    return FundoMenu::sorteio(semente);
}

std::string hexadecimal(int valor)
{
    const char *digitos = "0123456789ABCDEF";
    std::string texto = "0X";
    for (int i = 7; i >= 0; --i)
    {
        texto += digitos[(valor >> (i * 4)) & 0xF];
    }
    return texto;
}

// Faixas horizontais deslocadas e blocos de pixels mortos, como sinal corrompido.
void desenhar_falhas(SDL_Renderer *r, float tempo, int semente, float intensidade)
{
    const int quadro = static_cast<int>(tempo * 16.0f);
    const int faixas = static_cast<int>(4 + 22 * intensidade);
    for (int i = 0; i < faixas; ++i)
    {
        const int base = semente + quadro * 97 + i * 13;
        const int y = static_cast<int>(sorteio(base) * ALTURA_TELA);
        const int altura = 2 + static_cast<int>(sorteio(base + 1) * 14);
        const int x = static_cast<int>((sorteio(base + 2) - 0.5f) * 120);
        const float tipo = sorteio(base + 3);
        const SDL_Color cor =
            tipo < 0.4f ? SDL_Color{255, 30, 60, 255}
                        : (tipo < 0.7f ? SDL_Color{40, 230, 255, 255} : SDL_Color{0, 0, 0, 255});
        FundoMenu::preencher(r, {x, y, LARGURA_TELA, altura},
                             {cor.r, cor.g, cor.b, canal(150 * intensidade)});
    }
    const int blocos = static_cast<int>(60 * intensidade);
    for (int i = 0; i < blocos; ++i)
    {
        const int base = semente + quadro * 31 + i * 7 + 5000;
        const int lado = 4 + static_cast<int>(sorteio(base) * 12);
        FundoMenu::preencher(r,
                             {static_cast<int>(sorteio(base + 1) * LARGURA_TELA),
                              static_cast<int>(sorteio(base + 2) * ALTURA_TELA), lado, lado},
                             sorteio(base + 3) < 0.5f ? SDL_Color{255, 0, 120, 200}
                                                      : SDL_Color{0, 255, 140, 200});
    }
}

// Janela de erro falsa, no estilo das caixas de dialogo de sistema.
void desenhar_janela(SDL_Renderer *r, int indice, int semente, float idade)
{
    const int x = 40 + static_cast<int>(sorteio(semente + indice * 3) * (LARGURA_TELA - 330));
    const int y = 40 + static_cast<int>(sorteio(semente + indice * 3 + 1) * (ALTURA_TELA - 170));
    const float surgir = std::min(1.0f, idade / 0.12f);
    const int largura = static_cast<int>(250 * surgir);
    const SDL_Rect janela = {x + (250 - largura) / 2, y, largura, 96};
    FundoMenu::preencher(r, {janela.x + 6, janela.y + 6, janela.w, janela.h}, {0, 0, 0, 120});
    FundoMenu::preencher(r, janela, {205, 205, 215, 255});
    FundoMenu::preencher(r, {janela.x, janela.y, janela.w, 18}, {190, 20, 40, 255});
    if (surgir < 1.0f)
    {
        return;
    }
    Texto::desenhar(r, PROGRAMAS[indice % 8], janela.x + 6, janela.y + 5, 1, {255, 255, 255, 255});
    FundoMenu::preencher(r, {janela.x + janela.w - 16, janela.y + 3, 12, 12}, {230, 80, 90, 255});
    // Icone de alerta: triangulo em degraus com exclamacao.
    for (int linha = 0; linha < 8; ++linha)
    {
        FundoMenu::preencher(
            r, {janela.x + 30 - linha * 2, janela.y + 30 + linha * 3, 4 + linha * 4, 3},
            {250, 200, 40, 255});
    }
    FundoMenu::preencher(r, {janela.x + 31, janela.y + 35, 2, 10}, {30, 20, 20, 255});
    FundoMenu::preencher(r, {janela.x + 31, janela.y + 48, 2, 2}, {30, 20, 20, 255});
    Texto::desenhar(r, "ERRO FATAL", janela.x + 58, janela.y + 30, 2, {150, 0, 20, 255});
    Texto::desenhar(r, hexadecimal(static_cast<int>(sorteio(semente + indice) * 2147483647.0f)),
                    janela.x + 58, janela.y + 50, 1, {40, 40, 60, 255});
    const SDL_Rect botao = {janela.x + janela.w / 2 - 26, janela.y + 68, 52, 18};
    FundoMenu::preencher(r, botao, {170, 170, 182, 255});
    SDL_SetRenderDrawColor(r, 60, 60, 80, 255);
    SDL_RenderDrawRect(r, &botao);
    Texto::desenhar_centralizado(r, "OK", botao.x + botao.w / 2, botao.y + 5, 1, {20, 20, 30, 255});
}

} // namespace

void TelaDerrota::iniciar(int vidasRestantes, int semente)
{
    *this = TelaDerrota();
    this->vidasRestantes = vidasRestantes;
    this->semente = semente;
}

void TelaDerrota::atualizar(float dt)
{
    const float anterior = this->tempo;
    this->tempo += dt;
    if (anterior < INICIO_ERROS && this->tempo >= INICIO_ERROS)
    {
        Som::tocar(EfeitoSonoro::ERRO_SISTEMA);
    }
    if (anterior < INICIO_INCENTIVO && this->tempo >= INICIO_INCENTIVO)
    {
        Som::tocar(EfeitoSonoro::ENCORAJAR);
    }
}

void TelaDerrota::pular()
{
    if (this->tempo < INICIO_PULO)
    {
        return;
    }
    if (this->tempo < INICIO_AVISO)
    {
        // Primeiro Enter mostra a mensagem inteira; o segundo segue em frente.
        if (this->tempo < INICIO_INCENTIVO)
        {
            Som::tocar(EfeitoSonoro::ENCORAJAR);
        }
        this->tempo = INICIO_AVISO;
        return;
    }
    this->encerrada = true;
}

bool TelaDerrota::terminou() const
{
    return this->encerrada || this->tempo >= DURACAO;
}

void TelaDerrota::desenhar(SDL_Renderer *r, bool toque) const
{
    const float t = this->tempo;
    const SDL_Rect tela = {0, 0, LARGURA_TELA, ALTURA_TELA};

    // Clarao do impacto e tinta vermelha crescente sobre a partida congelada.
    if (t < 0.3f)
    {
        FundoMenu::preencher(r, tela, {255, 255, 255, canal(220 * (1.0f - t / 0.3f))});
    }
    FundoMenu::preencher(r, tela, {150, 0, 30, canal(130 * std::min(1.0f, t / 1.6f))});

    // Enxame de virus invadindo a tela pelas bordas.
    for (int i = 0; i < QUANTIDADE_VIRUS; ++i)
    {
        const float nascimento = 0.35f + i * 0.045f;
        if (t < nascimento)
        {
            continue;
        }
        const int base = this->semente + i * 11 + 900;
        const int lado = static_cast<int>(sorteio(base) * 4);
        const float ao = sorteio(base + 1);
        SDL_FPoint origem = {ao * LARGURA_TELA, -60.0f};
        if (lado == 1)
        {
            origem = {ao * LARGURA_TELA, ALTURA_TELA + 60.0f};
        }
        else if (lado == 2)
        {
            origem = {-60.0f, ao * ALTURA_TELA};
        }
        else if (lado == 3)
        {
            origem = {LARGURA_TELA + 60.0f, ao * ALTURA_TELA};
        }
        const SDL_FPoint destino = {60 + sorteio(base + 2) * (LARGURA_TELA - 120),
                                    50 + sorteio(base + 3) * (ALTURA_TELA - 100)};
        const float progresso = std::min(1.0f, (t - nascimento) / 1.1f);
        const float suave = 1.0f - (1.0f - progresso) * (1.0f - progresso);
        const float oscila = std::sin(t * 6.0f + i) * 6.0f;
        const float tamanho = 30 + sorteio(base + 4) * 34;
        const float x = origem.x + (destino.x - origem.x) * suave - tamanho / 2;
        const float y = origem.y + (destino.y - origem.y) * suave - tamanho / 2 + oscila;
        const float cor = sorteio(base + 5);
        const Afinidade afinidade =
            cor < 0.6f ? Afinidade::VERMELHA : (cor < 0.8f ? Afinidade::AZUL : Afinidade::VERDE);
        SpriteInimigo::desenhar(r, TipoInimigo::VIRUS, afinidade, {x, y, tamanho, tamanho});
    }

    // Janelas de erro pipocando.
    for (int i = 0; i < QUANTIDADE_JANELAS; ++i)
    {
        const float surge = INICIO_ERROS + i * 0.2f;
        if (t >= surge)
        {
            desenhar_janela(r, i, this->semente + 4000, t - surge);
        }
    }

    const float falhas = t < INICIO_ESCURO ? std::min(1.0f, t / 1.2f)
                                           : std::max(0.15f, 1.0f - (t - INICIO_ESCURO) * 1.5f);
    desenhar_falhas(r, t, this->semente, falhas);

    // A tela escurece e sobra so a mensagem.
    if (t >= INICIO_ESCURO)
    {
        const float escuro = std::min(1.0f, (t - INICIO_ESCURO) / (FIM_ESCURO - INICIO_ESCURO));
        FundoMenu::preencher(r, tela, {6, 0, 12, canal(232 * escuro)});
        // Linhas de varredura de monitor antigo.
        for (int y = 0; y < ALTURA_TELA; y += 3)
        {
            FundoMenu::preencher(r, {0, y, LARGURA_TELA, 1}, {0, 0, 0, canal(60 * escuro)});
        }
    }

    const int centro = LARGURA_TELA / 2;
    if (t >= INICIO_TITULO)
    {
        const float surgir = std::min(1.0f, (t - INICIO_TITULO) / 0.3f);
        const int quadro = static_cast<int>(t * 14.0f);
        const bool tremor = std::fmod(t, 1.7f) < 0.12f || t - INICIO_TITULO < 0.4f;
        const int desvio = tremor ? static_cast<int>((sorteio(quadro) - 0.5f) * 16) : 0;
        const int y = 92;
        Texto::desenhar_centralizado(r, "OH NAO!", centro - 4 + desvio, y, 8,
                                     {40, 230, 255, canal(150 * surgir)});
        Texto::desenhar_centralizado(r, "OH NAO!", centro + 4 - desvio, y, 8,
                                     {255, 20, 90, canal(150 * surgir)});
        Texto::desenhar_centralizado(r, "OH NAO!", centro, y, 8,
                                     {255, 70, 80, canal(255 * surgir)});
    }
    if (t >= INICIO_FRASE)
    {
        // Efeito de maquina de escrever, com cursor no fim.
        const std::string frase = FRASE_INVASAO;
        const std::size_t letras =
            std::min(frase.size(), static_cast<std::size_t>((t - INICIO_FRASE) * 34.0f));
        const std::string parcial = frase.substr(0, letras);
        const int largura = Texto::largura(frase, 2);
        const int x = centro - largura / 2;
        Texto::desenhar(r, parcial, x, 176, 2, {255, 210, 215, 255});
        if (letras < frase.size() || std::fmod(t, 0.7f) < 0.4f)
        {
            FundoMenu::preencher(r, {x + Texto::largura(parcial, 2) + 4, 176, 10, 14},
                                 {255, 210, 215, 255});
        }
        if (letras == frase.size())
        {
            Texto::desenhar_centralizado(r, "[ ERRO ] TUX.EXE PAROU DE RESPONDER", centro, 202, 1,
                                         {200, 110, 130, 255});
        }
    }
    if (t >= INICIO_INCENTIVO)
    {
        const float surgir = std::min(1.0f, (t - INICIO_INCENTIVO) / 0.5f);
        const float pulso = 0.5f + 0.5f * std::sin(t * 4.0f);
        const int y = 262 + static_cast<int>((1.0f - surgir) * 20);
        Texto::desenhar_centralizado(r, "NAO DESISTA, GUERREIRO!", centro + 3, y + 3, 4,
                                     {90, 40, 0, canal(200 * surgir)});
        Texto::desenhar_centralizado(r, "NAO DESISTA, GUERREIRO!", centro, y, 4,
                                     {255, canal(200 + 40 * pulso), 90, canal(255 * surgir)});
    }
    if (t >= INICIO_TROCADILHO)
    {
        const float surgir = std::min(1.0f, (t - INICIO_TROCADILHO) / 0.6f);
        const char *frase = TROCADILHOS[this->semente % QUANTIDADE_TROCADILHOS];
        int y = 322;
        for (const std::string &linha : Texto::quebrar_linhas(frase, 760, 2))
        {
            Texto::desenhar_centralizado(r, linha, centro, y, 2,
                                         {200, 220, 255, canal(235 * surgir)});
            y += 20;
        }
    }
    if (t >= INICIO_VIDAS)
    {
        const float surgir = std::min(1.0f, (t - INICIO_VIDAS) / 0.5f);
        if (this->vidasRestantes > 0)
        {
            const std::string texto = "VIDAS RESTANTES: " + std::to_string(this->vidasRestantes);
            const int largura = Texto::largura(texto, 2) + this->vidasRestantes * 24 + 10;
            const int x = centro - largura / 2;
            Texto::desenhar(r, texto, x, 392, 2, {255, 140, 150, canal(255 * surgir)});
            for (int i = 0; i < this->vidasRestantes; ++i)
            {
                // Coracao em pixels: dois blocos no topo e um corpo mais largo.
                const int cx = x + Texto::largura(texto, 2) + 16 + i * 24;
                const SDL_Color cor = {240, 60, 80, canal(255 * surgir)};
                FundoMenu::preencher(r, {cx, 392, 6, 4}, cor);
                FundoMenu::preencher(r, {cx + 8, 392, 6, 4}, cor);
                FundoMenu::preencher(r, {cx - 1, 396, 16, 5}, cor);
                FundoMenu::preencher(r, {cx + 2, 401, 10, 3}, cor);
                FundoMenu::preencher(r, {cx + 5, 404, 4, 3}, cor);
            }
        }
        else
        {
            Texto::desenhar_centralizado(r,
                                         "SUAS VIDAS ACABARAM, MAS A LENDA DO TUX NAO TERMINA AQUI",
                                         centro, 392, 2, {255, 140, 150, canal(255 * surgir)});
        }
    }
    if (t >= INICIO_AVISO && std::fmod(t, 0.9f) < 0.6f)
    {
        Texto::desenhar_centralizado(
            r,
            this->vidasRestantes > 0
                ? (toque ? "TOQUE: REINICIAR O SISTEMA" : "ENTER: REINICIAR O SISTEMA")
                : (toque ? "TOQUE: CONTINUAR" : "ENTER: CONTINUAR"),
            centro, 460, 2, {150, 150, 190, 255});
    }
}
