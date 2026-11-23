# Sprites e hitboxes

Todas as medidas sao em pixels logicos, na resolucao de referencia 960x540. O
redimensionamento da janela (ou da tela do celular) escala o jogo inteiro; um
quadro de 64x64 ocupa 64x64 no mundo, independentemente do monitor.

## Perfis atuais

`src/include/config/DimensoesEntidade.h` centraliza os perfis. A arte foi
ampliada na integracao final, mas as hitboxes anteriores foram mantidas para nao
mudar corredores, colisoes e dificuldade.

| Categoria | Hitbox | Quadro visual | Arquivos |
| --- | --- | --- | --- |
| Heroi (Tux) | 32x32 | 64x64 | `final/00_tux.png` |
| Inimigos basicos | 28x28 | 48x48 | ILOVEYOU.EXE, EULA-BOT, NPU DRONE (`01`, `04`, `07`) |
| Inimigos medios (brutos) | 44x44 | 128x128 | MIRAI SWARM, SENTINELA DRM, GPU H100 (`02`, `05`, `08`) |
| Bosses | 88x88 | 256x256 | WANNACRY PRIME, WINDOWS UPDATE, DATACENTER HIPERESCALA (`03`, `06`, `09`) |
| Armas (chao, inventario e mao) | - | 64x64 no PNG | `final/10_` a `final/18_` |
| Pisos | - | 936x516 | `pisos/faseN_XX.png` e `pisos/bau_compartilhado.png` |

A hitbox e a area solida usada nas colisoes; o quadro e o espaco completo da
figura, incluindo pixels transparentes. O centro do quadro coincide com o centro
da hitbox (o alinhamento **nao** e pelos pes).

## Animacao

Cada personagem e uma faixa PNG com **tres quadros iguais lado a lado**, sem
espacamento. O ciclo e 0,1,0,2, com 0,16 s por pose para Tux e inimigos comuns,
0,22 s para brutos e 0,28 s para bosses. Parado, o Tux fica no quadro 0. Pausar
congela as animacoes.

## Onde a arte e usada

- Todos os PNG de `assets/` sao listados em `ASSETS_EMBUTIDOS` no Makefile,
  embutidos no executavel e decodificados por `graficos/Sprite` via libpng, com
  transparencia e filtro de pixel art (nearest). O jogo nao abre arquivos de
  imagem ao rodar. A pasta `assets/` contem apenas arquivos usados pelo build; a
  unica excecao aos embutidos e `icone/jogo.ico`, o icone do `.exe` no Windows.
- `graficos/CatalogoSprites` carrega uma vez as texturas de inimigos, armas e
  pisos; `Jogo` carrega o Tux e a imagem de fim de jogo
  (`personagens/tux-gameover-v2.png`) e libera tudo antes de destruir o
  renderizador. Reiniciar fases nao recarrega imagens.
- `graficos/SpriteInimigo` guarda bitmaps de reserva, usados se a textura PNG de
  um inimigo nao estiver disponivel, e nas miniaturas da tela de derrota e do manual.
- A arma equipada e desenhada separada do corpo e gira com a mira.

## Como substituir uma arte

- Mantenha nome, dimensoes, transparencia e os tres quadros por personagem.
- Exporte PNG RGBA com fundo transparente, desenhado diretamente na grade final,
  sem suavizacao. Nao desenhe barras de vida ou HUD dentro dos sprites.
- Desenhe o corpo principal sobre a regiao da hitbox, no centro do quadro;
  acessorios podem ocupar a margem externa, que nao recebe tiros.
- Para um arquivo novo, inclua `NOME=caminho` em `ASSETS_EMBUTIDOS` e declare o
  nome em `src/include/graficos/AssetsEmbutidos.h`. Recompile depois.

## Convencao tecnica

`Jogador` e `Inimigo` recebem o perfil no construtor. `x` e `y` sao o canto
superior esquerdo da hitbox. `get_retangulo()` retorna a colisao e
`get_retanguloVisual()` o destino da imagem, ambos no espaco do mundo; a camera
converte para a tela. Tiros, paredes e portas usam a hitbox; mira e disparos
usam seu centro; a barra de vida fica acima do retangulo visual.

Um perfil retangular tambem funciona, por exemplo `{24, 32, 48, 64}`, desde que o
quadro contenha a hitbox e todas as dimensoes sejam positivas.

Pressione **F2 durante o jogo** (ou ative em Configuracoes) para mostrar o
contorno verde das hitboxes.
