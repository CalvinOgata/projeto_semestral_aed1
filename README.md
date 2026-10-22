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
| `W` `A` `S` `D` ou setas | mover pelas salas e corredores |
| mouse | mirar quando nao ha inimigos vivos na sala; em combate, a mira acompanha automaticamente o adversario mais proximo |
| botao esquerdo | atirar (segurar mantem o tiro na cadencia) |
| `F` | coletar a arma mais proxima (ate 54 pixels), se houver vaga |
| `Q` | passar a arma equipada para o fim da fila |
| `G` | soltar a arma equipada no chao |
| `F2` | mostrar/esconder hitboxes para conferir sprites |
| `M` | abrir/fechar minimapa da fase; congela o jogo enquanto aberto |
| `F5` | tentar salvar novamente se houver falha de gravacao |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).

## Como esta agora

Uma campanha com **tres fases** ramificadas, todas com salas de 936x516,
lobby, salas de bau e boss. Cada fase une um tema de computacao a uma referencia
de jogos, e seu nome aparece por 3 segundos ao entrar nela:

| Fase | Nome | Guardiao (bruta-monte) | Boss |
|------|------|------------------------|------|
| 1 | FASE DOS COMPILADORES - SPEEDRUN DO BUILD | SEGFAULT, bloco de erro rachado | COMPILADOR FECHADO, maquina trancada com chaves `{ }` |
| 2 | FASE DA REDE - MULTIPLAYER COM LAG | SPAMBOT, envelope com antena | REDE ANTISSOCIAL, icone de app com olho e notificacao |
| 3 | FASE DO KERNEL - GAME OVER EM TELA AZUL | PROCESSO ZUMBI | TELA AZUL DA MORTE, monitor CRT com rosto `:(` |

A fase 1 possui sete salas (Pre-processador, Cache de build, Analise lexica,
Parser, Linker e Compilador fechado):

```text
                  F (boss)
                  |
          D ----- E (2 baus)
          |       |
Lobby --- A ----- C
          |
          B (1 bau)
```

A tem 3 virus. C e D possuem, cada uma, 4 virus e 2 bruta-montes.
O lobby e as salas B/E nao possuem inimigos. As portas abrem ao eliminar todos
os inimigos da sala; atravesse o corredor estreito para chegar a sala seguinte.
Salas vazias ou limpas ficam abertas, permitindo voltar pelo corredor, inclusive em E–F.
Derrote o boss de F e toque no portal central para avancar.
Ao limpar uma sala, os inimigos das salas adjacentes ja ficam visiveis; eles so
comecam a agir quando o jogador entra.
Salas limpas e baus abertos persistem nas revisitas.
A fase 2 (dificuldade 2) possui nove salas (Roteador, Cache DNS, Caixa de spam,
Proxy, Botnet, Ping 999, Loot drop e Rede antissocial):

```text
          B (1 bau)        H (boss)
          |                |
Lobby --- A ----- D ------ F
          |                |
          C ----- E ------ G (2 baus)
```

A tem 4 virus; C, 5 virus e 1 bruta-monte; D, 4 e 2; E, 6 e 2; F, 5 e 3.

A fase 3 (dificuldade 3) possui onze salas, subindo a partir do lobby (Kernel panic,
Rootkit, Ransomware, Spyware, Worm, Cofre de memoria, Deadlock, Save point,
Processos zumbis e Tela azul da morte):

```text
                  J (boss + 2 virus)
                  |
                  I
                  |
          D ----- G ----- E ----- H (2 baus)
          |               |
          B ----- A ----- C ----- F (1 bau)
                  |
                Lobby
```

A tem 5 virus e 1 bruta-monte; B e C, 6 e 2; D e E, 7 e 2; G, 6 e 3; I, 6 e 4.
Nas tres fases, o boss da ultima sala libera o portal central de saida.

Baus abrem automaticamente a menos de 64 pixels e soltam duas armas aleatorias,
incluindo pistola e todas as sete outras armas; repeticoes sao possiveis.
Use F para coletar, Q para trocar e G para soltar uma arma se o inventario lotar.
Na fase 1, armas extras vem dos baus, em vez da distribuicao de teste em cada sala.

Virus tem visual de chip infectado; guardioes e boss usam pixel art embutida
(`graficos/SpriteInimigo`) que muda conforme a fase.
Todos miram e disparam uma arma simples. Escudo absorve dano antes da vida e
regenera 10 pontos/s apos 3 segundos sem dano. Ha 0,5 segundo de protecao apos
um acerto. Cada inimigo morto deixa um coracao de 10 de vida: coleta automatica
por contato quando falta vida, expiracao em 10 segundos de jogo (inclusive fora
da sala; pausa congela o tempo). Morrer reinicia a fase atual sem perder fases concluidas.

A dificuldade aumenta vida e velocidade dos inimigos. Paredes limitam movimento
e tiros; a camera acompanha continuamente o jogador pelas salas e corredores de 96 pixels de largura, sem inimigos. O HUD agrupa vida, escudo e energia numa caixa compacta e semitransparente
no canto superior esquerdo, com barras coloridas e icones. A municao aparece
apenas como numero no canto inferior direito. Na pausa aparecem apenas a fase e o nome da sala,
com o titulo PAUSADO e as opcoes Continuar, Reiniciar, Menu inicial e Sair.
Reiniciar recomeca a fase atual e o jogador, preservando fases concluidas.
O reinicio manual fica exclusivamente nesse menu; a tecla R nao executa nenhuma acao.

As salas de combate, baus e boss possuem cobertura: caixotes (pacotes de dados,
5 de vida) e barreiras de firewall formadas por fileiras de cinco bloquinhos 24x24
(3 de vida cada). Cada acerto atinge um so bloquinho, abrindo frestas na barreira. Eles bloqueiam o movimento de
todos e param os tiros inimigos sem sofrer dano; somente os ataques do jogador os
quebram, e rachaduras aparecem na metade da vida. O ricochete quica neles, o
bumerangue volta ao acerta-los, e laser/espada param na primeira peca do caminho; um golpe de espada atinge so um bloco.
Pecas quebradas continuam quebradas ao revisitar a sala.

As configuracoes das salas sao fixas. Dano de contato e geracao procedural ainda nao estao implementados.

## Tela inicial e salvamento

O minimapa mostra todas as salas e suas conexoes na fase atual, inclusive as nao
visitadas. A sala atual recebe uma borda amarela e um ponto indica a posicao do
jogador dentro dela. Verde indica sala concluida, cinza nao visitada e vermelho
em combate; salas de baus e boss possuem identificacao. Funciona nas tres fases,
inclusive pelo menu de pausa. M ou Esc fecham o mapa e retornam ao estado anterior.
Movimento, tiros e temporizadores ficam congelados durante a consulta.

- **Novo jogo** inicia a fase 1 e substitui o progresso salvo.
- **Continuar** retoma o inicio da proxima fase ainda nao concluida.
- **Sair** fecha o jogo. No menu de pausa ha tambem **Menu inicial**.

O jogo salva automaticamente ao iniciar uma nova campanha e ao concluir cada
fase. O arquivo `saves/progresso.json` fica ao lado do executavel, mesmo quando
o jogo e iniciado de outro diretorio. Ele nao e incluido no Git e `make clean`
nao o remove. Exemplo apos concluir uma fase com laser, espada e pistola:

```json
{
  "versao": 2,
  "fases_concluidas": 1,
  "armas": [
    {"tipo": 2, "municao": 33},
    {"tipo": 5, "municao": 40},
    {"tipo": 0, "municao": 0}
  ]
}
```

`armas` guarda, na ordem da fila (a primeira e a equipada), as armas com que a
proxima fase comeca. `tipo` e o indice de `TipoArma`: 0 pistola, 1 shotgun,
2 laser, 3 dupla, 4 ricochete, 5 espada, 6 arco, 7 boomerang. Saves antigos
(versao 1, sem `armas`) continuam validos e comecam com a pistola.

O salvamento e por fase: sair no meio da fase faz `Continuar` voltar ao inicio
dela. Salas, tiros e posicoes em andamento nao sao serializados. Cada fase comeca
com vida, estamina e municao iniciais, tanto ao avancar quanto ao carregar.
Apos a terceira fase, aparece a tela de vitoria e o save registra tres fases
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
| `mundo/` | `Sala`: entidades; `Fase`: salas; `Campanha`: tres fases |
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
ao inicial junto com o jogador ao iniciar/reiniciar uma fase. Nao existem dano de contato ou consumo de energia no combate.
Os rodapes de ajuda e as mensagens de save ficam ocultos durante a partida;
mensagens de salvamento podem ser consultadas na pausa ou nos outros menus.

## Sprites

Hitbox e imagem agora possuem tamanhos separados e configuraveis por entidade.
Os padroes sao: heroi 32x32 / sprite 48x48; inimigo comum 28x28 / sprite 48x48;
guarda-costas 44x44 / sprite 64x64; boss 88x88 / sprite 128x128.
Os dois ultimos sao usados pelos guardioes e bosses, com pixel art por fase.
Veja o [guia de sprites](docs/sprites.md) para medidas, alinhamento e exportacao.
`F2` mostra a hitbox verde sobre os sprites.

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
nem substitui nada. Em todas as fases, as armas vem dos baus. Coletas e
descartes persistem ao revisitar a sala.
As armas e a municao acompanham o jogador ao avancar de fase; vida, escudo e
estamina recomecam cheios. Reiniciar ou morrer devolve as armas com que a fase
comecou. Se o inventario chegar vazio, a pistola inicial e entregue.
`Continuar` restaura as armas e a municao salvas no checkpoint. As armas no chao usam os mesmos icones do inventario,
em quadros de 36x36 pixels com o nome abaixo, incluindo armas dos baus e descartadas.
Na mao do personagem, a arma equipada usa esse mesmo icone e acompanha a mira.
Trocar ou soltar a arma atualiza o desenho; sem armas, a mao fica sem icone.

| Arma | Ataque |
| --- | --- |
| Pistola inicial | Um projetil reto |
| Shotgun | Exatamente tres projeteis: reto e laterais a -15 e +15 graus |
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

O cenario usa placas texturizadas com circuitos discretos, paredes segmentadas e portais escuros com bordas violetas nas saidas de fase, desenhados em SDL sem novos arquivos de textura.
