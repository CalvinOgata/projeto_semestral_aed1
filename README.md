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

| Tecla | Acao |
| --- | --- |
| `W` `A` `S` `D` ou setas | mover |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).

## Estrutura

| Arquivo | Papel |
| --- | --- |
| `main.cpp` | ponto de entrada |
| `Jogo.*` | janela, laco principal, estados (jogando / pausado / saindo) |
| `Menu.*` | menu de pausa (continuar / sair) |
| `Jogador.*` | vida, estamina, posicao, movimento e desenho |
| `Armas.*` | municao, dano e disparo |
| `FilaArmas.*` | fila de armas do jogador (FIFO, ate `MAX_ARMAS`) |
| `Texto.*` | fonte de bitmap 5x7 desenhada com retangulos |
| `Config.h` | resolucao, paleta e constantes de ajuste |

O jogo trabalha sempre em 960x540; a janela pode ser redimensionada que o SDL
escala a imagem.
