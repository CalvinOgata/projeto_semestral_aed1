# projeto_semestral_aed1

Roguelike top-down em C++ com SDL2.

## Compilar e rodar

SDL2, libpng e zlib **nao precisam estar instaladas**: o codigo-fonte delas (versoes
fixadas) fica em `externo/` e o Makefile as compila como bibliotecas estaticas na
primeira compilacao (cerca de 1 minuto). Os sprites usados sao copiados para dentro
do executavel. O resultado e um unico arquivo, sem `.so`/DLL nem pasta `assets/`.
Nao ha SDL2_ttf: o texto usa uma fonte de bitmap embutida.

```sh
make            # gera ./jogo
make run        # compila e executa
make clean      # apaga o codigo compilado do jogo e os executaveis
make clean-tudo # tambem apaga as bibliotecas ja compiladas (recompila o SDL2)
```

Ferramentas necessarias: `g++`, `make`, `cmake`, `pkg-config` e `python3`. No Linux o
SDL2 so ativa o suporte a X11/Wayland/audio cujos cabecalhos (`-dev`) existirem ao
compilar. No Arch eles ja vem nos pacotes normais; no Debian/Ubuntu instale os
listados em `ferramentas/docker/linux.Dockerfile`, ou use `make dist-linux`.

### Executaveis para distribuir (Linux e Windows)

Precisa apenas do Docker; cada build roda num container com as ferramentas certas:

```sh
make dist-linux     # gera dist/jogo      (Linux x86-64, glibc 2.35+)
make dist-windows   # gera dist/jogo.exe  (Windows 64 bits, compilado com MinGW-w64)
make dist           # os dois
```

- `dist/jogo` e compilado no Ubuntu 22.04 e roda nele e em distros mais novas
  (Debian 12+, Fedora 36+, Arch...). So depende da libc; X11, Wayland, OpenGL e
  audio sao abertos pelo SDL a partir do sistema, como em qualquer jogo Linux.
- `dist/jogo.exe` so usa DLLs que acompanham o proprio Windows.
- Basta enviar o arquivo. Os saves sao criados em `saves/` ao lado dele.

Sem Docker, `make ALVO_SO=windows` compila o `.exe` localmente se o
`x86_64-w64-mingw32-g++` estiver instalado.

### Downloads automaticos (GitHub Actions)

O workflow `.github/workflows/executaveis.yml` compila Linux, Windows e web no GitHub:

- **Push no `main`:** atualiza a pre-release `desenvolvimento` (aba *Releases*) com
  `MCGA-windows-x86_64.exe`, `MCGA-linux-x86_64.tar.gz` e `MCGA-web.zip` do ultimo commit.
- **Tag `v*`:** cria uma release oficial com os mesmos arquivos:
  `git tag v1.0 && git push origin v1.0`.
- **Pull requests:** so compilam (os arquivos ficam em *Actions → execucao → Artifacts*).

As bibliotecas de `externo/` e os ports do Emscripten ficam em cache, entao so a
primeira execucao (ou uma troca de versao) recompila o SDL2.

### Deploy automatico da versao web (nginx)

O job `deploy-web` envia o `index.html` de cada push no `main` para um servidor por
SSH (`scp` para `index.html.novo` e depois `mv`, sem deixar o site pela metade). Ele so
roda se a variavel `DEPLOY_HOST` existir no repositorio.

**1. No servidor**, crie um usuario so para o deploy e a pasta do jogo:

```sh
sudo useradd -m -s /bin/sh deploy
sudo mkdir -p /var/www/meujogo && sudo chown deploy: /var/www/meujogo
```

**2. No seu computador**, gere uma chave exclusiva (sem senha) e autorize-a no servidor:

```sh
ssh-keygen -t ed25519 -N "" -C "deploy MCGA" -f ~/.ssh/mcga_deploy
# no servidor, em /home/deploy/.ssh/authorized_keys, uma linha:
#   restrict ssh-ed25519 AAAA... deploy MCGA
ssh-keyscan -p 22 SEU_SERVIDOR > /tmp/known_hosts_mcga   # opcional, veja abaixo
```

**3. No GitHub** (*Settings → Secrets and variables → Actions*), ou pelo terminal:

```sh
gh secret set DEPLOY_SSH_KEY      < ~/.ssh/mcga_deploy
gh secret set DEPLOY_KNOWN_HOSTS  < /tmp/known_hosts_mcga   # opcional
gh variable set DEPLOY_HOST --body "SEU_SERVIDOR"
gh variable set DEPLOY_USER --body "deploy"
gh variable set DEPLOY_DIR  --body "/var/www/meujogo"
gh variable set DEPLOY_URL  --body "https://meudominio.com.br/meujogo/"
gh variable set DEPLOY_PORT --body "22"        # opcional, padrao 22
```

`DEPLOY_KNOWN_HOSTS` e opcional. Com ele, o deploy so conecta se a identidade do
servidor for a cadastrada. Sem ele, o workflow le a identidade do proprio servidor a
cada execucao (`ssh-keyscan`) e mostra um aviso: funciona, mas nao detecta alguem se
passando pelo servidor.

**4. No nginx**, dentro do `server { ... }` do dominio:

```nginx
location = /meujogo { return 301 /meujogo/; }

location /meujogo/ {
    alias /var/www/meujogo/;
    index index.html;
    # Sempre confere se ha versao nova, para o deploy aparecer na hora.
    add_header Cache-Control "no-cache";
    gzip on;
    gzip_types text/html;
}
```

Depois `sudo nginx -t && sudo systemctl reload nginx`. Ao final, o job baixa
`DEPLOY_URL` e confere se a pagina do jogo esta no ar.

### Assets embutidos

`ferramentas/embutir_assets.py` converte os PNGs listados em `ASSETS_EMBUTIDOS` no
Makefile em um `.cpp` gerado dentro de `build/`. Para usar uma nova imagem:
declare-a em `src/include/graficos/AssetsEmbutidos.h`, adicione `NOME=caminho` em
`ASSETS_EMBUTIDOS` e carregue com `Sprite::carregar(renderizador, AssetsEmbutidos::NOME)`.

### Versao web (opcional)

So e necessaria para gerar a versao de navegador; o `make` normal nao depende dela.
Instale o [emsdk](https://github.com/emscripten-core/emsdk) fora da pasta do projeto:

```sh
git clone https://github.com/emscripten-core/emsdk.git ~/emsdk
cd ~/emsdk && ./emsdk install latest && ./emsdk activate latest
source ~/emsdk/emsdk_env.sh   # em cada terminal (ou no ~/.zshrc / ~/.bashrc)
```

```sh
make web       # gera build-web/index.html (unico arquivo, codigo e sprites embutidos)
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
| setas laterais / `A` `D` | ajustar configuracoes e trocar a aba do manual |
| `Enter` ou `Espaco` | confirmar |

O menu tambem responde ao mouse (passar por cima seleciona, clique confirma).
A tela **Manual** do proprio jogo repete estes comandos e resume as regras.

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
com o titulo PAUSADO e as opcoes Continuar, Manual, Reiniciar, Menu inicial e Sair.
Manual abre o mesmo manual do menu principal e volta para a pausa. Os comandos nao
ficam mais escritos no piso do lobby: so aparecem no manual.
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

O jogo salva automaticamente ao iniciar uma nova campanha e ao concluir cada
fase. O arquivo `saves/progresso.json` fica ao lado do executavel, mesmo quando
o jogo e iniciado de outro diretorio. Ele nao e incluido no Git e nem
`make clean` nem `make clean-tudo` o removem. Exemplo apos concluir uma fase com laser, espada e pistola:

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
| `persistencia/` | `ArquivoProgresso`: checkpoint JSON; `ArquivoConfiguracoes`: preferencias |
| `audio/` | `Sintetizador`, `Repertorio`: sons gerados por codigo; `Som`: mixer SDL |
| `sistemas/` | `Combate`: disparos, inimigos, colisoes e remocao de inativos |
| `nucleo/` | `main`, `Jogo`: janela, eventos, estados e coordenacao; `Plataforma`: diferencas desktop/web |
| `graficos/` | `Camera`, `Desenho`, `Sprite`: coordenadas, desenho e texturas PNG |
| `interface/` | `Hud`: barras; `Menu`, `Texto`: menus e fonte bitmap; `TelaCarregamento`, `FundoMenu`: abertura e cenario animado |
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

O jogador usa `assets/sprites/personagens/tux-pixel-v2.png` (1254x1254), encaixada
proporcionalmente no quadro de 48x48, sem recorte ou distorcao,
com transparencia e filtro de pixel art. A hitbox continua 32x32 e a arma gira
separadamente. O sprite e estatico, ainda sem animacao.

Os PNGs usados sao embutidos no executavel durante a compilacao (veja
"Assets embutidos"); a pasta `assets/` so e necessaria para compilar.

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
