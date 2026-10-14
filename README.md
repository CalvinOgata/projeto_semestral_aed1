# projeto_semestral_aed1

Roguelike top-down em C++ com SDL2.

## Compilar e rodar

Precisa apenas do SDL2 (sem SDL2_ttf: o texto usa uma fonte de bitmap embutida).

```sh
make        # gera ./jogo
make run    # compila e executa
make clean  # apaga build/ e o executavel
```

Arch: `sudo pacman -S sdl2` · Debian/Ubuntu: `sudo apt install libsdl2-dev`

## Controles

| Entrada | Acao |
| --- | --- |
| `W` `A` `S` `D` ou setas | mover (mapa aberto, sem bordas) |
| mouse | mirar: a arma gira acompanhando o cursor |
| botao esquerdo | atirar (segurar mantem o tiro na cadencia) |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).

## Como esta agora

Mapa aberto no estilo Vampire Survivors / Brotato: nao ha sala nem borda, a
camera segue o jogador e a grade serve so de referencia de movimento. Cinco
inimigos azuis nascem em volta, perseguem o jogador e morrem depois de
`VIDA_INIMIGO` acertos. A municao sai da `FilaArmas` do jogador.

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

```
src/
├── include/   cabecalhos (.h)
└── source/    implementacoes (.cpp)
build/         objetos gerados pelo make (fora do git)
jogo           executavel gerado pelo make (fora do git)
```

| Modulo | Papel |
| --- | --- |
| `main` | ponto de entrada |
| `Jogo` | janela, laco principal, estados, colisoes e listas de entidades |
| `Camera` | converte mundo <-> tela; e o que faz o mapa parecer infinito |
| `Jogador` | vida, estamina, posicao, movimento, mira e arma |
| `Inimigo` | persegue o jogador, leva dano e se afasta dos outros |
| `Projetil` | tiro com direcao, alcance e dano |
| `Armas` | municao, dano e disparo |
| `FilaArmas` | fila de armas do jogador (FIFO, ate `MAX_ARMAS`) |
| `Menu` | menu de pausa (continuar / sair) |
| `Texto` | fonte de bitmap 5x7 desenhada com retangulos |
| `Desenho` | retangulo girado (o cano da arma), via textura de 1x1 |
| `Config.h` | resolucao, paleta e constantes de ajuste |

O jogo trabalha sempre em 960x540; a janela pode ser redimensionada que o SDL
escala a imagem.
