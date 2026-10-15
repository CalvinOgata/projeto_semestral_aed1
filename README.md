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
| `W` `A` `S` `D` ou setas | mover dentro da sala |
| mouse | mirar: a arma gira acompanhando o cursor |
| botao esquerdo | atirar (segurar mantem o tiro na cadencia) |
| `E` | usar uma porta liberada quando estiver perto |
| `R` | reiniciar toda a fase e o jogador |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).

## Como esta agora

Uma fase com tres salas conectadas, com tamanhos e dificuldades crescentes.
Cada sala tem seus proprios inimigos, projeteis e requisitos de liberacao.

| Sala | Tamanho | Dificuldade | Inimigos | Requisito para abrir portas |
| --- | --- | --- | --- | --- |
| Setor de Boot | 800x400 | 1 | 4 | Eliminar todos |
| Rede Infectada | 1120x640 | 2 | 7 | Eliminar todos |
| Nucleo em Quarentena | 1440x800 | 3 | 10 | Eliminar todos e permanecer 20 segundos ativos |

As portas vermelhas estao bloqueadas; verdes estao liberadas. Aproxime-se da
porta direita e aperte `E` para avancar. Nas salas seguintes, a porta esquerda
permite voltar depois de completar o desafio. Salas ja liberadas permanecem
vazias ao revisitar. A saida direita da ultima sala conclui a fase.

A dificuldade aumenta vida e velocidade dos inimigos. Paredes limitam movimento
e tiros; a camera acompanha o jogador dentro dos limites da sala. O painel
mostra sala, dificuldade, inimigos, municao e tempo restante. `Esc` pausa tambem
o cronometro. `R` permite recomecar, inclusive se acabar a municao.

As salas desta primeira fase sao fixas. Dano de contato ao jogador, bosses,
recompensas e geracao procedural ainda serao adicionados conforme as decisoes
do projeto.

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
| `mundo/` | `Sala`: entidades e requisitos; `Fase`: sequencia e navegacao |
| `sistemas/` | `Combate`: disparos, inimigos, colisoes e remocao de inativos |
| `nucleo/` | `main`, `Jogo`: janela, eventos, estados e coordenacao |
| `graficos/` | `Camera`, `Desenho`: coordenadas e desenho auxiliar |
| `interface/` | `Menu`, `Texto`: menu de pausa e fonte bitmap |
| `config/` | `Config.h`: resolucao, paleta e constantes |

O Makefile descobre os fontes nos subdiretorios e espelha essa estrutura em
`build/`. O executavel continua sendo `./jogo`. O jogo trabalha em 960x540 e o
SDL escala a imagem quando a janela e redimensionada.

Veja [Arquitetura e TADs](docs/arquitetura.md) para contratos, custos das
operacoes e exemplos de aplicacao das estruturas ao jogo.
