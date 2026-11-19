#ifndef SINTETIZADOR_H
#define SINTETIZADOR_H

#include <vector>

// Sintese sonora no estilo dos consoles antigos, sem SDL e sem arquivos de audio:
// todo efeito e toda musica sao listas de notas transformadas em amostras.
namespace Sintetizador
{
constexpr int TAXA_AMOSTRAS = 44100; // amostras por segundo (mono)

enum class Onda
{
    QUADRADA,
    TRIANGULO,
    DENTE_SERRA,
    SENOIDE,
    RUIDO
};

// Uma nota com envelope simples. Frequencias variam de forma exponencial
// de freq ate freqFinal, o que gera "pew", quedas de explosao e subidas magicas.
struct Nota
{
    float inicio = 0.0f;  // segundos desde o comeco do som
    float duracao = 0.1f; // segundos
    float freq = 440.0f;
    float freqFinal = 0.0f; // 0 mantem a frequencia
    float volume = 0.3f;
    Onda onda = Onda::QUADRADA;
    float ciclo = 0.5f;      // largura do pulso da onda quadrada
    float ataque = 0.004f;   // subida linear do volume
    float liberacao = 0.03f; // descida linear no final
    float decaimento = 0.0f; // queda exponencial por segundo (0 = sustenta)
    float vibrato = 0.0f;    // profundidade relativa da modulacao (0.02 = 2%)
    float vibratoHz = 6.0f;
    float filtro = 1.0f;  // passa-baixas de um polo: 1 desliga, perto de 0 abafa
    float picotar = 0.0f; // saltos aleatorios de altura por segundo (som "glitch")
};

float frequencia_midi(int nota);
float duracao_total(const std::vector<Nota> &notas);

// Soma a nota no buffer. Em laco, o que passa do fim volta ao comeco, para a
// musica repetir sem estalo no ponto de emenda.
void renderizar(std::vector<float> &buffer, const Nota &nota, bool emLaco = false);
} // namespace Sintetizador

#endif
