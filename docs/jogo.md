# Manual do jogo e regras

Descricao detalhada do comportamento atual do jogo: controles, fases, inimigos,
armas, interface, audio e salvamento.

## Controles

| Entrada | Acao |
| --- | --- |
| `W` `A` `S` `D` ou setas | mover pelas salas e corredores |
| mouse | mirar quando nao ha inimigos vivos na sala; em combate, a mira acompanha automaticamente o adversario mais proximo |
| botao esquerdo ou `Espaco` | atirar (segurar mantem o tiro na cadencia) |
| `F` | coletar a arma mais proxima (ate 54 pixels), se houver vaga |
| `Q` | passar a arma equipada para o fim da fila |
| `G` | soltar a arma equipada no chao |
| `F2` | mostrar/esconder hitboxes para conferir sprites |
| `M` | abrir/fechar minimapa da fase; congela o jogo enquanto aberto |
| `F5` | tentar salvar novamente se houver falha de gravacao |
| `Esc` | abrir o menu de pausa (e voltar ao jogo) |
| setas / `W` `S` | navegar no menu |
| setas laterais / `A` `D` | ajustar configuracoes e trocar a aba do manual |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).
A tela **Manual** do proprio jogo repete estes comandos e resume as regras.

### Celular e tablet (versao web)

O primeiro toque na tela liga os controles de toque (teclado ou mouse os desligam):

| Toque | Acao |
| --- | --- |
| arrastar na metade esquerda | joystick que nasce onde o dedo encosta (meio curso = meia velocidade) |
| segurar o alvo ou qualquer ponto livre da direita | atirar; a mira continua automatica e, sem inimigos, segue o joystick |
| `PEGAR` / `TROCAR` / `SOLTAR` | o mesmo que `F` / `Q` / `G`; `PEGAR` acende perto de uma arma; tocar nas armas do canto tambem troca |
| `II` / `MAPA` | pausa e minimapa (um toque fecha o mapa) |
| toque nas demais telas | vale um clique: menus, resumo da fase, derrota, fim de jogo e creditos; segurar acelera o texto final |

Joystick e tiro funcionam ao mesmo tempo (multitoque). A pagina pede tela cheia
na horizontal no primeiro toque (onde o navegador permite; no iPhone nao) e, com
o aparelho em pe, mostra um aviso para gira-lo. O manual do jogo mostra os
comandos de toque quando eles estao ativos.

## Fases, inimigos e regras

Uma campanha com **tres fases** ramificadas, todas com salas de 936x516,
lobby, salas de bau e boss. Cada fase une um tema de computacao a uma referencia
de jogos, e seu nome aparece por 3 segundos ao entrar nela:

| Cor/tipo | Inimigo basico | Inimigo medio | Boss |
|------|------|------------------------|------|
| Vermelho — Malware | ILOVEYOU.EXE | MIRAI SWARM | WANNACRY PRIME |
| Azul — Software fechado | EULA-BOT | SENTINELA DRM | WINDOWS UPDATE |
| Verde — Computacao com IA | NPU DRONE | GPU H100 | DATACENTER HIPERESCALA |

As classes basico, medio e boss controlam forca e comportamento. A afinidade de
cor e independente: armas vermelhas sao eficazes contra inimigos vermelhos,
azuis contra azuis e verdes contra verdes. Uma combinacao correta causa **2x o
dano-base**; as demais causam 1x. O soco da mao vazia e neutro e sempre causa 1x.

A fase 1 possui sete salas (Pre-processador, Cache de build, Analise lexica,
Parser, Linker e Compilador fechado):

```text
          Lobby
          |
 B ------ A ------ C
(bau)     |        |
          D        E (2 baus)
                   |
                   F (boss)
```

A tem 4 inimigos basicos e 1 medio. C e D possuem, cada uma, 5 basicos e 2 medios. A, C e D recebem
uma onda de reforco.
O lobby e as salas B/E nao possuem inimigos. As portas abrem ao eliminar todos
os inimigos da sala; atravesse o corredor estreito para chegar a sala seguinte.
Salas vazias ou limpas ficam abertas, permitindo voltar pelo corredor, inclusive em E–F.
Derrote o boss de F e toque no portal central para avancar.
Ao limpar uma sala, seus filhos entram na arvore e sao preparados; seus inimigos so
comecam a agir quando o jogador entra.
Salas limpas e baus abertos persistem nas revisitas.
A fase 2 (dificuldade 2) possui nove salas (Roteador, Cache DNS, Caixa de spam,
Proxy, Botnet, Ping 999, Loot drop e H100 Colossus):

```text
          Lobby
          |
 B ------ A ------ C
(bau)     |        |
          D        E
          |
          F ------ G (2 baus)
          |
          H (H100 COLOSSUS)
```

A tem 5 basicos e 1 medio; C, 6 e 2; D, 5 e 2; E, 7 e 2; F, 6 e 3. D e F
recebem duas ondas de reforco; as demais salas de combate, uma.

A fase 3 (dificuldade 3) possui onze salas, com lobby no topo (Kernel panic,
Rootkit, Ransomware, Spyware, Worm, Cofre de memoria, Deadlock, Save point,
Processos zumbis e WannaCry Prime):

```text
          Lobby
          |
 B ------ A ------ C ------ F (bau)
 |                 |
 D ------ G        E ------ H (2 baus)
          |
          I
          |
          J (WANNACRY PRIME + 3 basicos)
```

A tem 6 basicos e 2 medios; B e C, 7 e 2; D, 8 e 2; E, 8 e 3; G, 7 e 3; I, 7 e 4.
A recebe uma onda de reforco, I recebe tres e as demais, duas.
Nas tres fases, o boss da ultima sala libera o portal central de saida.

Os desenhos acima e o minimapa preservam as posicoes relativas das salas:
esquerda, direita, cima e baixo correspondem ao cenario real. A escala e compactada.
Toda passagem liga exatamente pai e filho, nos dois sentidos; nao existem
atalhos entre ramos. A tabela de pais gera as portas e corredores, e tambem
governa a descoberta no minimapa. Cada no do mapa informa ESQ/DIR/CIMA/BAIXO:
a porta a usar na sala pai para chegar a ele. Os nos ja revelados nao mudam de
posicao ao descobrir outros ramos. As posicoes e o conteudo das salas continuam
fixos por fase.

Baus abrem automaticamente a menos de 64 pixels e soltam duas armas: a primeira
sempre tem a afinidade eficaz contra o boss da fase, e a segunda e sorteada entre
os nove tipos; repeticoes sao possiveis.
Use F para coletar, Q para trocar e G para soltar uma arma se o inventario lotar.
Na fase 1, armas extras vem dos baus, em vez da distribuicao de teste em cada sala.

Cada uma das nove combinacoes de classe e afinidade tem nome e pixel art embutida
provisoria em `graficos/SpriteInimigo`, pronta para posterior troca por PNG autoral.
Quando resta no maximo um inimigo, a proxima onda de reforco e anunciada por
molduras rosas piscando durante 1 segundo, longe do jogador; a sala so libera
depois de todas as ondas. Os inimigos mantem distancia, circulam o jogador,
contornam a cobertura quando ela bloqueia a mira (sem desperdicar tiros) e
antecipam o movimento do jogador. Bruta-montes abrem um leque de tres tiros a
cada tres ataques. O boss alterna leque mirado, anel giratorio de 12 tiros e a
invocacao de dois virus, e atira mais rapido abaixo de metade da vida. Cada fase
aumenta vida, velocidade, cadencia e velocidade dos tiros inimigos. Escudo absorve dano antes da vida e
regenera 10 pontos/s apos 3 segundos sem dano. Ha 0,5 segundo de protecao apos
um acerto. Cada inimigo morto deixa um coracao de 10 de vida: coleta automatica
por contato quando falta vida, expiracao em 10 segundos de jogo (inclusive fora
da sala; pausa congela o tempo).

A campanha tem **3 vidas** e **um unico continue**. Morrer gasta uma vida, mostra a
animacao de derrota e reinicia a fase atual, sem perder fases concluidas. Sem vidas,
aparece o game over com contagem de 10 s: aceitar o continue devolve as 3 vidas e
recomeca a fase; recusar (ou ja ter usado o continue) apaga o save do slot.

A pontuacao soma 100 por inimigo basico, 250 por bruto e 2000 por boss, mais um
bonus de rapidez ao concluir a fase (1000 x 300 s / tempo gasto). O resumo de cada
fase mostra essas parcelas e o total da campanha aparece na tela de vitoria.

A dificuldade aumenta vida e velocidade dos inimigos. Sobre o balanceamento bruto,
o jogo aplica primeiro uma reducao de 25% e depois outra de 10% sobre o restante,
tanto na vida final dos inimigos quanto no dano de seus tiros. Na fase 1, as vidas
sao 2/13/67 e os danos 5/8/11.
Paredes limitam movimento
e tiros; a camera acompanha continuamente o jogador pelas salas e corredores de 96 pixels de largura, sem inimigos. O HUD mostra vidas, vida, escudo, pontuacao e tempo
no canto superior esquerdo, direto sobre o jogo, com barras coloridas e icones. A municao aparece
apenas como numero no canto inferior direito. Na pausa aparecem apenas a fase e o nome da sala,
com o titulo PAUSADO e as opcoes Continuar, Manual, Reiniciar, Menu inicial e Sair.
Manual abre o mesmo manual do menu principal e volta para a pausa. Os comandos nao
ficam mais escritos no piso do lobby: so aparecem no manual.
Reiniciar recomeca a fase atual e o jogador, preservando fases concluidas.
O reinicio manual fica exclusivamente nesse menu; a tecla R nao executa nenhuma acao.

As salas de combate, baus e boss possuem cobertura: caixotes (pacotes de dados,
5 de vida) e barreiras de firewall formadas por fileiras de cinco bloquinhos 24x24
(3 de vida cada). Cada acerto atinge um so bloquinho, abrindo frestas na barreira. Eles bloqueiam o movimento de
todos e param os tiros inimigos sem sofrer dano; somente os ataques do jogador os
quebram, e rachaduras aparecem na metade da vida. O Canhao CRT ricocheteia neles, e o Raio Avira e os golpes corpo a corpo param na primeira peca do caminho, atingindo so um bloco.
Pecas quebradas continuam quebradas ao revisitar a sala.

As configuracoes das salas sao fixas. Dano de contato e geracao procedural ainda nao estao implementados.

## Armas e inventario

`Armas` representa cada arma e `FilaArmas` guarda ate tres: coleta entra no
fim, a frente fica equipada. Q retira a frente e a reinsere no fim; G retira
somente a frente e preserva sua municao no chao. F com fila cheia nao coleta
nem substitui nada. Em todas as fases, as armas vem dos baus. Coletas e
descartes persistem ao revisitar a sala.
As armas e a municao acompanham o jogador ao avancar de fase; vida e escudo
recomecam cheios. Reiniciar ou morrer devolve as armas com que a fase
comecou. Se o inventario chegar vazio, o Zapper 8-Bits inicial e entregue.
`Continuar` restaura as armas e a municao salvas no checkpoint. As armas no chao usam os mesmos icones do inventario,
em quadros de 36x36 pixels com o nome abaixo, incluindo armas dos baus e descartadas.
Na mao do personagem, a arma equipada usa esse mesmo icone e acompanha a mira.
Trocar ou soltar a arma atualiza o desenho; sem armas, a mao fica sem icone.

| Tipo | Alcance | Arma | Ataque |
| --- | --- | --- | --- |
| Antivirus (vermelho) | Curto | Laminas Malwarebytes | Setor de 120 graus, 96 px, dano 3, intervalo 0,38 s |
| Antivirus (vermelho) | Medio | Clams do ClamAV | Tres tiros a -15/0/+15 graus, 320 px, dano 1 por tiro, 0,50 s |
| Antivirus (vermelho) | Longo | Raio Avira | Raio instantaneo que atravessa inimigos, para na cobertura, 900 px, dano 3, 0,65 s |
| Open Source (azul) | Curto | Fork Open Source | Estocada em setor de 40 graus, 116 px, dano 4, 0,50 s |
| Open Source (azul) | Medio | Penas Apache | Atravessa ate dois inimigos diferentes, 480 px, dano 2, 0,45 s |
| Open Source (azul) | Longo | Rifle Wireshark | Projetil preciso de 850 px/s, alcance 900 px, dano 4, 0,75 s |
| Hardware retro (verde) | Curto | Chicote Macintosh | Setor de 70 graus, 150 px, dano 3, 0,65 s |
| Hardware retro (verde) | Medio | Zapper 8-Bits | Tiro reto, 420 px, dano 1, 0,20 s |
| Hardware retro (verde) | Longo | Canhao CRT | Ate quatro ricochetes; 1050 px de trajeto total, dano 3, 0,65 s |

Cada ataque a distancia consome uma unidade de municao, inclusive os tres tiros do ClamAV.
As novas armas com municao iniciam com 40 unidades (Zapper: 50). As tres armas corpo a corpo sao
reutilizaveis e mostram `--` no contador. Cada tipo tem sua cadencia; trocar
arma nao cancela o intervalo do ultimo ataque. Projeteis e efeitos sao
removidos ao sair da sala. Cada golpe corpo a corpo acerta cada inimigo no maximo uma vez.

O cenario usa os pisos PNG de baixo contraste de `assets/sprites/pisos`, com identidade
propria por fase e sala; todas as salas de bau usam a mesma textura. Paredes,
obstaculos e portais continuam desenhados por SDL. Personagens possuem tres poses
em faixas PNG de `assets/sprites/final`; armas e HUD usam os mesmos icones.

## Area de jogo e HUD

A sala inicial ocupa 936x516 dos 960x540 pixels logicos, com paredes de 12 pixels.
A camera das salas maiores usa a mesma margem, sem reservar faixas para paineis.
O HUD e uma sobreposicao e nao bloqueia movimento, mira ou cliques.
No canto superior direito, o inventario mostra tres espacos na ordem da fila:
a arma equipada fica a esquerda em um quadrado de 64x64 com borda dourada;
as duas seguintes ficam em quadrados de 48x48. Cada tipo tem um sprite de
pixel art embutido; vagas livres ficam vazias. Q atualiza a ordem mostrada,
e F/G atualizam os espacos ao coletar/soltar armas.

No canto superior esquerdo, sem painel de fundo, ficam: as vidas restantes (cabecas
do Tux), as barras de vida (vermelha) e escudo (azul) com icones e valores, a
pontuacao da campanha e o tempo da fase. A municao aparece somente como numero no
canto inferior direito (`--` nas armas corpo a corpo; vermelho quando acaba). A vida
inicia com 100/100 e o escudo com 50/50; ambos voltam ao inicial ao iniciar ou
reiniciar uma fase. Nao existe dano de contato nem barra de energia.
Os rodapes de ajuda e as mensagens de save ficam ocultos durante a partida;
mensagens de salvamento podem ser consultadas na pausa ou nos outros menus.

## Tela inicial, menus e minimapa

O minimapa mostra apenas as salas ja reveladas na arvore: inicialmente o lobby;
ao liberar uma sala, seus filhos aparecem, mesmo antes da visita. M apenas consulta
a arvore. A sala atual recebe borda amarela e um marcador. Verde indica sala concluida, cinza nao visitada e vermelho
em combate; salas de baus e boss possuem identificacao. Funciona nas tres fases,
inclusive pelo menu de pausa. M ou Esc fecham o mapa e retornam ao estado anterior.
Movimento, tiros e temporizadores ficam congelados durante a consulta.

Ao abrir, o jogo mostra uma tela de carregamento (log de "boot", barra com o Tux
andando e dicas) enquanto sintetiza os sons; ela dura pelo menos 3,6 s e qualquer
tecla encurta a espera depois que fica pronta. Em seguida vem o menu principal:

- **Jogar** abre **Novo jogo** (escolhe um slot e substitui o progresso dele) e
  **Continuar** (retoma o inicio da proxima fase ainda nao concluida).
- **Manual** mostra os comandos e um resumo de como jogar, em duas abas.
- **Configuracoes** ajusta volume da musica e dos efeitos (0 a 10), tela cheia
  (so no desktop) e hitboxes. Fica em `saves/configuracoes.txt` (`chave=valor`).
- **Sobre** lista as tecnologias usadas e a equipe.
- **Sair** fecha o jogo (nao aparece na versao web). Na pausa ha tambem **Menu inicial**.

## Audio

Nao ha arquivos de som: `audio/Sintetizador` gera ondas quadrada, triangular,
serra, senoide e ruido com envelope, varredura de frequencia, vibrato e filtro;
`audio/Repertorio` compoe cada efeito e as tres trilhas (menu, fases e chefe) como
listas de notas; `audio/Som` mistura tudo no dispositivo do SDL. A sintese acontece
durante a tela de carregamento, em fatias de 8 ms por quadro.

- Trilhas: menu em la menor tranquilo; fases em mi menor; chefe em re frigio, que
  entra enquanto a sala do chefe esta em combate. A pausa e o minimapa abafam a trilha.
- Efeitos: um disparo diferente por arma, soco, sem municao, acerto, morte de virus,
  bruto e chefe, tiros inimigos, alerta de reforcos, invocacao, dano no escudo e na
  vida, coracao, portas trancando, sala liberada, bau, coletar/soltar/trocar arma,
  portal, fase concluida, morte, game over, continue, vitoria e navegacao dos menus.
- Ao morrer, uma animacao mostra o sistema sendo invadido por virus e janelas de erro,
  seguida de "NAO DESISTA, GUERREIRO!", um trocadilho de programacao e as vidas restantes
  (Enter acelera). So entao a fase recomeca ou aparece o game over.
- Ao vencer a campanha: celebracao com fogos, um texto reflexivo que sobe devagar com
  trocadilhos de programacao (segure Espaco para acelerar, Esc pula) e os creditos, com
  trilha propria de encerramento.
- Repeticoes rapidas do mesmo efeito tem intervalo minimo e no maximo 4 copias
  simultaneas. Sem dispositivo de audio, o jogo continua mudo.

## Salvamento

O jogo salva automaticamente ao iniciar uma nova campanha e ao concluir cada
fase, e tambem ao perder uma vida. Ha tres slots independentes,
`saves/progresso_1.json` a `progresso_3.json`, na pasta `saves/` ao lado do
executavel, mesmo quando o jogo e iniciado de outro diretorio (na web, ficam no
IndexedDB do navegador). Eles nao sao incluidos no Git e nem `make clean` nem
`make clean-tudo` os removem. Exemplo apos concluir uma fase com Raio Avira, Laminas Malwarebytes e Zapper:

```json
{
  "versao": 3,
  "fases_concluidas": 1,
  "vidas": 2,
  "continue_disponivel": 1,
  "pontuacao_total": 4310,
  "armas": [
    {"tipo": 2, "municao": 33},
    {"tipo": 5, "municao": 40},
    {"tipo": 0, "municao": 0}
  ]
}
```

`armas` guarda, na ordem da fila (a primeira e a equipada), as armas com que a
proxima fase comeca. `tipo` e o indice de `TipoArma`: 0 Zapper 8-Bits, 1 Clams do
ClamAV, 2 Raio Avira, 3 Fork Open Source, 4 Canhao CRT, 5 Laminas Malwarebytes,
6 Penas Apache, 7 Chicote Macintosh e 8 Rifle Wireshark. Os indices 0 a 8
foram preservados. `vidas`, `continue_disponivel` (0 ou 1) e `pontuacao_total`
entraram na versao 3; saves antigos (versao 1 sem `armas`, versao 2 sem esses tres
campos) continuam validos.

O salvamento e por fase: sair no meio da fase faz `Continuar` voltar ao inicio
dela. Salas, tiros e posicoes em andamento nao sao serializados. Cada fase comeca
com vida e escudo cheios; armas e municao vem do checkpoint.
Apos a terceira fase, aparece a tela de vitoria e o save registra tres fases
concluidas. `Continuar` fica desabilitado; escolha `Novo jogo` para recomecar.

Sem save, com arquivo invalido ou com campanha concluida, a tela inicial explica
por que nao e possivel continuar. Falhas de escrita aparecem nos menus/pausa e podem ser
repetidas com `F5`. O arquivo anterior so e substituido depois da escrita completa
de um arquivo temporario. Se sair antes de uma gravacao bem-sucedida, apenas o
ultimo checkpoint gravado estara disponivel.

## Sprites

Hitbox e imagem possuem tamanhos separados e configuraveis por entidade
(hitbox / quadro): heroi 32x32 / 64x64; inimigo comum 28x28 / 48x48;
bruto 44x44 / 128x128; boss 88x88 / 256x256. Personagens sao faixas PNG de tres
quadros animados. Veja o [guia de sprites](sprites.md) para medidas, alinhamento e exportacao.
`F2` mostra a hitbox verde sobre os sprites.

O jogador usa `assets/sprites/final/00_tux.png`, uma faixa com tres quadros
desenhados em 64x64 (ciclo 0,1,0,2 ao andar), com transparencia e filtro de pixel
art. A hitbox continua 32x32 e a arma gira separadamente. Somente os PNG listados em
`ASSETS_EMBUTIDOS` no Makefile ficam em `assets/`.

Os PNGs usados sao embutidos no executavel durante a compilacao (veja
"Assets embutidos" em [desenvolvimento](desenvolvimento.md)); a pasta `assets/` so e necessaria para compilar.
