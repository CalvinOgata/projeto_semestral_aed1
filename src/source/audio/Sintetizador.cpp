#include "audio/Sintetizador.h"

#include <algorithm>
#include <cmath>
#include <cstdint>

namespace
{
constexpr float DOIS_PI = 6.28318530718f;

// Gerador pseudoaleatorio deterministico: o mesmo som sai igual em toda execucao.
struct Ruido
{
    std::uint32_t estado;
    float proximo()
    {
        estado = estado * 1664525u + 1013904223u;
        return static_cast<float>(estado >> 8) / 8388608.0f - 1.0f;
    }
};

float amostra_onda(Sintetizador::Onda onda, float fase, float ciclo, Ruido &ruido)
{
    switch (onda)
    {
    case Sintetizador::Onda::QUADRADA:
        return fase < ciclo ? 1.0f : -1.0f;
    case Sintetizador::Onda::TRIANGULO:
        return fase < 0.5f ? 4.0f * fase - 1.0f : 3.0f - 4.0f * fase;
    case Sintetizador::Onda::DENTE_SERRA:
        return 2.0f * fase - 1.0f;
    case Sintetizador::Onda::SENOIDE:
        return std::sin(DOIS_PI * fase);
    case Sintetizador::Onda::RUIDO:
        return ruido.proximo();
    }
    return 0.0f;
}
} // namespace

float Sintetizador::frequencia_midi(int nota)
{
    return 440.0f * std::pow(2.0f, static_cast<float>(nota - 69) / 12.0f);
}

float Sintetizador::duracao_total(const std::vector<Nota> &notas)
{
    float fim = 0.0f;
    for (const Nota &nota : notas)
    {
        fim = std::max(fim, nota.inicio + nota.duracao);
    }
    return fim;
}

void Sintetizador::renderizar(std::vector<float> &buffer, const Nota &nota, bool emLaco)
{
    const int total = static_cast<int>(nota.duracao * TAXA_AMOSTRAS);
    const int inicio = static_cast<int>(nota.inicio * TAXA_AMOSTRAS);
    const int tamanho = static_cast<int>(buffer.size());
    if (total <= 0 || tamanho == 0)
    {
        return;
    }
    const float freqFinal = nota.freqFinal > 0.0f ? nota.freqFinal : nota.freq;
    const float razao = std::log(freqFinal / nota.freq);
    Ruido ruido{static_cast<std::uint32_t>(nota.freq * 977.0f) +
                static_cast<std::uint32_t>(inicio)};
    Ruido saltos{ruido.estado ^ 0x9E3779B9u};

    float fase = 0.0f;
    float filtrada = 0.0f;
    float multiplicadorSalto = 1.0f;
    float proximoSalto = 0.0f;
    // O ruido avanca na "frequencia" da nota: grave soa como estrondo, agudo como chiado.
    float faseRuido = 0.0f;
    float valorRuido = 0.0f;
    for (int i = 0; i < total; ++i)
    {
        const float t = static_cast<float>(i) / TAXA_AMOSTRAS;
        const float progresso = static_cast<float>(i) / total;
        float freq = nota.freq * std::exp(razao * progresso);
        if (nota.vibrato > 0.0f)
        {
            freq *= 1.0f + nota.vibrato * std::sin(DOIS_PI * nota.vibratoHz * t);
        }
        if (nota.picotar > 0.0f)
        {
            if (t >= proximoSalto)
            {
                multiplicadorSalto = 0.6f + 0.9f * (saltos.proximo() * 0.5f + 0.5f);
                proximoSalto = t + 1.0f / nota.picotar;
            }
            freq *= multiplicadorSalto;
        }

        float valor;
        if (nota.onda == Onda::RUIDO)
        {
            faseRuido += freq / TAXA_AMOSTRAS;
            if (faseRuido >= 1.0f)
            {
                faseRuido -= std::floor(faseRuido);
                valorRuido = ruido.proximo();
            }
            valor = valorRuido;
        }
        else
        {
            fase += freq / TAXA_AMOSTRAS;
            fase -= std::floor(fase);
            valor = amostra_onda(nota.onda, fase, nota.ciclo, ruido);
        }
        filtrada += nota.filtro * (valor - filtrada);

        float envelope = 1.0f;
        if (t < nota.ataque)
        {
            envelope = t / nota.ataque;
        }
        const float restante = nota.duracao - t;
        if (restante < nota.liberacao)
        {
            envelope *= std::max(0.0f, restante / nota.liberacao);
        }
        if (nota.decaimento > 0.0f)
        {
            envelope *= std::exp(-nota.decaimento * t);
        }

        int destino = inicio + i;
        if (destino >= tamanho)
        {
            if (!emLaco)
            {
                break;
            }
            destino %= tamanho;
        }
        buffer[static_cast<std::size_t>(destino)] += filtrada * envelope * nota.volume;
    }
}
