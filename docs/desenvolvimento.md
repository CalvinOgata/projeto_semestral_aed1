# Guia de desenvolvimento

Convencoes de codigo, organizacao das pastas e como adicionar imagens ao jogo.

## Convencoes de nome

| Tipo | Padrao | Exemplo |
| --- | --- | --- |
| classes | `PascalCase` | `FilaArmas`, `Jogador` |
| metodos | comecam em minuscula | `vazio()`, `inserir()`, `atirar()` |
| metodo com frase verbal | `snake_case` | `levar_dano()`, `mirar_em()`, `afastar_de()` |
| metodo com nome composto | `camelCase` no final | `get_vidaMax()`, `get_anguloMira()` |
| atributos | `camelCase` | `vidaMax`, `quantidadeDisparos` |
| constantes | `MAIUSCULA_COM_SUBLINHADO` | `LADO_JOGADOR`, `MAX_ARMAS` |

O `.clang-format` cuida do espacamento, mas **nao** verifica nomes: a regra
acima e mista (verbo em `snake_case`, substantivo composto em `camelCase`) e o
`readability-identifier-naming` do clang-tidy nao consegue descrever isso. Se um
dia o grupo quiser verificacao automatica, o caminho e adotar `snake_case` puro
(`get_vida_max`) e ligar o clang-tidy.

## Estrutura

Cabecalhos ficam em `src/include/` e implementacoes em `src/source/`, com os
mesmos modulos nos dois diretorios (templates ficam apenas nos cabecalhos):

| Modulo | Responsabilidade |
| --- | --- |
| `estruturas/` | TADs genericos: fila circular e lista sequencial |
| `inventario/` | `FilaArmas`: aplica a fila ao inventario |
| `entidades/` | `Jogador`, `Inimigo`, `Projetil`, `Armas` |
| `mundo/` | `Sala`: entidades; `Fase`: salas; `Campanha`: tres fases |
| `persistencia/` | `ArquivoProgresso`: checkpoint JSON; `ArquivoConfiguracoes`: preferencias |
| `audio/` | `Sintetizador`, `Repertorio`: sons gerados por codigo; `Som`: mixer SDL |
| `sistemas/` | `Combate`: disparos, inimigos, colisoes e remocao de inativos |
| `nucleo/` | `main`, `Jogo`: janela, eventos, estados e coordenacao; `Plataforma`: diferencas desktop/web |
| `graficos/` | `Camera`, `Desenho`, `Sprite`: coordenadas, desenho e texturas PNG |
| `interface/` | `Hud`: barras; `Menu`, `Texto`: menus e fonte bitmap; `TelaCarregamento`, `FundoMenu`: abertura e cenario animado; `ControlesToque`: joystick e botoes de toque |
| `config/` | `Config.h`: resolucao, paleta e constantes |

O Makefile descobre os fontes nos subdiretorios e espelha essa estrutura em
`build/`. O executavel continua sendo `./jogo`. O jogo trabalha em 960x540 e o
SDL escala a imagem quando a janela e redimensionada.

Veja [Arquitetura e TADs](arquitetura.md) para contratos, custos das
operacoes e exemplos de aplicacao das estruturas ao jogo.

## Assets embutidos

`ferramentas/embutir_assets.py` converte os PNGs listados em `ASSETS_EMBUTIDOS` no
Makefile em um `.cpp` gerado dentro de `build/`. Para usar uma nova imagem:
declare-a em `src/include/graficos/AssetsEmbutidos.h`, adicione `NOME=caminho` em
`ASSETS_EMBUTIDOS` e carregue com `Sprite::carregar(renderizador, AssetsEmbutidos::NOME)`.
