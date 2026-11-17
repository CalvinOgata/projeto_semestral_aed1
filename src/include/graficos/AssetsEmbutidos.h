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
} // namespace AssetsEmbutidos

#endif
