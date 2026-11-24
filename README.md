# Make Computers Great Again

Jogo roguelike de ação com visão de cima (*top-down*), escrito em C++ com SDL2 e
inspirado em **Soul Knight**.

Este projeto é o trabalho semestral da disciplina de **Algoritmos e Estruturas de
Dados 1 (AED1)**. Além de ser um jogo completo, ele mostra na prática como tipos
abstratos de dados (TADs) resolvem problemas reais de um programa.

## Equipe

- Felipe Baratela Pimenta ([@felipebp14-207](https://github.com/felipebp14-207))
- Calvin da Serra Ogata de Paula ([@CalvinOgata](https://github.com/CalvinOgata))
- João Pedro Tomaz dos Santos ([@jsantos43](https://github.com/jsantos43))
- Pedro Testa Lourenço ([@pedro-testa-ptl](https://github.com/pedro-testa-ptl))

## O jogo

O mundo da computação foi tomado por **sistemas malignos**: vírus, software fechado
e hardware de IA descontrolado. Você controla o **Tux**, o pinguim do Linux, que
representa o "código do bem", e precisa libertar os computadores.

- A campanha tem **três fases**. Cada fase é um mapa de salas ligadas por
  corredores, com um lobby, salas de combate, salas de baú e um **chefe** no final.
- As portas trancam durante o combate e só abrem quando todos os inimigos da sala
  (incluindo as ondas de reforço) são derrotados.
- Os inimigos têm três cores: **vermelho** (malware, como ILOVEYOU e WannaCry),
  **azul** (software fechado, como EULA-BOT e Windows Update) e **verde** (hardware
  de IA, como NPU e GPU H100).
- Existem nove armas, também em três cores (antivírus, open source e hardware
  retrô) e três alcances (curto, médio e longo). Usar a arma da **mesma cor** do
  inimigo causa **o dobro de dano**.
- O jogador tem vida, escudo que se regenera, 3 vidas e um *continue*. O progresso
  é salvo automaticamente em três slots.
- Funciona no computador (teclado e mouse) e no navegador, inclusive em celulares
  com controles de toque.

**Controles principais:** `WASD` ou setas para andar, botão esquerdo do mouse ou `Espaço` para
atirar (a mira é automática), `F` pega arma, `Q` troca de arma, `G` solta a arma,
`M` abre o mapa e `Esc` pausa. A tela **Manual**, dentro do jogo, explica tudo.

## Estruturas de dados (TADs)

As estruturas ficam em `src/include/estruturas/` e não dependem do SDL nem de
nenhuma regra do jogo. As classes do jogo apenas as utilizam.

| TAD | Como funciona | Onde é usado no jogo |
| --- | --- | --- |
| **Fila** (`Fila<T, Capacidade>`) | Fila circular (FIFO) em um vetor de tamanho fixo: insere no fim e retira do início, tudo em O(1) | **Inventário de armas** (`FilaArmas`, até 3 armas). A arma da frente é a equipada; `F` insere no fim, `Q` retira a da frente e a coloca no fim, `G` retira a da frente e a deixa no chão |
| **Lista sequencial** (`ListaSequencial<T>`) | Lista com acesso por índice, inserção no fim e remoção por critério | **Salas** de cada fase, e, dentro de cada sala, **inimigos**, **projéteis**, **obstáculos**, **corações**, **baús** e **armas no chão** |
| **Árvore** (`Arvore<T>`) | Árvore enraizada com nós encadeados; cada nó conhece o pai e os filhos; percurso em pré-ordem | **Mapa de cada fase**: o lobby é a raiz e cada sala é filha da sala anterior. Ao limpar uma sala, seus filhos são revelados; só é possível andar entre pai e filho. O **minimapa** desenha a árvore com um percurso em pré-ordem |

Mais detalhes (contratos, custos e decisões) em
[docs/arquitetura.md](docs/arquitetura.md).

## Ferramentas utilizadas

- **C++17**: a linguagem do jogo.
- **SDL2**: abre a janela, desenha na tela, lê teclado/mouse/toque e toca o som.
- **libpng** e **zlib**: leem as imagens PNG dos personagens, armas e pisos.
- **Make** e **CMake**: automatizam a compilação do jogo e das bibliotecas.
- **Emscripten**: transforma o jogo em uma página web (WebAssembly).
- **Docker** e **MinGW-w64**: geram os executáveis prontos para Linux e Windows.
- **GitHub Actions**: compila e publica as versões automaticamente a cada atualização.
- **clang-format**: mantém o código formatado do mesmo jeito para toda a equipe.

Não há arquivos de som: as músicas e efeitos são sintetizados pelo próprio código.

## Como rodar

### Jeito mais fácil: baixar pronto

Na aba **Releases** do repositório há um arquivo único para cada sistema, sem
precisar instalar nada:

- **Windows:** `MCGA-windows-x86_64.exe`, é só abrir.
- **Linux:** `MCGA-linux-x86_64.tar.gz`, extraia e execute `./jogo`.
- **Navegador:** `MCGA-web.zip`, extraia e abra o `index.html`.

### Compilando o código (Linux)

Instale `g++`, `make`, `cmake`, `pkg-config` e `python3`. Na pasta do projeto:

```sh
make run
```

Isso compila e abre o jogo. A primeira compilação demora cerca de um minuto, porque
também compila o SDL2, a libpng e a zlib, que já vêm na pasta `externo/` (não é
preciso instalá-las). Depois, basta rodar `./jogo`.

Os saves ficam na pasta `saves/`, ao lado do executável.

Versão para navegador, executáveis para Windows e outras opções estão em
[docs/compilacao.md](docs/compilacao.md).

## Documentação

| Arquivo | Conteúdo |
| --- | --- |
| [docs/jogo.md](docs/jogo.md) | Manual completo: controles, fases, inimigos, armas, HUD, áudio e saves |
| [docs/compilacao.md](docs/compilacao.md) | Compilação, executáveis, versão web, GitHub Actions e deploy |
| [docs/arquitetura.md](docs/arquitetura.md) | Arquitetura do código e TADs em detalhe |
| [docs/desenvolvimento.md](docs/desenvolvimento.md) | Convenções de nomes, estrutura das pastas e imagens embutidas |
| [docs/sprites.md](docs/sprites.md) | Medidas e regras para criar sprites |
| [docs/integracao_final.md](docs/integracao_final.md) | Registro da integração final da arte e do arsenal |
