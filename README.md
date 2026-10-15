# projeto_semestral_aed1

Roguelike top-down em C++ com SDL2.

## Compilar e rodar

Precisa de SDL2 e libpng (sem SDL2_ttf: o texto usa uma fonte de bitmap embutida).

```sh
make        # gera ./jogo
make run    # compila e executa
make clean  # apaga build/ e o executavel
```

Arch: `sudo pacman -S sdl2 libpng` · Debian/Ubuntu: `sudo apt install libsdl2-dev libpng-dev`

## Controles

| Entrada | Acao |
| --- | --- |
| `W` `A` `S` `D` ou setas | mover dentro da sala |
| mouse | mirar: a arma gira acompanhando o cursor |
| botao esquerdo | atirar (segurar mantem o tiro na cadencia) |
| `F` | coletar a arma mais proxima (ate 54 pixels), se houver vaga |
| `Q` | passar a arma equipada para o fim da fila |
| `G` | soltar a arma equipada no chao |
| `F2` | mostrar/esconder hitboxes para conferir sprites |
| `R` | reiniciar a fase atual e o jogador, mantendo fases ja concluidas |
| `F5` | tentar salvar novamente se houver falha de gravacao |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).

## Como esta agora

Uma campanha com **cinco fases**, cada uma com tres salas conectadas.
A tabela mostra a primeira fase; cada fase seguinte acrescenta 1 a dificuldade
e 2 inimigos por sala.
Cada sala tem seus proprios inimigos, projeteis e requisitos de liberacao.

| Sala | Tamanho | Dificuldade | Inimigos | Requisito para abrir portas |
| --- | --- | --- | --- | --- |
| Setor de Boot | 936x516 | 1 | 4 | Eliminar todos |
| Rede Infectada | 1120x640 | 2 | 7 | Eliminar todos |
| Nucleo em Quarentena | 1440x800 | 3 | 10 | Eliminar todos |

As portas vermelhas estao bloqueadas; verdes estao liberadas. Encoste na
porta direita para avancar automaticamente. Nas salas seguintes, a porta esquerda
permite voltar depois de completar o desafio. Salas ja liberadas permanecem
vazias ao revisitar. A saida direita da ultima sala conclui a fase.

A dificuldade aumenta vida e velocidade dos inimigos. Paredes limitam movimento
e tiros; a camera acompanha o jogador dentro dos limites da sala. O HUD agrupa vida, escudo e energia numa caixa compacta e semitransparente
no canto superior esquerdo, com barras coloridas e icones. A municao aparece
apenas como numero no canto inferior direito. Sala, dificuldade e quantidade de
inimigos aparecem somente na pausa. `Esc` pausa o jogo. `R` permite recomecar, inclusive se acabar a municao.

As configuracoes das salas sao fixas. Dano de contato ao jogador, bosses,
recompensas e geracao procedural ainda serao adicionados conforme as decisoes
do projeto.

## Tela inicial e salvamento

- **Novo jogo** inicia a fase 1 e substitui o progresso salvo.
- **Continuar** retoma o inicio da proxima fase ainda nao concluida.
- **Sair** fecha o jogo. No menu de pausa ha tambem **Menu inicial**.

O jogo salva automaticamente ao iniciar uma nova campanha e ao concluir cada
fase. O arquivo `saves/progresso.json` fica ao lado do executavel, mesmo quando
o jogo e iniciado de outro diretorio. Ele nao e incluido no Git e `make clean`
nao o remove. Exemplo apos concluir duas fases:

```json
{
  "versao": 1,
  "fases_concluidas": 2
}
```

O salvamento e por fase: sair no meio da fase faz `Continuar` voltar ao inicio
dela. Salas, tiros e posicoes em andamento nao sao serializados. Cada fase comeca
com vida, estamina e municao iniciais, tanto ao avancar quanto ao carregar.
Apos a quinta fase, aparece a tela de vitoria e o save registra cinco fases
concluidas. `Continuar` fica desabilitado; escolha `Novo jogo` para recomecar.

Sem save, com arquivo invalido ou com campanha concluida, a tela inicial explica
por que nao e possivel continuar. Falhas de escrita aparecem nos menus/pausa e podem ser
repetidas com `F5`. O arquivo anterior so e substituido depois da escrita completa
de um arquivo temporario. Se sair antes de uma gravacao bem-sucedida, apenas o
ultimo checkpoint gravado estara disponivel.

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
| `mundo/` | `Sala`: entidades; `Fase`: salas; `Campanha`: cinco fases |
| `persistencia/` | `ArquivoProgresso`: leitura e escrita do checkpoint JSON |
| `sistemas/` | `Combate`: disparos, inimigos, colisoes e remocao de inativos |
| `nucleo/` | `main`, `Jogo`: janela, eventos, estados e coordenacao |
| `graficos/` | `Camera`, `Desenho`, `Sprite`: coordenadas, desenho e texturas PNG |
| `interface/` | `Hud`: barras; `Menu`, `Texto`: menus e fonte bitmap |
| `config/` | `Config.h`: resolucao, paleta e constantes |

O Makefile descobre os fontes nos subdiretorios e espelha essa estrutura em
`build/`. O executavel continua sendo `./jogo`. O jogo trabalha em 960x540 e o
SDL escala a imagem quando a janela e redimensionada.

Veja [Arquitetura e TADs](docs/arquitetura.md) para contratos, custos das
operacoes e exemplos de aplicacao das estruturas ao jogo.

## Area de jogo e HUD

A sala inicial ocupa 936x516 dos 960x540 pixels logicos, com paredes de 12 pixels.
A camera das salas maiores usa a mesma margem, sem reservar faixas para paineis.
O HUD e uma sobreposicao e nao bloqueia movimento, mira ou cliques.
No canto superior direito, o inventario mostra tres espacos na ordem da fila:
a arma equipada fica a esquerda em um quadrado de 64x64 com borda dourada;
as duas seguintes ficam em quadrados de 48x48. Cada tipo tem um sprite de
pixel art embutido; vagas livres ficam vazias. Q atualiza a ordem mostrada,
e F/G atualizam os espacos ao coletar/soltar armas.

Vida (rosa/vermelho), escudo (azul) e energia (amarelo) aparecem em barras com
icones, valores e preenchimento proporcional. As tres barras ficam juntas em uma unica caixa de 200x104 pixels no canto
superior esquerdo. A municao aparece somente como numero no canto inferior
direito, sem caixa ou legenda. O escudo inicia com 50/50; vida e energia com 100/100. Os valores voltam
ao inicial junto com o jogador ao iniciar/reiniciar uma fase. Ainda nao existem
dano de contato, absorcao de dano pelo escudo ou consumo de energia no combate.
Os rodapes de ajuda e as mensagens de save ficam ocultos durante a partida;
mensagens de salvamento podem ser consultadas na pausa ou nos outros menus.

## Sprites

Hitbox e imagem agora possuem tamanhos separados e configuraveis por entidade.
Os padroes sao: heroi 32x32 / sprite 48x48; inimigo comum 28x28 / sprite 48x48;
guarda-costas 44x44 / sprite 64x64; boss 88x88 / sprite 128x128.
Os dois ultimos sao perfis preparados para uso futuro, sem novos inimigos nas salas.
Veja o [guia de sprites](docs/sprites.md) para medidas, alinhamento e exportacao.
`F2` mostra a hitbox verde sobre os retangulos visuais provisórios.

O jogador agora usa `assets/sprites/personagens/tux.png`. A imagem original de
48x61 e encaixada proporcionalmente no quadro de 48x48, sem recorte ou distorcao,
com transparencia e filtro de pixel art. A hitbox continua 32x32 e a arma gira
separadamente. O sprite e estatico, ainda sem animacao.

Mantenha a pasta `assets/` ao lado de `jogo` ao copiar o executavel. Os recursos
sao localizados pelo diretorio do executavel, independentemente do diretorio do
terminal. Se o PNG estiver ausente ou invalido, o jogo informa o erro ao iniciar.

## Armas e inventario

`Armas` representa cada arma e `FilaArmas` guarda ate tres: coleta entra no
fim, a frente fica equipada. Q retira a frente e a reinsere no fim; G retira
somente a frente e preserva sua municao no chao. F com fila cheia nao coleta
nem substitui nada. As sete novidades aparecem identificadas no chao de cada
sala, na metade esquerda; coletas e descartes persistem ao revisitar a sala.
A fase seguinte e R reiniciam armas e inventario. A pausa mostra arma equipada,
ocupacao da fila e controles. Os desenhos das armas ainda sao provisorios.

| Arma | Ataque |
| --- | --- |
| Pistola inicial | Um projetil reto |
| Shotgun | Exatamente tres projeteis: reto e diagonais a -45 e +45 graus |
| Laser | Raio instantaneo que atravessa inimigos ate a parede ou 1600 pixels |
| Dupla | Um projetil para frente e outro para tras |
| Ricochete | Projetil que rebate ate quatro vezes nas paredes |
| Espada | Golpe em leque de 90 graus, alcance de 80 pixels, um acerto por inimigo |
| Arco | Flecha com dano 2 e alcance de 1200 pixels |
| Boomerang | Vai ate 260 pixels e retorna buscando o jogador; acerta uma vez por inimigo na ida e na volta |

Cada ataque consome uma unidade de municao, inclusive shotgun e dupla.
As novas armas com municao iniciam com 40 unidades. Espada e boomerang sao
reutilizaveis e mostram `--` no contador. Cada tipo tem sua cadencia; trocar
arma nao cancela o intervalo do ultimo ataque. Projeteis e efeitos sao
removidos ao sair da sala, inclusive boomerangs em voo.
