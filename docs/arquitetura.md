# Arquitetura e tipos abstratos de dados

O objetivo desta organizacao e mostrar separadamente o contrato de um TAD,
sua implementacao e sua aplicacao no jogo. As estruturas nao conhecem SDL,
armas ou inimigos; as classes do jogo escolhem quando e por que usa-las.

## Responsabilidades e composicao

```text
Jogo (janela, entrada, estados e laco principal)
  |-- Jogador (movimento, mira, vida e estamina)
  |     `-- FilaArmas (inventario: primeira arma = equipada)
  |           `-- Fila<Armas, MAX_ARMAS> (armazenamento FIFO)
  |-- Campanha (tres fases; mantem a fase atual)
  |     `-- Fase (progressao e sala ativa)
  |           `-- ListaSequencial<Sala>
  |                 `-- Sala (tamanho, dificuldade, requisitos e estado)
  |                       `-- Combate (entidades exclusivas desta sala)
  |                             |-- ListaSequencial<Inimigo>
  |                             `-- ListaSequencial<Projetil>
  |-- Camera
  `-- Menu
```

`Jogo` controla a cadencia dos tiros e chama a sala ativa, que delega a
`Combate`. Este consulta a arma
na frente da fila, consome uma unidade de municao por ataque e cria projeteis
ou ataques instantaneos conforme o tipo. Depois atualiza os
projeteis e inimigos, resolve colisoes e remove projeteis inativos. A remocao acontece
apos os percursos para evitar referencias invalidadas durante a iteracao.
A ordem de atualizacao e de desenho do prototipo foi preservada.

Entidades ainda sabem desenhar sua representacao simples via SDL. A separacao
atual isola armazenamento e combate sem criar uma hierarquia de heranca ou um
sistema de renderizacao adicional. `Jogo` nao pode ser copiado, pois possui a
janela e o renderizador SDL e os libera no destrutor.

## Estruturas implementadas

A mira consulta `Sala::buscar_alvoMaisProximo`, que delega a `Combate` uma busca
O(n) pelo inimigo vivo de menor distancia entre centros, sem expor a lista.
`Jogo` atualiza a direcao antes de disparar a cada quadro; sem alvo na sala atual,
usa o mouse. A arma desenhada e os ataques compartilham essa direcao.

| TAD | Implementacao | Uso atual | Custos |
| --- | --- | --- | --- |
| `Fila<T, Capacidade>` | Vetor circular com `std::array`, inicio e quantidade | Armas do jogador | Inserir, retirar, frente e consultar: O(1); memoria O(capacidade) |
| `ListaSequencial<T>` | Armazenamento dinamico via `std::vector` | Salas de `Fase`, inimigos e projeteis de `Combate` | Acesso O(1), inserir O(1) amortizado, remover por criterio O(n) |

A fila foi implementada pelo projeto; a lista sequencial encapsula a biblioteca
padrao. Ela **nao** e uma lista encadeada nem uma implementacao manual de
alocacao. Se a disciplina exigir implementacoes manuais, essa restricao deve
ser registrada antes de substituir o armazenamento.

### Fila e inventario

- `inserir` coloca no fim; `retirar` remove o mais antigo e o devolve pelo
  parametro de saida. Ambas retornam `false` quando nao podem operar.
- Retirar de uma fila vazia preserva o parametro de saida. Inserir em uma fila
  cheia preserva os elementos existentes.
- `frente` devolve ponteiro para o primeiro elemento, ou `nullptr` se vazia.
- `consultar` permite inspecao constante por posicao logica e retorna `nullptr`
  para indice invalido. A fila nao oferece reordenacao ou acesso mutavel ao meio.
- O vetor circular reutiliza espacos liberados sem deslocar os demais elementos.
- O template requer valores construiveis por padrao e copiaveis. Os slots
  existem durante toda a vida da fila; retirar nao destroi imediatamente o slot.
- `FilaArmas` limita a capacidade a tres e oferece apenas operacoes de
  inventario FIFO. O tiro altera a municao da arma da frente, sem remove-la.
  F coleta no fim, Q alterna retirando a frente e reinserindo no fim em O(1),
  e G solta a frente no chao. Fila cheia recusa a coleta sem perder a arma.
- `FilaArmas::consultar` oferece somente leitura em ordem logica para os tres
  espacos do HUD, em O(1), delegando ao TAD sem expor seu armazenamento.
- Consulte novamente ponteiros apos alterar a estrutura.

### Lista e combate

- `inserir` acrescenta ao final; `obter` acessa por indice e lanca
  `std::out_of_range` para indice invalido.
- `remover_se` remove todos os elementos que satisfazem o criterio, preservando
  a ordem dos sobreviventes; `limpar` esvazia a lista.
- Os percursos usam iteradores (`begin`/`end`), incluindo acesso constante.
  Insercoes/remocoes podem invalidar referencias e iteradores.
- A lista cuida do armazenamento; `Combate` decide quais entidades estao mortas
  ou inativas. As colisoes ainda custam O(projeteis * inimigos), e a separacao
  entre pares de inimigos custa O(inimigos²), como no prototipo original.

## Possibilidades futuras, ainda nao implementadas

| Estrutura | Exemplo de aplicacao | Condicao para adotar |
| --- | --- | --- |
| Pilha (LIFO) | Retorno entre submenus: ultimo aberto e o primeiro fechado | Existirem menus aninhados |
| Lista cadastral | Cadastro de definicoes de armas, poderes ou personagens por identificador | Definir registros, chaves unicas e regras de busca/alteracao/exclusao |
| Lista duplamente encadeada | Navegacao anterior/proximo em uma selecao ou inventario | A mecanica exigir percurso bidirecional e insercoes/remocoes locais |
| Fila (FIFO) | Eventos ou ondas de inimigos processados por ordem de chegada | Definir essa mecanica e sua politica de processamento |

Uma lista cadastral descreve operacoes de cadastro; sua representacao pode ser
sequencial ou encadeada. Uma lista duplamente encadeada descreve a representacao
com nos ligados nos dois sentidos. Essas classificacoes nao sao sinonimos.

Novos TADs devem ficar em `estruturas/` e ser usados por classes do dominio.
Escolha a estrutura pelo comportamento necessario, mantenha seus detalhes
internos privados e documente o uso real antes de declarar uma nova estrutura
como parte implementada do jogo.

## Salas e progressao

`ConfiguracaoSala` define nome, largura, altura, dificuldade, quantidade de
inimigos e `RequisitoSala`. Atualmente o requisito e eliminar todos os inimigos,
sem tempo minimo. `eliminarInimigos = false` permite uma sala de passagem livre.

`Sala` transita de `NAO_VISITADA` para `EM_COMBATE` na primeira entrada e para
`LIBERADA` quando cumpre os requisitos. Uma revisita preserva os inimigos e o
estado; apenas a primeira entrada cria entidades. `sair` descarta projeteis.
Apenas a sala ativa simula combate; coracoes nas demais salas tambem envelhecem.
A pausa interrompe todos esses tempos.

`Fase` guarda a sequencia em `ListaSequencial<Sala>` e acessa a atual por indice.
Avancar/voltar nao remove salas da lista; isso preserva seu estado. A navegacao
ocorre ao terminar de atravessar o corredor e exige liberacao da sala atual.
Cada sala usa quatro indices de vizinhos (esquerda, direita, cima, baixo),
derivados exclusivamente da tabela de pais da arvore. Portas travam apenas durante
o combate da propria sala. O portal central da sala do boss conclui a fase
apos derrota-lo. As plantas das tres fases (7, 9 e 11 salas) ficam em tabelas
constantes em `Fase.cpp`, com posicao na grade, inimigos e baus. `PAIS_FASE_n`
e a fonte unica das arestas: `construir_conexoes` gera os dois sentidos de cada
ligacao pai-filho. O construtor rejeita pais invalidos/ciclos, salas sobrepostas,
pares nao adjacentes e portas ocupadas por duas conexoes. Nao ha ligacoes cruzadas.
As tres fases possuem respectivamente 6, 8 e 10 arestas para 7, 9 e 11 salas.

`Arvore<size_t>` guarda os indices revelados, com raiz no lobby. Ao liberar a sala
atual, `preparar_adjacentes` insere apenas seus filhos canonicos e prepara seu
conteudo sem marcar visita. A navegacao consulta filho ou pai; conhecer um destino
de outro ramo nao autoriza uma passagem. Reiniciar limpa a arvore e recria a raiz.
A chegada preserva a posicao no mundo ao mudar as coordenadas locais da sala. `Campanha` avanca e `Jogo` grava
o checkpoint. Reiniciar no menu de pausa ou morte recriam apenas a fase atual e o jogador.
`Campanha` guarda uma copia de `FilaArmas` do inicio da fase: avancar copia o
inventario atual para a proxima fase, e reiniciar devolve essa copia ao jogador.
O checkpoint em disco guarda essa copia; `Jogo` converte `FilaArmas` para
`ArmaSalva` (indice de `TipoArma` e municao), mantendo a persistencia sem
dependencia das entidades.
A tecla R nao reinicia mais. O reinicio pelo menu retoma a partida.

Para configurar a fase inicial, edite os registros no construtor de `Fase`, em
`src/source/mundo/Fase.cpp`. As dimensoes minimas sao 400x300, dificuldade pelo
menos 1, quantidade de inimigos nao negativa. Os inimigos nascem na
metade direita, afastados da entrada inicial. As classes basico, medio e boss
calculam inicialmente 4/20/100 de vida multiplicados por `1 + 0.2 * (d - 1)`.
O ajuste global conserva 75% do resultado e, em seguida, 90% do restante,
arredondando cada etapa para baixo; na fase 1, portanto, as vidas efetivas sao
2/13/67. A velocidade continua usando
`1 + 0.12 * (d - 1)`.

O jogador caminha pelo piso e pelos corredores liberados; inimigos ficam no piso da sala;
projeteis saindo dele sao desativados, exceto ricochetes e boomerangs. A camera respeita os limites da fase com margem de 12 pixels
para paredes. `Sala` desenha piso, portas e entidades; `interface/Hud` desenha
vida, escudo e energia em uma caixa unica e compacta no canto superior esquerdo
e somente o numero de municao no canto inferior direito, sobre o mundo.
O inventario aparece no canto superior direito; `IconeArma` desenha sprites
de bitmap embutidos, sem dependencia grafica em `Armas` ou nos TADs. `Fase` nao desenha interface. Novos objetos de sala devem ter seu ciclo
de vida associado a `Sala`, evitando listas globais compartilhadas entre salas.

## Campanha, menus e persistencia

`interface/Minimapa` consulta salas e vizinhos por acesso somente leitura em `Fase`.
O percurso em pre-ordem determina quais nos estao revelados, nao a posicao deles.
`get_origemSala` fornece uma copia das mesmas coordenadas usadas pelo cenario.
O minimapa aplica escalas positivas a essas coordenadas, preservando esquerda/direita,
cima/baixo e alinhamentos. Os limites da fase inteira fixam o enquadramento, sem
mover nos quando outros sao revelados. Somente as arestas pai-filho da arvore
revelada sao desenhadas, em segmentos retos entre as portas correspondentes.
Os rotulos ESQ/DIR/CIMA/BAIXO consultam os vizinhos reais e indicam qual porta usar
na sala pai para chegar ao filho; o mesmo acesso define as pontas dos segmentos.
A interface nao mantem outra tabela de conexoes nem ordena irmaos por indice.
`Jogo` controla a sobreposicao com M, bloqueia a simulacao e as acoes enquanto ela
esta aberta e restaura a partida ou pausa ao fechar com M/Esc. O minimapa mostra
somente os nos revelados, seus estados e um marcador da sala atual. Desenhar o
mapa nao insere nos nem modifica a progressao.

`Campanha` conhece o numero de fases concluidas e a fase atual. Ela controla a
passagem entre tres fases e reinicializa o jogador em cada etapa. `Fase(numero)`
aplica a dificuldade e quantidade de inimigos daquele estagio, a partir
da planta ramificada daquele estagio (7, 9 ou 11 salas). Nao e preciso armazenar todas as fases simultaneamente.

`Jogo` coordena os estados `CARREGANDO`, `INICIAL`, `JOGANDO`, `PAUSADO`, `VITORIA`,
`RESUMO_FASE`, `GAME_OVER` e `SAINDO`. `TelaCarregamento` combina o progresso real
da sintese de audio com uma duracao minima. `Menu` tem as telas principal, jogar,
slots, manual, configuracoes, sobre e pausa, com teclado e mouse; a navegacao entre
telas informativas e interna, e o menu so pede ao `Jogo` acoes que envolvem saves,
partida ou preferencias. Ele nao le arquivos nem altera diretamente o progresso.
`ArquivoConfiguracoes` grava volumes, tela cheia e hitboxes em texto `chave=valor`
com o mesmo esquema de arquivo temporario dos saves; valores invalidos voltam ao padrao.

`audio/` separa sintese e reproducao: `Sintetizador` e `Repertorio` nao dependem de
SDL e transformam listas de `Nota` em amostras; `Som` e a unica parte ligada ao SDL,
com vozes fixas, transicao cruzada entre trilhas e limitador suave. As entidades
apenas pedem `Som::tocar(efeito)` nos eventos (dano, morte, bau, sala liberada).

`ArquivoProgresso`, em `persistencia/`, recebe um caminho e trata somente leitura,
validacao e escrita. O esquema JSON v2 tem `versao`, `fases_concluidas` (0 a 3) e `armas`
(ate tres objetos `{tipo, municao}`); o v1, sem `armas`, ainda e aceito.
O leitor e especifico para esses esquemas, sem biblioteca externa; um esquema mais
complexo deve motivar a revisao dessa escolha. O arquivo temporario e renomeado
somente apos fechar a escrita com sucesso, preservando o save anterior em erros.
Isso protege contra escritas incompletas, mas nao promete durabilidade contra
queda de energia, pois nao utiliza fsync.

A camada de aplicacao resolve `saves/progresso.json` ao lado do executavel e
salva ao iniciar um jogo novo e quando o contador de fases concluidas muda.
O checkpoint nao armazena entidades, salas intermediarias ou posicoes. Carregar
reconstroi o inicio da proxima fase; concluir tres fases registra a vitoria.
Falhas sao exibidas na pausa e nos menus e podem ser repetidas por F5, sem
interromper o jogo. Durante a partida os avisos permanecem ocultos.

## Geometria das entidades

`DimensoesEntidade` separa o tamanho de colisao do quadro visual por composicao,
sem misturar dimensoes com TADs ou armazenamento. `Jogador` e `Inimigo` guardam
seus perfis; suas posicoes sao relativas a hitbox. `get_retanguloVisual()` calcula
a area centralizada para desenho, e `get_retangulo()` continua sendo a colisao.
Detalhes e medidas para artistas estao no [guia de sprites](sprites.md).

## Recursos graficos

`Sprite` possui uma textura SDL, decodifica PNG RGBA via libpng e desenha a imagem
centralizada dentro de um quadro, preservando proporcoes e transparencia.
`Jogo` possui o sprite do Tux e o fornece ao desenho de `Jogador`. A textura
nao faz parte das entidades copiadas/reiniciadas, e e liberada antes do renderizador.
Os bytes dos PNGs vem de `graficos/AssetsEmbutidos.h`: o Makefile gera, a partir de
`assets/`, um `.cpp` com esses bytes, e `Sprite` os le da memoria
(`png_image_begin_read_from_memory`). Assim o executavel nao depende de arquivos externos.

## Tipos de armas e objetos de sala

`Armas` guarda `TipoArma`, municao, dano, quantidade de projeteis, cadencia,
afinidade e classe de alcance; continua independente de SDL. Os nove tipos
formam uma matriz de tres afinidades por tres alcances. `Combate` implementa
os tres setores corpo a corpo (Malwarebytes, Fork e Mouse Macintosh), os disparos
medios (ClamAV, Apache e Zapper) e os longos (Avira, Wireshark e CRT).
Corpo a corpo e Avira aplicam dano imediato e armazenam rastros visuais breves.
Cada golpe acerta um alvo apenas uma vez; cobertura interrompe os raios do setor.
Projeteis usam passos de no maximo 1/240 s para evitar saltar sobre hitboxes.
Apache registra ate dois indices atingidos e CRT rebate quatro vezes; o alcance
restante limita o trajeto total. Inimigos mortos permanecem inativos na lista ate reiniciar a sala,
preservando esses indices. Apenas projeteis inativos e rastros vencidos sao removidos.

`Sala` possui `ListaSequencial<ArmaChao>` com armas coletaveis e descartadas.
A busca da arma mais proxima custa O(n); as coletas ficam marcadas para preservar
o estado local. Revisitar nao recria itens. Todas as fases usam baus aleatorios. Nenhum TAD conhece essas regras.

## Obstaculos de cobertura

`Obstaculo` (em `entidades/`) guarda tipo, retangulo e vida, calcula o menor
deslocamento para separar um corpo e desenha caixote ou barreira de firewall.
Cada `Combate` possui uma `ListaSequencial<Obstaculo>`; como os inimigos, pecas
quebradas ficam inativas na lista ate a fase ser recriada. `Combate` aplica as
regras: tiros do jogador causam dano, tiros inimigos sao absorvidos, e corpos sao
empurrados pelo eixo de menor penetracao (busca linear, O(n) por entidade).
`Sala::preparar` escolhe um dos modelos autorais de cobertura indicados na planta e
expande cada barreira em blocos independentes de 24x24, que quebram um a um.

## Inimigos, recompensas e defesa

`TipoInimigo` distingue basico, medio e boss. O ajuste de facilidade aplica uma
reducao de 25% e depois outra de 10% sobre a vida calculada e o dano hostil: na
fase 1, as vidas efetivas sao 2/13/67 e os tiros causam 5/8/11. `Afinidade` distingue
vermelho (malware), azul (software fechado) e verde (hardware com IA), sem
alterar a classe de forca. Arma e alvo com a mesma afinidade causam 2x o
dano-base; combinacoes diferentes e o soco neutro causam 1x. `Combate` guarda
tiros hostis separados dos projeteis do jogador e verifica colisoes em passos
de ate 1/240 s. `Inimigo` decide o proprio movimento (distancia preferida, giro
e contorno) a partir da linha de visao que `Combate` calcula por quadro; `Combate`
decide os padroes de tiro, a mira antecipada e as ondas de reforco, mantidas
como contadores e uma `ListaSequencial` de avisos de surgimento. Inimigos novos
sao inseridos no fim da lista, preservando os indices usados pelo bumerangue.
`Jogador` aplica absorcao pelo escudo, protecao de 0,5 s e regeneracao de
10 pontos/s apos 3 s sem dano.

Cada morte produz exatamente um coracao, inclusive ataques instantaneos.
`Combate` controla coleta e expiracao; `Fase` envelhece coletas inativas.
`Sala` possui baus e armas no chao; abrir um bau gera primeiro uma arma da
afinidade eficaz contra o boss da fase e depois sorteia uma entre os nove tipos,
com reposicao. B/E possuem um/dois baus. Nenhum TAD depende
dessas regras de jogo; armazenamento permanece encapsulado.

A pausa apresenta somente o titulo PAUSADO, fase/nome da sala e as opcoes
Continuar, Manual, Reiniciar, Menu inicial e Sair; o manual aberto pela pausa volta a ela.
A quantidade de opcoes usada para desenho, teclado e mouse depende do modo.

### Corredores e continuidade visual

`Fase` armazena as origens das salas e desenha primeiro os corredores, depois as
salas, com cameras traduzidas. A camera principal segue o jogador nos limites da
fase. Cada conexao tem 240 pixels de comprimento e 96 de largura; nao cria uma
nova sala no TAD. `Sala::limitar_jogador` restringe o movimento a uniao do piso e
das passagens liberadas. Combate e inimigos continuam restritos ao piso da sala.
`graficos/Ambiente` concentra pisos, paredes, grades e portais geometricos.

`Sala::preparar` cria as entidades uma unica vez, sem marcar visita ou simular
combate. `Fase::preparar_adjacentes` chama essa preparacao quando a sala atual
esta liberada, incluindo o lobby inicial. `Sala::ativar` inicia a visita e o
combate na chegada, preservando as entidades ja visiveis. A colisao considera
retangulos sobrepostos de posicoes validas da hitbox na juncao sala/corredor,
permitindo retorno e conservando o ultimo passo na mudanca de coordenadas.
