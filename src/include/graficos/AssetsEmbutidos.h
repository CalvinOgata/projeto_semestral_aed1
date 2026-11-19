#ifndef ASSETS_EMBUTIDOS_H
#define ASSETS_EMBUTIDOS_H

#include <cstddef>

// Bytes de um arquivo de assets/ copiados para dentro do executavel durante a
// compilacao, para que o jogo nao precise da pasta assets/ ao lado dele.
struct AssetEmbutido
{
    const unsigned char *dados;
    std::size_t tamanho;
};

// Definidos no .cpp gerado por ferramentas/embutir_assets.py. Ao adicionar um
// asset, declare-o aqui e inclua NOME=caminho em ASSETS_EMBUTIDOS no Makefile.
namespace AssetsEmbutidos
{
extern const AssetEmbutido SPRITE_JOGADOR;
extern const AssetEmbutido SPRITE_GAME_OVER;
extern const AssetEmbutido INIMIGO_0;
extern const AssetEmbutido INIMIGO_1;
extern const AssetEmbutido INIMIGO_2;
extern const AssetEmbutido INIMIGO_3;
extern const AssetEmbutido INIMIGO_4;
extern const AssetEmbutido INIMIGO_5;
extern const AssetEmbutido INIMIGO_6;
extern const AssetEmbutido INIMIGO_7;
extern const AssetEmbutido INIMIGO_8;
extern const AssetEmbutido ARMA_0;
extern const AssetEmbutido ARMA_1;
extern const AssetEmbutido ARMA_2;
extern const AssetEmbutido ARMA_3;
extern const AssetEmbutido ARMA_4;
extern const AssetEmbutido ARMA_5;
extern const AssetEmbutido ARMA_6;
extern const AssetEmbutido ARMA_7;
extern const AssetEmbutido ARMA_8;
extern const AssetEmbutido PISO_0;
extern const AssetEmbutido PISO_1;
extern const AssetEmbutido PISO_2;
extern const AssetEmbutido PISO_3;
extern const AssetEmbutido PISO_4;
extern const AssetEmbutido PISO_5;
extern const AssetEmbutido PISO_6;
extern const AssetEmbutido PISO_7;
extern const AssetEmbutido PISO_8;
extern const AssetEmbutido PISO_9;
extern const AssetEmbutido PISO_10;
extern const AssetEmbutido PISO_11;
extern const AssetEmbutido PISO_12;
extern const AssetEmbutido PISO_13;
extern const AssetEmbutido PISO_14;
extern const AssetEmbutido PISO_15;
extern const AssetEmbutido PISO_16;
extern const AssetEmbutido PISO_17;
extern const AssetEmbutido PISO_18;
extern const AssetEmbutido PISO_19;
extern const AssetEmbutido PISO_20;
extern const AssetEmbutido PISO_21;
} // namespace AssetsEmbutidos

#endif
