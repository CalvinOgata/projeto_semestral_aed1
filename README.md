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

### Versao web (opcional)

So e necessaria para gerar a versao de navegador; o `make` normal nao depende dela.
Instale o [emsdk](https://github.com/emscripten-core/emsdk) fora da pasta do projeto:

```sh
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
source ~/emsdk/emsdk_env.sh   # em cada terminal (ou no ~/.zshrc / ~/.bashrc)
```

```sh
make web       # gera build-web/index.html (unico arquivo, codigo e assets embutidos)
make web-zip   # gera jogo-web.zip para o itch.io
```

O `build-web/index.html` abre direto no navegador (duplo clique), sem servidor.
No navegador os saves ficam no IndexedDB do proprio navegador e a opcao SAIR
nao aparece nos menus. O layout da pagina esta em `web/shell.html`.

Publicar no itch.io: projeto do tipo **HTML**, enviar `jogo-web.zip`, marcar
"This file will be played in the browser", tamanho do embed 960x540 e ativar o
botao de tela cheia.

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

| Cor/tipo | Inimigo basico | Inimigo medio | Boss |
|------|------|------------------------|------|
| Vermelho — Malware | ILOVEYOU.EXE | MIRAI SWARM | WANNACRY PRIME |
| Azul — Software fechado | EULA-BOT | SENTINELA DRM | COMPILADOR FECHADO |
| Verde — Hardware com IA | NPU DRONE | TPU SENTINEL | H100 COLOSSUS |

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
da sala; pausa congela o tempo). Morrer reinicia a fase atual sem perder fases concluidas.

A dificuldade aumenta vida e velocidade dos inimigos. Sobre o balanceamento bruto,
o jogo aplica primeiro uma reducao de 25% e depois outra de 10% sobre o restante,
tanto na vida final dos inimigos quanto no dano de seus tiros. Na fase 1, as vidas
sao 2/13/67 e os danos 5/8/11.
Paredes limitam movimento
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

O minimapa mostra apenas as salas ja reveladas na arvore: inicialmente o lobby;
ao liberar uma sala, seus filhos aparecem, mesmo antes da visita. M apenas consulta
a arvore. A sala atual recebe borda amarela e um marcador. Verde indica sala concluida, cinza nao visitada e vermelho
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
proxima fase comeca. `tipo` e o indice de `TipoArma`: 0 Zapper 8-Bits, 1 Escopeta
ClamAV, 2 Raio VirusTotal, 3 Garfo Git, 4 Canhao CRT, 5 Lamina de Torvalds,
6 Arco Apache, 7 Disquete Bumerangue e 8 Pulso Malwarebytes. Os indices 0 a 7
foram preservados, portanto saves antigos
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
| `nucleo/` | `main`, `Jogo`: janela, eventos, estados e coordenacao; `Plataforma`: diferencas desktop/web |
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

| Tipo | Alcance | Arma | Ataque |
| --- | --- | --- | --- |
| Antivirus (vermelho) | Curto | Escopeta ClamAV | Tres projeteis: reto e laterais a -15 e +15 graus |
| Antivirus (vermelho) | Medio | Pulso Malwarebytes | Pulso grande e mais lento, com 500 pixels de alcance |
| Antivirus (vermelho) | Longo | Raio VirusTotal | Raio instantaneo que atravessa inimigos ate a parede ou 1600 pixels |
| Open Source (azul) | Curto | Lamina de Torvalds | Golpe em leque de 90 graus, alcance de 80 pixels, um acerto por inimigo |
| Open Source (azul) | Medio | Garfo Git | Um projetil para frente e outro para tras |
| Open Source (azul) | Longo | Arco Apache | Flecha de dano-base 2 e alcance de 1200 pixels |
| Hardware retro (verde) | Curto | Disquete Bumerangue | Vai ate 170 pixels e retorna; pode acertar na ida e na volta |
| Hardware retro (verde) | Medio | Zapper 8-Bits | Um projetil reto com 600 pixels de alcance |
| Hardware retro (verde) | Longo | Canhao CRT | Projetil que rebate ate quatro vezes nas paredes |

Cada ataque consome uma unidade de municao, inclusive Escopeta ClamAV e Garfo Git.
As novas armas com municao iniciam com 40 unidades. Lamina e Disquete Bumerangue sao
reutilizaveis e mostram `--` no contador. Cada tipo tem sua cadencia; trocar
arma nao cancela o intervalo do ultimo ataque. Projeteis e efeitos sao
removidos ao sair da sala, inclusive boomerangs em voo.

O cenario usa placas texturizadas com circuitos discretos, paredes segmentadas e portais escuros com bordas violetas nas saidas de fase, desenhados em SDL sem novos arquivos de textura.
