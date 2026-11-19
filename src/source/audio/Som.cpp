#include "audio/Som.h"

#include <SDL2/SDL.h>

#include <algorithm>
#include <array>
#include <cmath>
#include <vector>

#include "nucleo/Plataforma.h"

namespace
{
constexpr int QUANTIDADE_EFEITOS = static_cast<int>(EfeitoSonoro::TOTAL);
constexpr int QUANTIDADE_MUSICAS = static_cast<int>(Musica::TOTAL);
constexpr int MAX_VOZES = 32;
constexpr int MAX_REPETICOES = 4; // copias simultaneas do mesmo efeito
constexpr float SEGUNDOS_TRANSICAO = 1.2f;
constexpr float GANHO_ABAFADO = 0.35f;
// Os sons sao compostos com folga; este ganho leva a mistura a um nivel confortavel.
constexpr float GANHO_MESTRE = 1.7f;
// Peso de cada trilha na barra de carregamento, contra 1 por efeito.
constexpr float PESO_MUSICA = 12.0f;

struct Voz
{
    int efeito = -1;
    std::size_t posicao = 0;
    float volume = 0.0f;
};

struct Faixa
{
    int musica = 0; // 0 = Musica::NENHUMA
    std::size_t posicao = 0;
    float ganho = 0.0f;
    float alvo = 0.0f;
};

struct Estado
{
    SDL_AudioDeviceID dispositivo = 0;
    std::array<std::vector<float>, QUANTIDADE_EFEITOS> efeitos;
    std::array<bool, QUANTIDADE_EFEITOS> efeitoPronto = {};
    std::array<std::vector<float>, QUANTIDADE_MUSICAS> musicas;
    std::array<bool, QUANTIDADE_MUSICAS> musicaPronta = {};
    std::array<Uint32, QUANTIDADE_EFEITOS> ultimoDisparo = {};

    std::array<Voz, MAX_VOZES> vozes;
    std::array<Faixa, 2> faixas; // tocando e saindo, para a transicao cruzada
    int musicaDesejada = 0;
    float volumeMusica = 0.7f;
    float volumeEfeitos = 0.8f;
    float abafamento = 1.0f;
    float abafamentoAlvo = 1.0f;

    // Carregamento incremental.
    int proximoEfeito = 0;
    int musicaEmCarga = 1;
    std::vector<Sintetizador::Nota> partitura;
    std::size_t proximaNota = 0;
    bool concluido = false;
} estado;

// Curva perceptiva: metade da barra soa como metade do volume.
float curva(float volume)
{
    return volume * volume;
}

void misturar(void * /*dados*/, Uint8 *fluxo, int bytes)
{
    float *saida = reinterpret_cast<float *>(fluxo);
    const int quantidade = bytes / static_cast<int>(sizeof(float));
    const float passoTransicao = 1.0f / (SEGUNDOS_TRANSICAO * Sintetizador::TAXA_AMOSTRAS);

    // A trilha pedida comeca quando sua sintese termina.
    const int desejada = estado.musicaDesejada;
    if (desejada != estado.faixas[0].musica &&
        (desejada == 0 || estado.musicaPronta[static_cast<std::size_t>(desejada)]))
    {
        estado.faixas[1] = estado.faixas[0];
        estado.faixas[1].alvo = 0.0f;
        estado.faixas[0] = Faixa{desejada, 0, 0.0f, 1.0f};
    }

    const float volumeMusica = curva(estado.volumeMusica);
    const float volumeEfeitos = curva(estado.volumeEfeitos);
    for (int i = 0; i < quantidade; ++i)
    {
        float musica = 0.0f;
        for (Faixa &faixa : estado.faixas)
        {
            if (faixa.musica == 0)
            {
                continue;
            }
            const auto &dados = estado.musicas[static_cast<std::size_t>(faixa.musica)];
            if (dados.empty())
            {
                faixa.musica = 0;
                continue;
            }
            musica += dados[faixa.posicao] * faixa.ganho;
            faixa.posicao = (faixa.posicao + 1) % dados.size();
            if (faixa.ganho < faixa.alvo)
            {
                faixa.ganho = std::min(faixa.alvo, faixa.ganho + passoTransicao);
            }
            else if (faixa.ganho > faixa.alvo)
            {
                faixa.ganho = std::max(faixa.alvo, faixa.ganho - passoTransicao);
                if (faixa.ganho <= 0.0f)
                {
                    faixa.musica = 0;
                }
            }
        }
        estado.abafamento += (estado.abafamentoAlvo - estado.abafamento) * 0.0002f;

        float efeitos = 0.0f;
        for (Voz &voz : estado.vozes)
        {
            if (voz.efeito < 0)
            {
                continue;
            }
            const auto &dados = estado.efeitos[static_cast<std::size_t>(voz.efeito)];
            if (voz.posicao >= dados.size())
            {
                voz.efeito = -1;
                continue;
            }
            efeitos += dados[voz.posicao++] * voz.volume;
        }

        const float mistura = musica * volumeMusica * estado.abafamento + efeitos * volumeEfeitos;
        // Saturacao suave: muitos sons juntos encostam no limite sem estalar.
        saida[i] = std::tanh(mistura * GANHO_MESTRE);
    }
}

float progresso()
{
    const float total = QUANTIDADE_EFEITOS + PESO_MUSICA * (QUANTIDADE_MUSICAS - 1);
    float feito = static_cast<float>(estado.proximoEfeito);
    feito += PESO_MUSICA * static_cast<float>(estado.musicaEmCarga - 1);
    if (!estado.partitura.empty())
    {
        feito += PESO_MUSICA * static_cast<float>(estado.proximaNota) /
                 static_cast<float>(estado.partitura.size());
    }
    return estado.concluido ? 1.0f : std::min(1.0f, feito / total);
}

void publicar_musica(int indice, std::vector<float> &&dados)
{
    SDL_LockAudioDevice(estado.dispositivo);
    estado.musicas[static_cast<std::size_t>(indice)] = std::move(dados);
    estado.musicaPronta[static_cast<std::size_t>(indice)] = true;
    SDL_UnlockAudioDevice(estado.dispositivo);
}

// Cada trilha e sintetizada num buffer proprio e so entra no mixer pronta.
std::vector<float> trilhaEmCarga;
} // namespace

bool Som::inicializar()
{
    if (SDL_InitSubSystem(SDL_INIT_AUDIO) != 0)
    {
        SDL_Log("Sem audio: %s", SDL_GetError());
        return false;
    }
    SDL_AudioSpec desejado;
    SDL_zero(desejado);
    desejado.freq = Sintetizador::TAXA_AMOSTRAS;
    desejado.format = AUDIO_F32SYS;
    desejado.channels = 1;
    desejado.samples = Plataforma::AMOSTRAS_AUDIO;
    desejado.callback = misturar;
    // Sem permitir mudancas: o SDL converte para o formato real do dispositivo.
    estado.dispositivo = SDL_OpenAudioDevice(nullptr, 0, &desejado, nullptr, 0);
    if (estado.dispositivo == 0)
    {
        SDL_Log("Sem dispositivo de audio: %s", SDL_GetError());
        return false;
    }
    SDL_PauseAudioDevice(estado.dispositivo, 0);
    return true;
}

void Som::finalizar()
{
    if (estado.dispositivo != 0)
    {
        SDL_CloseAudioDevice(estado.dispositivo);
        estado.dispositivo = 0;
    }
    SDL_QuitSubSystem(SDL_INIT_AUDIO);
}

float Som::carregar_etapa(int orcamentoMs)
{
    if (estado.dispositivo == 0)
    {
        estado.concluido = true; // sem audio nao ha o que sintetizar
    }
    const Uint32 inicio = SDL_GetTicks();
    while (!estado.concluido && SDL_GetTicks() - inicio < static_cast<Uint32>(orcamentoMs))
    {
        if (estado.proximoEfeito < QUANTIDADE_EFEITOS)
        {
            const auto notas =
                Repertorio::compor_efeito(static_cast<EfeitoSonoro>(estado.proximoEfeito));
            std::vector<float> dados(static_cast<std::size_t>(
                (Sintetizador::duracao_total(notas) + 0.01f) * Sintetizador::TAXA_AMOSTRAS));
            for (const auto &nota : notas)
            {
                Sintetizador::renderizar(dados, nota);
            }
            SDL_LockAudioDevice(estado.dispositivo);
            estado.efeitos[static_cast<std::size_t>(estado.proximoEfeito)] = std::move(dados);
            estado.efeitoPronto[static_cast<std::size_t>(estado.proximoEfeito)] = true;
            SDL_UnlockAudioDevice(estado.dispositivo);
            ++estado.proximoEfeito;
            continue;
        }
        if (estado.musicaEmCarga >= QUANTIDADE_MUSICAS)
        {
            estado.concluido = true;
            break;
        }
        if (estado.partitura.empty())
        {
            auto partitura = Repertorio::compor_musica(static_cast<Musica>(estado.musicaEmCarga));
            estado.partitura = std::move(partitura.notas);
            estado.proximaNota = 0;
            trilhaEmCarga.assign(
                static_cast<std::size_t>(partitura.duracao * Sintetizador::TAXA_AMOSTRAS), 0.0f);
        }
        // Algumas notas por vez, para a barra de progresso andar suave.
        const std::size_t fim = std::min(estado.partitura.size(), estado.proximaNota + 24);
        for (; estado.proximaNota < fim; ++estado.proximaNota)
        {
            Sintetizador::renderizar(trilhaEmCarga, estado.partitura[estado.proximaNota], true);
        }
        if (estado.proximaNota >= estado.partitura.size())
        {
            publicar_musica(estado.musicaEmCarga, std::move(trilhaEmCarga));
            trilhaEmCarga = {};
            estado.partitura.clear();
            ++estado.musicaEmCarga;
        }
    }
    return progresso();
}

bool Som::carregado()
{
    return estado.concluido;
}

void Som::tocar(EfeitoSonoro efeito, float volume)
{
    const int indice = static_cast<int>(efeito);
    if (estado.dispositivo == 0 || indice < 0 || indice >= QUANTIDADE_EFEITOS ||
        !estado.efeitoPronto[static_cast<std::size_t>(indice)])
    {
        return;
    }
    const Uint32 agora = SDL_GetTicks();
    const auto minimo = static_cast<Uint32>(Repertorio::intervalo_minimo(efeito) * 1000.0f);
    Uint32 &ultimo = estado.ultimoDisparo[static_cast<std::size_t>(indice)];
    if (ultimo != 0 && agora - ultimo < minimo)
    {
        return;
    }
    ultimo = agora;

    SDL_LockAudioDevice(estado.dispositivo);
    int repeticoes = 0;
    Voz *livre = nullptr;
    Voz *maisAntiga = &estado.vozes[0];
    for (Voz &voz : estado.vozes)
    {
        if (voz.efeito < 0)
        {
            livre = livre ? livre : &voz;
            continue;
        }
        repeticoes += voz.efeito == indice;
        if (voz.posicao > maisAntiga->posicao)
        {
            maisAntiga = &voz;
        }
    }
    if (repeticoes < MAX_REPETICOES)
    {
        // Sem voz livre, o som mais adiantado cede lugar ao novo.
        Voz &destino = livre ? *livre : *maisAntiga;
        destino = Voz{indice, 0, volume};
    }
    SDL_UnlockAudioDevice(estado.dispositivo);
}

void Som::tocar_musica(Musica musica)
{
    if (estado.dispositivo == 0)
    {
        return;
    }
    SDL_LockAudioDevice(estado.dispositivo);
    estado.musicaDesejada = static_cast<int>(musica);
    SDL_UnlockAudioDevice(estado.dispositivo);
}

void Som::set_volumes(float musica, float efeitos)
{
    if (estado.dispositivo == 0)
    {
        return;
    }
    SDL_LockAudioDevice(estado.dispositivo);
    estado.volumeMusica = std::clamp(musica, 0.0f, 1.0f);
    estado.volumeEfeitos = std::clamp(efeitos, 0.0f, 1.0f);
    SDL_UnlockAudioDevice(estado.dispositivo);
}

void Som::set_abafado(bool abafado)
{
    if (estado.dispositivo == 0)
    {
        return;
    }
    SDL_LockAudioDevice(estado.dispositivo);
    estado.abafamentoAlvo = abafado ? GANHO_ABAFADO : 1.0f;
    SDL_UnlockAudioDevice(estado.dispositivo);
}
