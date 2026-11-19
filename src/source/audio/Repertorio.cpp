#include "audio/Repertorio.h"

#include <cctype>
#include <cstring>
#include <sstream>
#include <string>

using Sintetizador::Nota;
using Sintetizador::Onda;

namespace
{
// Construtor encadeado: N(inicio, duracao, freq, volume).ate(200).queda(10) ...
struct N
{
    Nota nota;
    N(float inicio, float duracao, float freq, float volume, Onda onda = Onda::QUADRADA)
    {
        nota.inicio = inicio;
        nota.duracao = duracao;
        nota.freq = freq;
        nota.volume = volume;
        nota.onda = onda;
    }
    N &ate(float freq)
    {
        nota.freqFinal = freq;
        return *this;
    }
    N &ciclo(float valor)
    {
        nota.ciclo = valor;
        return *this;
    }
    N &queda(float valor)
    {
        nota.decaimento = valor;
        return *this;
    }
    N &filtro(float valor)
    {
        nota.filtro = valor;
        return *this;
    }
    N &vibrato(float profundidade, float hz)
    {
        nota.vibrato = profundidade;
        nota.vibratoHz = hz;
        return *this;
    }
    N &picote(float valor)
    {
        nota.picotar = valor;
        return *this;
    }
    N &ataque(float valor)
    {
        nota.ataque = valor;
        return *this;
    }
    N &liberacao(float valor)
    {
        nota.liberacao = valor;
        return *this;
    }
    operator Nota() const
    {
        return nota;
    }
};

float hz(int midi)
{
    return Sintetizador::frequencia_midi(midi);
}

// "C#5" -> numero MIDI. Aceita sustenido (#) e bemol (b).
int midi(const std::string &nome)
{
    static const int BASE[] = {9, 11, 0, 2, 4, 5, 7}; // A B C D E F G
    const int letra = std::toupper(static_cast<unsigned char>(nome[0])) - 'A';
    int nota = BASE[letra];
    std::size_t i = 1;
    if (i < nome.size() && nome[i] == '#')
    {
        ++nota;
        ++i;
    }
    else if (i < nome.size() && nome[i] == 'b')
    {
        --nota;
        ++i;
    }
    const int oitava = std::stoi(nome.substr(i));
    return (oitava + 1) * 12 + nota;
}

// Arpejo de notas iguais espacadas, usado em vinhetas.
void arpejo(std::vector<Nota> &notas, float inicio, float passo, const std::vector<int> &alturas,
            float duracao, float volume, Onda onda, float ciclo = 0.5f)
{
    for (std::size_t i = 0; i < alturas.size(); ++i)
    {
        notas.push_back(
            N(inicio + passo * static_cast<float>(i), duracao, hz(alturas[i]), volume, onda)
                .ciclo(ciclo)
                .queda(3.0f));
    }
}

// ---------------------------------------------------------------- musica

// Escreve trilhas em compassos 4/4 divididos em 16 semicolcheias.
class Compositor
{
private:
    float semicolcheia;

public:
    std::vector<Nota> notas;

    explicit Compositor(float bpm) : semicolcheia(60.0f / bpm / 4.0f)
    {
    }

    float tempo(int compasso, int passo = 0) const
    {
        return (compasso * 16 + passo) * semicolcheia;
    }

    float duracao(int compassos) const
    {
        return tempo(compassos);
    }

    // Padrao de 16 caracteres: K bumbo, S caixa, h chimbal, o chimbal aberto.
    void bateria(int compasso, const char *padrao, float intensidade = 1.0f)
    {
        for (int passo = 0; passo < 16 && padrao[passo] != '\0'; ++passo)
        {
            const float t = tempo(compasso, passo);
            switch (padrao[passo])
            {
            case 'K':
                notas.push_back(N(t, 0.16f, 150, 0.55f * intensidade, Onda::SENOIDE)
                                    .ate(42)
                                    .queda(14)
                                    .liberacao(0.02f));
                notas.push_back(N(t, 0.012f, 3000, 0.10f * intensidade, Onda::RUIDO));
                break;
            case 'S':
                notas.push_back(
                    N(t, 0.15f, 9000, 0.24f * intensidade, Onda::RUIDO).queda(20).filtro(0.55f));
                notas.push_back(
                    N(t, 0.07f, 240, 0.16f * intensidade, Onda::TRIANGULO).ate(170).queda(25));
                break;
            case 'h':
                notas.push_back(N(t, 0.035f, 22000, 0.055f * intensidade, Onda::RUIDO).queda(70));
                break;
            case 'o':
                notas.push_back(N(t, 0.14f, 22000, 0.05f * intensidade, Onda::RUIDO).queda(18));
                break;
            default:
                break;
            }
        }
    }

    // Melodia em texto: "E5:2 -:2 G5:4" (nota:duracao em semicolcheias, "-" e pausa).
    // O eco repete cada nota mais baixo, tres semicolcheias depois.
    void melodia(int compasso, const std::string &texto, Onda onda, float volume, float ciclo,
                 int transpor = 0, float eco = 0.0f, float filtro = 1.0f)
    {
        std::istringstream fluxo(texto);
        std::string token;
        int passo = compasso * 16;
        while (fluxo >> token)
        {
            const auto separador = token.find(':');
            const int duracao = std::stoi(token.substr(separador + 1));
            const std::string nome = token.substr(0, separador);
            if (nome != "-")
            {
                const float inicio = passo * semicolcheia;
                const float tamanho = duracao * semicolcheia * 0.92f;
                const float freq = hz(midi(nome) + transpor);
                notas.push_back(N(inicio, tamanho, freq, volume, onda)
                                    .ciclo(ciclo)
                                    .vibrato(0.006f, 5.5f)
                                    .filtro(filtro)
                                    .queda(1.2f)
                                    .liberacao(0.04f));
                if (eco > 0.0f)
                {
                    notas.push_back(N(inicio + 3 * semicolcheia, tamanho, freq, volume * eco, onda)
                                        .ciclo(ciclo)
                                        .filtro(filtro * 0.6f)
                                        .queda(2.0f)
                                        .liberacao(0.04f));
                }
            }
            passo += duracao;
        }
    }

    // Baixo pulsante: cada caractere e uma semicolcheia. 'r' toca a fundamental,
    // 'R' uma oitava acima, '5' a quinta e '.' segura em silencio.
    void baixo(int compasso, int raiz, const char *padrao, Onda onda, float volume,
               float filtro = 0.35f)
    {
        for (int passo = 0; passo < 16 && padrao[passo] != '\0'; ++passo)
        {
            int nota = -1;
            switch (padrao[passo])
            {
            case 'r':
                nota = raiz;
                break;
            case 'R':
                nota = raiz + 12;
                break;
            case '5':
                nota = raiz + 7;
                break;
            default:
                break;
            }
            if (nota < 0)
            {
                continue;
            }
            int duracao = 1;
            while (passo + duracao < 16 && padrao[passo + duracao] == '.')
            {
                ++duracao;
            }
            notas.push_back(
                N(tempo(compasso, passo), duracao * semicolcheia * 0.9f, hz(nota), volume, onda)
                    .ciclo(0.5f)
                    .filtro(filtro)
                    .queda(2.5f)
                    .liberacao(0.02f));
        }
    }

    // Arpejo continuo das notas do acorde, uma por semicolcheia.
    void arpejo_acorde(int compasso, const std::vector<int> &acorde, Onda onda, float volume,
                       float ciclo = 0.25f, int passoArpejo = 1)
    {
        for (int passo = 0; passo < 16; passo += passoArpejo)
        {
            const int nota = acorde[static_cast<std::size_t>(passo / passoArpejo) % acorde.size()];
            notas.push_back(
                N(tempo(compasso, passo), semicolcheia * passoArpejo * 0.8f, hz(nota), volume, onda)
                    .ciclo(ciclo)
                    .queda(9.0f)
                    .liberacao(0.01f));
        }
    }

    // Acorde sustentado com ataque lento, como um teclado de fundo.
    void colchao(int compasso, const std::vector<int> &acorde, float volume, int compassos = 1)
    {
        for (int nota : acorde)
        {
            notas.push_back(
                N(tempo(compasso), duracao(compassos) * 0.98f, hz(nota), volume, Onda::TRIANGULO)
                    .ataque(0.25f)
                    .liberacao(0.3f)
                    .vibrato(0.004f, 4.0f));
        }
    }
};

// Acordes em MIDI a partir da fundamental.
std::vector<int> menor(int raiz)
{
    return {raiz, raiz + 3, raiz + 7};
}
std::vector<int> maior(int raiz)
{
    return {raiz, raiz + 4, raiz + 7};
}
std::vector<int> setima(int raiz)
{
    return {raiz, raiz + 4, raiz + 7, raiz + 10};
}

// Tela inicial: la menor tranquilo, com arpejo e melodia em eco.
Repertorio::Partitura musica_menu()
{
    Compositor c(96);
    const int A3 = midi("A3"), F3 = midi("F3"), C4 = midi("C4"), G3 = midi("G3"), E3 = midi("E3");
    const std::vector<std::vector<int>> acordes = {menor(A3), maior(F3), maior(C4), maior(G3),
                                                   menor(A3), maior(F3), maior(G3), maior(E3)};
    const char *melodias[] = {"E5:4 A5:4 C6:4 B5:4", "A5:6 G5:2 F5:4 E5:4", "G5:4 E5:4 C5:4 E5:4",
                              "D5:8 B4:4 D5:4",      "E5:4 A5:4 C6:4 E6:4", "D6:6 C6:2 A5:8",
                              "B5:4 D6:4 G5:4 B5:4", "G#5:8 E5:8"};
    for (int volta = 0; volta < 2; ++volta)
    {
        for (int i = 0; i < 8; ++i)
        {
            const int compasso = volta * 8 + i;
            const auto &acorde = acordes[static_cast<std::size_t>(i)];
            c.colchao(compasso, acorde, 0.035f);
            std::vector<int> subida = {acorde[0] + 12, acorde[1] + 12, acorde[2] + 12,
                                       acorde[0] + 24, acorde[2] + 12, acorde[1] + 12};
            c.arpejo_acorde(compasso, subida, Onda::QUADRADA, 0.035f, 0.25f);
            c.baixo(compasso, acorde[0] - 12, "r.......5...r...", Onda::TRIANGULO, 0.22f, 1.0f);
            if (volta == 1)
            {
                c.bateria(compasso, "K.....h.S.....h.", 0.6f);
                c.melodia(compasso, melodias[i], Onda::QUADRADA, 0.06f, 0.5f, 0, 0.4f, 0.5f);
            }
            else if (i >= 4)
            {
                c.bateria(compasso, "K.......h.......", 0.45f);
            }
        }
    }
    return {c.notas, c.duracao(16)};
}

// Fases: mi menor energico, com uma segunda parte mais intensa.
Repertorio::Partitura musica_fase()
{
    Compositor c(140);
    const int E3 = midi("E3"), C3 = midi("C3"), D3 = midi("D3"), B2 = midi("B2"), A2 = midi("A2");
    const std::vector<std::vector<int>> acordes = {menor(E3 + 12), maior(C3 + 12), maior(D3 + 12),
                                                   menor(B2 + 12), menor(E3 + 12), maior(C3 + 12),
                                                   menor(A2 + 12), setima(B2 + 12)};
    const int raizes[] = {E3 - 12, C3 - 12, D3 - 12, B2 - 12, E3 - 12, C3 - 12, A2 - 12, B2 - 12};
    const char *parteA[] = {"E5:2 -:2 E5:2 G5:2 B5:4 A5:2 G5:2",
                            "E5:6 D5:2 C5:4 E5:4",
                            "F#5:2 -:2 F#5:2 A5:2 D6:4 C6:2 A5:2",
                            "B5:8 F#5:8",
                            "E5:2 -:2 E5:2 G5:2 B5:4 E6:4",
                            "D6:4 C6:2 B5:2 G5:4 E5:4",
                            "A5:4 C6:4 E6:4 D6:2 C6:2",
                            "B5:8 D#6:4 F#6:4"};
    const char *parteB[] = {"B4:2 E5:2 G5:2 E5:2 B4:2 E5:2 G5:2 B5:2",
                            "A5:4 G5:4 E5:4 C5:4",
                            "A4:2 D5:2 F#5:2 D5:2 A4:2 D5:2 F#5:2 A5:2",
                            "B5:8 A5:4 F#5:4",
                            "G5:4 F#5:4 E5:4 D5:4",
                            "E5:6 C5:2 G4:8",
                            "A4:2 C5:2 E5:2 A5:2 G5:4 F#5:4",
                            "F#5:8 D#5:4 B4:4"};
    for (int compasso = 0; compasso < 24; ++compasso)
    {
        const int i = compasso % 8;
        const auto &acorde = acordes[static_cast<std::size_t>(i)];
        // Introducao de 8 compassos, so ritmo, baixo e arpejo; depois A e B.
        const bool intro = compasso < 8;
        c.baixo(compasso, raizes[i], "rRrRrRrRrRrRrRr5", Onda::QUADRADA, 0.13f, 0.3f);
        c.arpejo_acorde(compasso, acorde, Onda::QUADRADA, 0.03f, 0.125f);
        if (intro)
        {
            c.bateria(compasso, i % 2 == 0 ? "K.h.S.h.K.h.S.h." : "K.h.S.h.K.KhS.ho", 0.85f);
            continue;
        }
        const bool b = compasso >= 16;
        c.bateria(compasso, i == 7 ? "K.hKS.hKK.SSS.SS" : "K.h.S.hhK.KhS.ho");
        c.melodia(compasso, b ? parteB[i] : parteA[i], Onda::QUADRADA, 0.075f, 0.25f, 0, 0.3f,
                  0.6f);
        if (b)
        {
            c.melodia(compasso, parteB[i], Onda::TRIANGULO, 0.08f, 0.5f, -12);
        }
    }
    return {c.notas, c.duracao(24)};
}

// Chefe: frigio sombrio em re, rapido, baixo serrilhado e alarmes.
Repertorio::Partitura musica_chefe()
{
    Compositor c(162);
    const int D3 = midi("D3"), Eb3 = midi("Eb3"), C3 = midi("C3"), Bb2 = midi("Bb2"),
              A2 = midi("A2");
    const std::vector<std::vector<int>> acordes = {
        menor(D3 + 12), maior(Eb3 + 12), menor(D3 + 12),  maior(C3 + 12),
        menor(D3 + 12), maior(Eb3 + 12), maior(Bb2 + 12), maior(A2 + 12)};
    const int raizes[] = {D3 - 12, Eb3 - 12, D3 - 12,  C3 - 12,
                          D3 - 12, Eb3 - 12, Bb2 - 12, A2 - 12};
    const char *melodias[] = {"D5:4 F5:4 A5:4 G#5:4",         "G5:8 Eb5:4 F5:4",
                              "D5:4 F5:4 A5:4 D6:4",          "C6:6 Bb5:2 A5:4 G5:4",
                              "D6:2 -:2 D6:2 C6:2 A5:4 F5:4", "G5:4 Bb5:4 Eb6:8",
                              "D6:4 C6:4 Bb5:4 F5:4",         "A5:4 C#6:4 E6:8"};
    for (int compasso = 0; compasso < 16; ++compasso)
    {
        const int i = compasso % 8;
        const bool segunda = compasso >= 8;
        c.baixo(compasso, raizes[i], "rrRrrrRrrrRrrRr5", Onda::DENTE_SERRA, 0.12f, 0.22f);
        c.arpejo_acorde(compasso, {acordes[i][0], acordes[i][1], acordes[i][2], acordes[i][0] + 6},
                        Onda::QUADRADA, 0.028f, 0.125f);
        c.bateria(compasso, i == 7 ? "KKSKKKSKSSSSSSSS"
                                   : (segunda ? "K.hKS.hKK.hKS.hS" : "K.h.S.h.K.hKS.h."));
        c.melodia(compasso, melodias[i], Onda::DENTE_SERRA, 0.06f, 0.5f, 0, 0.25f, 0.35f);
        if (segunda)
        {
            c.melodia(compasso, melodias[i], Onda::QUADRADA, 0.045f, 0.25f, 12, 0.0f, 0.5f);
        }
        // Sirene no primeiro tempo de cada dois compassos.
        if (i % 2 == 0)
        {
            c.notas.push_back(N(c.tempo(compasso), 0.3f, hz(midi("D6")), 0.03f, Onda::QUADRADA)
                                  .ate(hz(midi("A5")))
                                  .ciclo(0.5f)
                                  .filtro(0.4f));
        }
    }
    return {c.notas, c.duracao(16)};
}
} // namespace

// ---------------------------------------------------------------- efeitos

std::vector<Nota> Repertorio::compor_efeito(EfeitoSonoro efeito)
{
    const Onda Q = Onda::QUADRADA, T = Onda::TRIANGULO, S = Onda::DENTE_SERRA, R = Onda::RUIDO,
               SEN = Onda::SENOIDE;
    std::vector<Nota> n;
    switch (efeito)
    {
    case EfeitoSonoro::MENU_MOVER:
        n = {N(0, 0.045f, 880, 0.12f).ciclo(0.25f).queda(35)};
        break;
    case EfeitoSonoro::MENU_CONFIRMAR:
        n = {N(0, 0.06f, 660, 0.14f).ciclo(0.25f),
             N(0.055f, 0.12f, 990, 0.14f).ciclo(0.25f).queda(14),
             N(0.055f, 0.12f, 1320, 0.05f, T).queda(14)};
        break;
    case EfeitoSonoro::MENU_VOLTAR:
        n = {N(0, 0.06f, 660, 0.12f).ciclo(0.25f),
             N(0.055f, 0.1f, 440, 0.12f).ciclo(0.25f).queda(18)};
        break;
    case EfeitoSonoro::MENU_AJUSTE:
        n = {N(0, 0.035f, 1200, 0.14f, T).queda(40)};
        break;
    case EfeitoSonoro::CARREGAMENTO_LINHA:
        n = {N(0, 0.02f, 1760, 0.05f).ciclo(0.125f)};
        break;
    case EfeitoSonoro::CARREGAMENTO_PRONTO:
        arpejo(n, 0, 0.07f, {midi("C5"), midi("E5"), midi("G5"), midi("C6")}, 0.9f, 0.08f, T);
        arpejo(n, 0, 0.07f, {midi("C4"), midi("G4"), midi("E5"), midi("G5")}, 0.9f, 0.04f, Q,
               0.25f);
        n.push_back(N(0.28f, 1.0f, hz(midi("C7")), 0.025f, SEN).vibrato(0.01f, 7).queda(3));
        break;
    case EfeitoSonoro::TIRO_SOCO:
        n = {N(0, 0.09f, 5000, 0.22f, R).ate(900).filtro(0.35f).queda(25),
             N(0, 0.07f, 220, 0.25f, SEN).ate(70).queda(20)};
        break;
    case EfeitoSonoro::TIRO_PISTOLA:
        n = {N(0, 0.09f, 1100, 0.12f).ate(280).ciclo(0.25f).queda(18),
             N(0, 0.03f, 8000, 0.08f, R).queda(60)};
        break;
    case EfeitoSonoro::TIRO_SHOTGUN:
        n = {N(0, 0.28f, 6000, 0.32f, R).ate(400).filtro(0.45f).queda(11),
             N(0, 0.16f, 140, 0.32f, SEN).ate(45).queda(14),
             N(0.12f, 0.05f, 2200, 0.05f).ciclo(0.5f), N(0.18f, 0.05f, 1800, 0.05f).ciclo(0.5f)};
        break;
    case EfeitoSonoro::TIRO_LASER:
        n = {N(0, 0.28f, 2400, 0.09f, S).ate(180).vibrato(0.04f, 40).filtro(0.6f),
             N(0, 0.22f, 3200, 0.05f).ate(500).ciclo(0.125f).queda(6)};
        break;
    case EfeitoSonoro::TIRO_DUPLA:
        n = {N(0, 0.07f, 1300, 0.1f).ate(420).ciclo(0.25f).queda(15),
             N(0.035f, 0.07f, 900, 0.1f).ate(300).ciclo(0.25f).queda(15)};
        break;
    case EfeitoSonoro::TIRO_RICOCHETE:
        n = {N(0, 0.3f, 240, 0.22f, S).ate(55).filtro(0.2f).queda(8),
             N(0, 0.12f, 3000, 0.18f, R).ate(400).filtro(0.3f).queda(20),
             N(0.02f, 0.18f, 1600, 0.04f, SEN).ate(2400).vibrato(0.03f, 60)};
        break;
    case EfeitoSonoro::TIRO_ESPADA:
        n = {N(0, 0.17f, 12000, 0.2f, R).ate(1800).filtro(0.35f).ataque(0.03f),
             N(0.02f, 0.16f, 1700, 0.05f, SEN).ate(2300).queda(10)};
        break;
    case EfeitoSonoro::TIRO_ARCO:
        n = {N(0, 0.09f, 320, 0.16f, T).ate(980).queda(10),
             N(0, 0.14f, 180, 0.1f).ciclo(0.125f).queda(20).filtro(0.4f),
             N(0.03f, 0.1f, 9000, 0.06f, R).ate(3000).queda(25)};
        break;
    case EfeitoSonoro::TIRO_BOOMERANG:
        n = {N(0, 0.26f, 650, 0.13f, T).vibrato(0.18f, 16).ataque(0.03f),
             N(0, 0.2f, 4000, 0.05f, R).filtro(0.3f).vibrato(0.3f, 16).ataque(0.05f)};
        break;
    case EfeitoSonoro::TIRO_PULSO:
        n = {N(0, 0.13f, 260, 0.13f, SEN).ate(1400).queda(8),
             N(0, 0.1f, 130, 0.09f).ate(260).ciclo(0.5f).queda(14).filtro(0.5f)};
        break;
    case EfeitoSonoro::SEM_MUNICAO:
        n = {N(0, 0.016f, 2200, 0.16f).ciclo(0.5f), N(0.05f, 0.016f, 1600, 0.16f).ciclo(0.5f)};
        break;
    case EfeitoSonoro::ACERTO_INIMIGO:
        n = {N(0, 0.05f, 340, 0.1f).ate(120).ciclo(0.5f).queda(25),
             N(0, 0.03f, 4000, 0.06f, R).queda(60)};
        break;
    case EfeitoSonoro::OBSTACULO_QUEBRA:
        n = {N(0, 0.3f, 2400, 0.3f, R).ate(260).filtro(0.35f).queda(10),
             N(0, 0.14f, 150, 0.22f, T).ate(55).queda(14), N(0.06f, 0.04f, 900, 0.06f, R).queda(40),
             N(0.11f, 0.04f, 700, 0.05f, R).queda(40)};
        break;
    case EfeitoSonoro::MORTE_VIRUS:
        n = {N(0, 0.26f, 900, 0.12f).ate(110).picote(45).ciclo(0.25f),
             N(0, 0.12f, 6000, 0.1f, R).ate(800).queda(18).filtro(0.5f)};
        break;
    case EfeitoSonoro::MORTE_BRUTO:
        n = {N(0, 0.5f, 320, 0.2f, S).ate(38).filtro(0.25f).queda(4),
             N(0, 0.45f, 2400, 0.32f, R).ate(160).filtro(0.3f).queda(6),
             N(0, 0.2f, 130, 0.4f, SEN).ate(35).queda(10),
             N(0.1f, 0.3f, 1200, 0.07f).ate(90).picote(30).ciclo(0.25f)};
        break;
    case EfeitoSonoro::MORTE_CHEFE:
        n = {N(0, 2.4f, 3000, 0.45f, R).ate(60).filtro(0.25f).queda(1.4f),
             N(0, 1.4f, 120, 0.5f, SEN).ate(28).queda(2.0f),
             N(0, 1.8f, 1400, 0.12f).ate(55).picote(22).ciclo(0.25f).queda(1.2f)};
        for (int i = 0; i < 5; ++i)
        {
            const float t = 0.25f + i * 0.28f;
            n.push_back(
                N(t, 0.35f, 2600 - i * 300.0f, 0.3f - i * 0.04f, R).ate(150).filtro(0.3f).queda(8));
            n.push_back(N(t, 0.15f, 160, 0.3f - i * 0.04f, SEN).ate(40).queda(12));
        }
        // Acorde de "sistema desligado" depois da explosao.
        arpejo(n, 1.6f, 0.12f, {midi("A4"), midi("E4"), midi("C4"), midi("A3")}, 0.9f, 0.08f, T);
        break;
    case EfeitoSonoro::TIRO_INIMIGO:
        n = {N(0, 0.08f, 560, 0.045f).ate(250).ciclo(0.5f).filtro(0.45f).queda(14)};
        break;
    case EfeitoSonoro::TIRO_CHEFE:
        n = {N(0, 0.16f, 320, 0.08f, S).ate(110).filtro(0.35f).queda(9),
             N(0, 0.1f, 1500, 0.04f, R).filtro(0.3f).queda(20)};
        break;
    case EfeitoSonoro::INVOCACAO:
        n = {N(0, 0.6f, 180, 0.12f, T).ate(1300).vibrato(0.06f, 18).ataque(0.05f),
             N(0.1f, 0.5f, 90, 0.08f, S).ate(500).filtro(0.25f).vibrato(0.1f, 9)};
        break;
    case EfeitoSonoro::ONDA_ALERTA:
        for (int i = 0; i < 4; ++i)
        {
            n.push_back(N(i * 0.14f, 0.13f, i % 2 == 0 ? 880 : 620, 0.1f)
                            .ciclo(0.5f)
                            .filtro(0.45f)
                            .liberacao(0.02f));
        }
        break;
    case EfeitoSonoro::DANO_ESCUDO:
        n = {N(0, 0.12f, 1600, 0.12f).ate(500).picote(70).ciclo(0.25f),
             N(0, 0.07f, 2600, 0.06f, T).ate(1800).queda(20)};
        break;
    case EfeitoSonoro::DANO_VIDA:
        n = {N(0, 0.2f, 260, 0.2f).ate(75).ciclo(0.5f).filtro(0.5f).queda(8),
             N(0, 0.14f, 1500, 0.2f, R).ate(250).filtro(0.4f).queda(15)};
        break;
    case EfeitoSonoro::CORACAO:
        arpejo(n, 0, 0.06f, {midi("C6"), midi("E6"), midi("G6"), midi("C7")}, 0.18f, 0.12f, T);
        n.push_back(N(0, 0.3f, hz(midi("C5")), 0.05f).ciclo(0.25f).queda(6));
        break;
    case EfeitoSonoro::PORTAS_TRANCAM:
        n = {N(0, 0.35f, 900, 0.4f, R).ate(70).filtro(0.2f).queda(7),
             N(0, 0.4f, 82, 0.2f, S).filtro(0.25f).queda(5),
             N(0.03f, 0.03f, 3000, 0.1f).ciclo(0.5f),
             N(0.12f, 0.03f, 2400, 0.08f).ciclo(0.5f),
             N(0.1f, 0.55f, hz(midi("D4")), 0.06f).ciclo(0.25f).filtro(0.4f).vibrato(0.02f, 6),
             N(0.1f, 0.55f, hz(midi("G#4")), 0.05f).ciclo(0.25f).filtro(0.4f).vibrato(0.02f, 6)};
        break;
    case EfeitoSonoro::SALA_LIBERADA:
        arpejo(n, 0, 0.075f, {midi("G5"), midi("C6"), midi("E6"), midi("G6")}, 0.14f, 0.1f, Q,
               0.25f);
        for (int nota : {midi("C5"), midi("E5"), midi("G5"), midi("C6")})
        {
            n.push_back(N(0.3f, 0.6f, hz(nota), 0.05f, T).queda(2.5f));
        }
        n.push_back(N(0.3f, 0.6f, hz(midi("E6")), 0.05f).ciclo(0.125f).vibrato(0.01f, 6).queda(3));
        n.push_back(N(0.05f, 0.5f, 400, 0.12f, R).ate(3000).filtro(0.15f).ataque(0.2f));
        break;
    case EfeitoSonoro::BAU_ABRE:
        n = {N(0, 0.3f, 170, 0.1f, S).ate(280).picote(28).filtro(0.3f),
             N(0.26f, 0.06f, 600, 0.18f, R).filtro(0.3f).queda(30)};
        arpejo(n, 0.3f, 0.05f, {midi("E6"), midi("G#6"), midi("B6"), midi("E7"), midi("G#7")},
               0.25f, 0.08f, T);
        n.push_back(N(0.3f, 0.6f, hz(midi("B7")), 0.02f, SEN).vibrato(0.02f, 9).queda(4));
        break;
    case EfeitoSonoro::ARMA_COLETA:
        n = {N(0, 0.05f, 520, 0.12f).ciclo(0.25f),
             N(0.05f, 0.1f, 780, 0.12f).ate(1560).ciclo(0.25f).queda(10),
             N(0, 0.03f, 6000, 0.06f, R).queda(50)};
        break;
    case EfeitoSonoro::ARMA_SOLTA:
        n = {N(0, 0.1f, 700, 0.1f, T).ate(180).queda(15),
             N(0.06f, 0.08f, 1400, 0.16f, R).filtro(0.3f).queda(30)};
        break;
    case EfeitoSonoro::ARMA_TROCA:
        n = {N(0, 0.025f, 5000, 0.12f, R).queda(60), N(0, 0.03f, 1300, 0.08f).ciclo(0.25f),
             N(0.06f, 0.025f, 4000, 0.12f, R).queda(60), N(0.06f, 0.04f, 950, 0.08f).ciclo(0.25f)};
        break;
    case EfeitoSonoro::PORTAL_SURGE:
        n = {N(0, 1.6f, 90, 0.2f, SEN).ate(720).vibrato(0.06f, 7).ataque(0.3f).liberacao(0.5f),
             N(0, 1.6f, 300, 0.12f, R).ate(9000).filtro(0.12f).ataque(0.6f).liberacao(0.6f)};
        for (int nota : {midi("D5"), midi("A5"), midi("D6"), midi("F#6")})
        {
            n.push_back(
                N(0.4f, 1.4f, hz(nota), 0.035f, T).ataque(0.5f).liberacao(0.6f).vibrato(0.01f, 5));
        }
        break;
    case EfeitoSonoro::FASE_CONCLUIDA:
        arpejo(n, 0, 0.11f, {midi("C5"), midi("E5"), midi("G5"), midi("C6")}, 0.12f, 0.1f, Q,
               0.25f);
        n.push_back(N(0.44f, 0.2f, hz(midi("A5")), 0.1f).ciclo(0.25f).queda(4));
        n.push_back(N(0.66f, 0.2f, hz(midi("B5")), 0.1f).ciclo(0.25f).queda(4));
        n.push_back(
            N(0.88f, 1.2f, hz(midi("C6")), 0.11f).ciclo(0.25f).vibrato(0.012f, 6).queda(1.2f));
        for (int nota : {midi("C4"), midi("G4"), midi("C5"), midi("E5")})
        {
            n.push_back(N(0.88f, 1.2f, hz(nota), 0.06f, T).queda(1.2f));
        }
        n.push_back(N(0.88f, 0.16f, 150, 0.4f, SEN).ate(45).queda(14));
        break;
    case EfeitoSonoro::MORTE_JOGADOR:
        n = {N(0, 1.1f, 700, 0.14f).ate(55).picote(14).ciclo(0.5f).filtro(0.6f),
             N(0, 0.4f, 2500, 0.25f, R).ate(120).filtro(0.3f).queda(5),
             N(0, 0.25f, 180, 0.35f, SEN).ate(40).queda(9)};
        break;
    case EfeitoSonoro::GAME_OVER:
    {
        const char *notas[] = {"E5", "D#5", "D5", "C#5"};
        for (int i = 0; i < 4; ++i)
        {
            n.push_back(
                N(i * 0.32f, 0.3f, hz(midi(notas[i])), 0.1f).ciclo(0.5f).filtro(0.5f).queda(2));
        }
        n.push_back(N(1.3f, 1.6f, hz(midi("C5")), 0.1f)
                        .ciclo(0.5f)
                        .vibrato(0.02f, 5)
                        .filtro(0.5f)
                        .liberacao(0.8f));
        for (int nota : {midi("A2"), midi("A3"), midi("C4"), midi("E4")})
        {
            n.push_back(N(1.3f, 1.8f, hz(nota), 0.06f, T).liberacao(0.9f));
        }
        break;
    }
    case EfeitoSonoro::CONTINUE:
        arpejo(n, 0, 0.06f,
               {midi("A4"), midi("C5"), midi("E5"), midi("A5"), midi("C6"), midi("E6")}, 0.2f, 0.1f,
               Q, 0.25f);
        break;
    case EfeitoSonoro::VITORIA:
    {
        const char *tema[] = {"G4", "C5", "E5", "G5", "E5", "G5"};
        const float tempos[] = {0, 0.14f, 0.28f, 0.42f, 0.7f, 0.84f};
        for (int i = 0; i < 6; ++i)
        {
            n.push_back(N(tempos[i], 0.13f, hz(midi(tema[i])), 0.11f).ciclo(0.25f).queda(3));
        }
        n.push_back(
            N(1.0f, 1.8f, hz(midi("C6")), 0.12f).ciclo(0.25f).vibrato(0.015f, 6).queda(0.8f));
        for (int nota : {midi("C4"), midi("E4"), midi("G4"), midi("C5"), midi("E5")})
        {
            n.push_back(N(1.0f, 2.0f, hz(nota), 0.05f, T).queda(0.8f));
        }
        for (int i = 0; i < 6; ++i)
        {
            n.push_back(
                N(1.0f + i * 0.15f, 0.25f, hz(midi("C7") + (i % 3) * 4), 0.03f, SEN).queda(6));
        }
        n.push_back(N(1.0f, 0.2f, 150, 0.45f, SEN).ate(40).queda(12));
        n.push_back(N(1.0f, 0.6f, 9000, 0.12f, R).queda(5));
        break;
    }
    case EfeitoSonoro::CHEAT:
        arpejo(n, 0, 0.045f,
               {midi("C5"), midi("G5"), midi("C6"), midi("D5"), midi("A5"), midi("D6"), midi("E5"),
                midi("B5"), midi("E6")},
               0.08f, 0.09f, Q, 0.25f);
        break;
    case EfeitoSonoro::TOTAL:
        break;
    }
    return n;
}

Repertorio::Partitura Repertorio::compor_musica(Musica musica)
{
    switch (musica)
    {
    case Musica::MENU:
        return musica_menu();
    case Musica::FASE:
        return musica_fase();
    case Musica::CHEFE:
        return musica_chefe();
    default:
        return {};
    }
}

float Repertorio::intervalo_minimo(EfeitoSonoro efeito)
{
    switch (efeito)
    {
    case EfeitoSonoro::ACERTO_INIMIGO:
    case EfeitoSonoro::TIRO_INIMIGO:
        return 0.05f;
    case EfeitoSonoro::OBSTACULO_QUEBRA:
    case EfeitoSonoro::MORTE_VIRUS:
    case EfeitoSonoro::DANO_ESCUDO:
    case EfeitoSonoro::SEM_MUNICAO:
        return 0.06f;
    case EfeitoSonoro::TIRO_CHEFE:
    case EfeitoSonoro::ONDA_ALERTA:
        return 0.1f;
    default:
        return 0.02f;
    }
}
