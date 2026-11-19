#include "interface/TelaVitoria.h"

#include <algorithm>
#include <cmath>

#include "audio/Som.h"
#include "config/Config.h"
#include "graficos/Sprite.h"
#include "interface/FundoMenu.h"
#include "interface/Texto.h"

namespace
{
constexpr float DURACAO_CELEBRACAO = 11.0f; // depois disso segue sozinha
constexpr float ESPERA_AVISO = 2.5f;        // so entao Enter avanca
constexpr float INTERVALO_FOGOS = 0.7f;
constexpr float VELOCIDADE_TEXTO = 24.0f; // pixels por segundo
constexpr float FATOR_ACELERADO = 6.0f;
constexpr int LARGURA_TEXTO = 700;
constexpr int ALTURA_LINHA = 22;
constexpr int PARAGRAFO = 26;

// As duas "vozes" do texto final se alternam pelas cores.
constexpr SDL_Color VOZ_CIANO = {140, 225, 255, 255};
constexpr SDL_Color VOZ_DOURADA = {255, 215, 140, 255};

struct Trecho
{
    int voz; // 0 ciano, 1 dourada, 2 destaque branco, 3 destaque dourado
    const char *texto;
};

const Trecho TEXTO_FINAL[] = {
    {0, "VOCE CONSEGUIU. O ULTIMO SISTEMA FECHADO CAIU, E O CODIGO-FONTE DO MUNDO VOLTOU A SER "
        "DE TODOS."},
    {1, "MAS NAO FOI SO UM JOGO, FOI? CADA SALA LIMPA FOI UM BUG A MENOS NA SUA HISTORIA."},
    {0, "A VIDA TAMBEM E UM PROJETO DE CODIGO ABERTO. OUTRAS PESSOAS LEEM, CONTRIBUEM, ABREM "
        "ISSUES. E AS VEZES ALGUEM MANDA UM PULL REQUEST QUE MUDA TUDO."},
    {1, "NEM TODO DIA COMPILA DE PRIMEIRA. TUDO BEM. ATE O MELHOR CODIGO JA FOI UM RASCUNHO "
        "CHEIO DE WARNINGS."},
    {0, "QUANDO ALGO QUEBRAR, LEIA A MENSAGEM DE ERRO COM CALMA. ELA ESTA TENTANDO TE ENSINAR "
        "ALGUMA COISA."},
    {1, "NAO EXISTE GIT RESET PARA O PASSADO. MAS SEMPRE EXISTE UM NOVO COMMIT ESPERANDO POR "
        "VOCE."},
    {0, "FACA COMMITS PEQUENOS E FREQUENTES. UM GESTO GENTIL POR DIA. UMA LINHA A MAIS DE "
        "CORAGEM."},
    {1, "E ESCREVA MENSAGENS DE COMMIT QUE VOCE TENHA ORGULHO DE RELER DAQUI A DEZ ANOS."},
    {0, "VOCE NAO PRECISA CARREGAR TUDO NA MEMORIA. PECA AJUDA. ATE O KERNEL DIVIDE O TRABALHO "
        "EM VARIOS PROCESSOS."},
    {1, "DESCANSAR NAO E TRAVAR. E SO UM SLEEP PARA O SISTEMA VOLTAR MAIS RAPIDO."},
    {0, "AS PESSOAS QUE VOCE AMA NAO SAO DEPENDENCIAS OPCIONAIS. CUIDE DELAS. MANTENHA TUDO "
        "ATUALIZADO."},
    {1, "SEU VALOR NAO E MEDIDO EM LINHAS DE CODIGO, NEM EM PONTOS, NEM EM ESTRELAS NO "
        "REPOSITORIO."},
    {0, "ELE ESTA NO QUE VOCE COMPARTILHA. CODIGO BOM E AQUELE QUE OUTRAS PESSOAS CONSEGUEM "
        "LER, USAR E MELHORAR."},
    {1, "VAO EXISTIR OUTROS VIRUS. OUTROS SISTEMAS FECHADOS. OUTROS DIAS DE TELA AZUL."},
    {0, "E VOCE VAI REINICIAR, ABRIR O TERMINAL E TENTAR DE NOVO. EXATAMENTE COMO FEZ AQUI."},
    {1, "PORQUE A VIDA NAO TEM VERSAO FINAL. ELA ESTA SEMPRE EM DESENVOLVIMENTO, E CADA VERSAO "
        "SUA E MELHOR QUE A ANTERIOR, MESMO QUANDO O CHANGELOG PARECE PEQUENO."},
    {0, "AGORA VA. O MUNDO LA FORA TAMBEM PRECISA DE CODIGO BOM."},
    {2, "SEUS COMMITS NA VIDA VALEM A PENA."},
    {3, "CONTINUE COMPILANDO SEUS SONHOS."},
};

const char *EQUIPE[] = {"CALVIN DA SERRA OGATA DE PAULA", "FELIPE BARATELA PIMENTA",
                        "JOAO PEDRO TOMAZ DOS SANTOS", "PEDRO TESTA LOURENCO"};

Uint8 canal(float valor)
{
    return static_cast<Uint8>(std::clamp(valor, 0.0f, 255.0f));
}

float sorteio(int semente)
{
    return FundoMenu::sorteio(semente);
}

// Um fogo de artificio: estoura num ponto e as faiscas caem com gravidade.
void desenhar_fogo(SDL_Renderer *r, int indice, float idade)
{
    constexpr float VIDA = 1.5f;
    if (idade < 0.0f || idade > VIDA)
    {
        return;
    }
    const float cx = 120 + sorteio(indice * 5 + 1) * (LARGURA_TELA - 240);
    const float cy = 60 + sorteio(indice * 5 + 2) * 170;
    const SDL_Color cores[] = {{255, 90, 200, 255},
                               {90, 220, 255, 255},
                               {255, 220, 90, 255},
                               {120, 255, 150, 255},
                               {255, 120, 90, 255}};
    const SDL_Color cor = cores[static_cast<int>(sorteio(indice * 5 + 3) * 5) % 5];
    const float desbota = 1.0f - idade / VIDA;
    constexpr int FAISCAS = 30;
    for (int i = 0; i < FAISCAS; ++i)
    {
        const float angulo = static_cast<float>(i) / FAISCAS * 6.2831853f;
        const float velocidade = 90 + sorteio(indice * 97 + i) * 70;
        const float x = cx + std::cos(angulo) * velocidade * idade;
        const float y = cy + std::sin(angulo) * velocidade * idade + 60 * idade * idade;
        const int lado = idade < 0.3f ? 4 : 3;
        FundoMenu::preencher(r, {static_cast<int>(x), static_cast<int>(y), lado, lado},
                             {cor.r, cor.g, cor.b, canal(255 * desbota)});
        // Rastro curto atras de cada faisca.
        FundoMenu::preencher(r,
                             {static_cast<int>(cx + std::cos(angulo) * velocidade * idade * 0.85f),
                              static_cast<int>(cy + std::sin(angulo) * velocidade * idade * 0.85f +
                                               50 * idade * idade),
                              2, 2},
                             {255, 255, 255, canal(120 * desbota)});
    }
    if (idade < 0.12f)
    {
        FundoMenu::preencher(r, {static_cast<int>(cx) - 10, static_cast<int>(cy) - 10, 20, 20},
                             {255, 255, 255, canal(255 * (1.0f - idade / 0.12f))});
    }
}
} // namespace

void TelaVitoria::iniciar(int pontuacao)
{
    *this = TelaVitoria();
    this->pontuacao = pontuacao;
    this->montar_texto();
}

void TelaVitoria::montar_texto()
{
    auto espaco = [this](int altura)
    {
        Bloco bloco;
        bloco.altura = altura;
        this->blocos.push_back(bloco);
    };
    auto linhas = [this](const std::string &texto, int escala, SDL_Color cor)
    {
        for (const std::string &linha : Texto::quebrar_linhas(texto, LARGURA_TEXTO, escala))
        {
            Bloco bloco;
            bloco.tipo = Bloco::Tipo::TEXTO;
            bloco.texto = linha;
            bloco.escala = escala;
            bloco.cor = cor;
            bloco.altura = escala == 2 ? ALTURA_LINHA : Texto::altura(escala) + 10;
            this->blocos.push_back(bloco);
        }
    };

    for (const Trecho &trecho : TEXTO_FINAL)
    {
        if (trecho.voz >= 2)
        {
            espaco(40);
            linhas(trecho.texto, 3, trecho.voz == 2 ? SDL_Color{255, 255, 255, 255} : VOZ_DOURADA);
            continue;
        }
        linhas(trecho.texto, 2, trecho.voz == 0 ? VOZ_CIANO : VOZ_DOURADA);
        espaco(PARAGRAFO);
    }

    // Creditos logo depois da mensagem.
    espaco(170);
    Bloco logo;
    logo.tipo = Bloco::Tipo::LOGO;
    logo.altura = 90;
    this->blocos.push_back(logo);
    linhas("MAKE COMPUTERS GREAT AGAIN", 2, {210, 196, 255, 255});
    espaco(30);
    linhas("UM JOGO DE", 1, {150, 150, 190, 255});
    for (const char *nome : EQUIPE)
    {
        linhas(nome, 2, {235, 238, 250, 255});
    }
    espaco(30);
    linhas("FEITO COM C++17, SDL2 E MUITO CAFE", 1, {150, 150, 190, 255});
    linhas("PROJETO DE ALGORITMOS E ESTRUTURAS DE DADOS 1", 1, {150, 150, 190, 255});
    espaco(40);
    linhas("OBRIGADO POR JOGAR!", 4, VOZ_DOURADA);

    // O texto para quando o agradecimento chega perto do meio da tela.
    int total = 0;
    for (const Bloco &bloco : this->blocos)
    {
        total += bloco.altura;
    }
    const int ultimo = this->blocos.back().altura;
    this->rolagemFinal = static_cast<float>(ALTURA_TELA + total - ultimo - 410);
}

void TelaVitoria::atualizar(float dt, bool acelerar)
{
    this->tempo += dt;
    this->tempoEtapa += dt;
    switch (this->etapa)
    {
    case Etapa::CELEBRACAO:
    {
        const int fogos = static_cast<int>(this->tempoEtapa / INTERVALO_FOGOS);
        if (fogos > this->fogosDisparados)
        {
            this->fogosDisparados = fogos;
            Som::tocar(EfeitoSonoro::FOGOS, 0.8f);
        }
        if (this->tempoEtapa >= DURACAO_CELEBRACAO)
        {
            this->avancar();
        }
        break;
    }
    case Etapa::MENSAGEM:
        this->rolagem += dt * VELOCIDADE_TEXTO * (acelerar ? FATOR_ACELERADO : 1.0f);
        if (this->rolagem >= this->rolagemFinal)
        {
            this->rolagem = this->rolagemFinal;
            this->etapa = Etapa::CREDITOS;
            this->tempoEtapa = 0.0f;
        }
        break;
    case Etapa::CREDITOS:
        break;
    }
}

void TelaVitoria::avancar()
{
    if (this->etapa == Etapa::CELEBRACAO)
    {
        if (this->tempoEtapa < ESPERA_AVISO && this->tempoEtapa < DURACAO_CELEBRACAO)
        {
            return;
        }
        Som::tocar(EfeitoSonoro::MENU_CONFIRMAR);
        this->etapa = Etapa::MENSAGEM;
        this->tempoEtapa = 0.0f;
    }
    else if (this->etapa == Etapa::CREDITOS && this->tempoEtapa > 1.0f)
    {
        this->encerrada = true;
    }
}

void TelaVitoria::pular()
{
    if (this->etapa == Etapa::CREDITOS)
    {
        this->encerrada = this->tempoEtapa > 1.0f;
        return;
    }
    this->etapa = Etapa::CREDITOS;
    this->rolagem = this->rolagemFinal;
    this->tempoEtapa = 0.0f;
}

bool TelaVitoria::terminou() const
{
    return this->encerrada;
}

void TelaVitoria::desenhar(SDL_Renderer *r, const Sprite &tux) const
{
    if (this->etapa == Etapa::CELEBRACAO)
    {
        this->desenhar_celebracao(r, tux);
    }
    else
    {
        this->desenhar_mensagem(r);
    }
    // Transicao: escurece no inicio do texto.
    if (this->etapa == Etapa::MENSAGEM && this->tempoEtapa < 1.0f)
    {
        FundoMenu::preencher(r, {0, 0, LARGURA_TELA, ALTURA_TELA},
                             {0, 0, 0, canal(255 * (1.0f - this->tempoEtapa))});
    }
}

void TelaVitoria::desenhar_celebracao(SDL_Renderer *r, const Sprite &tux) const
{
    const float t = this->tempoEtapa;
    FundoMenu::desenhar(r, this->tempo);
    for (int i = 0; i <= this->fogosDisparados; ++i)
    {
        desenhar_fogo(r, i, t - i * INTERVALO_FOGOS);
    }

    // Confete caindo pela tela toda.
    const SDL_Color confetes[] = {
        {255, 90, 200, 255}, {90, 220, 255, 255}, {255, 220, 90, 255}, {120, 255, 150, 255}};
    for (int i = 0; i < 70; ++i)
    {
        const float velocidade = 50 + sorteio(i + 300) * 70;
        const float y = std::fmod(t * velocidade + sorteio(i + 400) * 600, 600.0f) - 40;
        const float x = sorteio(i + 500) * LARGURA_TELA + std::sin(t * 2 + i) * 14;
        const bool deitado = std::fmod(t * 3 + i, 1.0f) < 0.5f;
        FundoMenu::preencher(
            r, {static_cast<int>(x), static_cast<int>(y), deitado ? 7 : 3, deitado ? 3 : 7},
            confetes[i % 4]);
    }

    const int centro = LARGURA_TELA / 2;
    const float entrada = std::min(1.0f, t / 0.6f);
    const int tituloY = 40 + static_cast<int>((1.0f - entrada) * -60);
    const float pulso = 0.5f + 0.5f * std::sin(t * 3.0f);
    Texto::desenhar_centralizado(r, "VITORIA!", centro + 6, tituloY + 6, 11, {60, 10, 70, 220});
    Texto::desenhar_centralizado(r, "VITORIA!", centro - 3, tituloY, 11, {40, 220, 255, 170});
    Texto::desenhar_centralizado(r, "VITORIA!", centro + 3, tituloY, 11, {255, 40, 170, 170});
    Texto::desenhar_centralizado(r, "VITORIA!", centro, tituloY, 11,
                                 {255, canal(215 + 40 * pulso), 110, 255});

    if (t > 0.6f)
    {
        const float surgir = std::min(1.0f, (t - 0.6f) / 0.6f);
        Texto::desenhar_centralizado(r, "PARABENS, GUERREIRO DO CODIGO ABERTO!", centro, 136, 3,
                                     {255, 255, 255, canal(255 * surgir)});
    }
    if (t > 1.2f)
    {
        const float surgir = std::min(1.0f, (t - 1.2f) / 0.6f);
        const SDL_Rect painel = {130, 172, 700, 112};
        FundoMenu::desenhar_painel(r, painel, this->tempo);
        const char *texto = "VOCE LIVROU O MUNDO DOS CODIGOS FECHADOS E MALICIOSOS. OS VIRUS "
                            "FORAM DELETADOS, OS SISTEMAS FECHADOS TIVERAM SEU CODIGO LIBERTADO "
                            "E O TUX PODE, ENFIM, DESCANSAR.";
        int y = painel.y + 18;
        for (const std::string &linha : Texto::quebrar_linhas(texto, painel.w - 50, 2))
        {
            Texto::desenhar_centralizado(r, linha, centro, y, 2,
                                         {210, 225, 250, canal(255 * surgir)});
            y += 22;
        }
    }

    // Tux comemorando aos pulos.
    const float pulo = std::abs(std::sin(t * 4.5f)) * 26.0f;
    FundoMenu::preencher(r, {centro - 32, 398 + static_cast<int>(pulo * 0.2f), 64, 7},
                         {0, 0, 0, 120});
    tux.desenhar(r, {static_cast<float>(centro) - 50.0f, 318.0f - pulo, 100.0f, 127.0f});

    if (t > 1.8f)
    {
        Texto::desenhar_centralizado(r, "PONTUACAO FINAL: " + std::to_string(this->pontuacao),
                                     centro, 470, 3, {255, 222, 110, 255});
    }
    if (t > ESPERA_AVISO && std::fmod(t, 1.0f) < 0.65f)
    {
        Texto::desenhar_centralizado(r, "ENTER: CONTINUAR", centro, 512, 2, {200, 200, 230, 255});
    }
}

void TelaVitoria::desenhar_mensagem(SDL_Renderer *r) const
{
    // Ceu escuro com estrelas que derivam devagar.
    for (int y = 0; y < ALTURA_TELA; y += 6)
    {
        const float t = static_cast<float>(y) / ALTURA_TELA;
        FundoMenu::preencher(r, {0, y, LARGURA_TELA, 6},
                             {canal(4 + 14 * t), canal(4 + 4 * t), canal(14 + 24 * t), 255});
    }
    for (int i = 0; i < 120; ++i)
    {
        const float velocidade = 3 + sorteio(i + 700) * 10;
        const float y =
            std::fmod(sorteio(i + 800) * ALTURA_TELA - this->tempo * velocidade + ALTURA_TELA * 10,
                      static_cast<float>(ALTURA_TELA));
        const float brilho = 0.5f + 0.5f * std::sin(this->tempo * (1 + sorteio(i)) + i);
        const int lado = sorteio(i + 900) > 0.9f ? 2 : 1;
        FundoMenu::preencher(
            r, {static_cast<int>(sorteio(i + 600) * LARGURA_TELA), static_cast<int>(y), lado, lado},
            {200, 210, 255, canal(50 + 150 * brilho)});
    }

    // Texto subindo, com as bordas de cima e de baixo esmaecidas.
    float y = ALTURA_TELA - this->rolagem;
    const int centro = LARGURA_TELA / 2;
    for (const Bloco &bloco : this->blocos)
    {
        if (y > -100 && y < ALTURA_TELA + 20 && bloco.tipo != Bloco::Tipo::ESPACO)
        {
            float alfa = 1.0f;
            if (y < 60)
            {
                alfa = std::max(0.0f, (y + 10) / 70.0f);
            }
            else if (y > ALTURA_TELA - 90)
            {
                alfa = std::max(0.0f, (ALTURA_TELA - y) / 90.0f);
            }
            if (bloco.tipo == Bloco::Tipo::LOGO)
            {
                FundoMenu::desenhar_logo(r, centro, static_cast<int>(y), 9, this->tempo,
                                         canal(255 * alfa));
            }
            else
            {
                Texto::desenhar_centralizado(
                    r, bloco.texto, centro, static_cast<int>(y), bloco.escala,
                    {bloco.cor.r, bloco.cor.g, bloco.cor.b, canal(255 * alfa)});
            }
        }
        y += static_cast<float>(bloco.altura);
    }

    if (this->etapa == Etapa::MENSAGEM && this->tempoEtapa < 7.0f)
    {
        const float alfa = std::min(1.0f, (7.0f - this->tempoEtapa) / 1.5f);
        Texto::desenhar(r, "SEGURE ESPACO PARA ACELERAR - ESC PULA", 16, ALTURA_TELA - 18, 1,
                        {120, 120, 160, canal(200 * alfa)});
    }
    if (this->etapa == Etapa::CREDITOS && this->tempoEtapa > 1.0f &&
        std::fmod(this->tempoEtapa, 1.0f) < 0.65f)
    {
        Texto::desenhar_centralizado(r, "ENTER: VOLTAR AO MENU", centro, ALTURA_TELA - 34, 2,
                                     {200, 200, 230, 255});
    }
}
