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
  |-- Campanha (cinco fases; mantem a fase atual)
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
A fase 1 usa quatro indices de vizinhos por sala (esquerda, direita, cima, baixo),
formando um pequeno grafo sobre a lista existente. Portas travam apenas durante
o combate da propria sala; E–F nao tem trava adicional. O portal central de F
conclui a fase apos derrotar o boss.
As fases 2–5 mantem navegacao linear e portal central na ultima sala liberada.
A chegada preserva a posicao no mundo ao mudar as coordenadas locais da sala. `Campanha` avanca e `Jogo` grava
o checkpoint. Reiniciar no menu de pausa ou morte recriam apenas a fase atual e o jogador.
A tecla R nao reinicia mais. O reinicio pelo menu retoma a partida.

Para configurar a fase inicial, edite os registros no construtor de `Fase`, em
`src/source/mundo/Fase.cpp`. As dimensoes minimas sao 400x300, dificuldade pelo
menos 1, quantidade de inimigos nao negativa. Os inimigos nascem na
metade direita, afastados da entrada inicial. Dificuldade `d` aplica vida
`VIDA_INIMIGO + d - 1` e velocidade `VELOCIDADE_INIMIGO * (1 + 0.12 * (d - 1))`.

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
Um percurso das conexoes posiciona os blocos conforme as quatro direcoes, sem copiar
o mapa autoral para a interface. Fases lineares fornecem vizinhos esquerdo/direito.
`Jogo` controla a sobreposicao com M, bloqueia a simulacao e as acoes enquanto ela
esta aberta e restaura a partida ou pausa ao fechar com M/Esc. O minimapa mostra
todas as salas, estados e um marcador da posicao do jogador na sala atual.

`Campanha` conhece o numero de fases concluidas e a fase atual. Ela controla a
passagem entre cinco fases e reinicializa o jogador em cada etapa. `Fase(numero)`
aplica a dificuldade e quantidade de inimigos daquele estagio, mantendo a
estrutura linear de tres salas nas fases 2–5 e sete salas ramificadas na fase 1. Nao e preciso armazenar todas as fases simultaneamente.

`Jogo` coordena os estados `INICIAL`, `JOGANDO`, `PAUSADO`, `VITORIA` e `SAINDO`.
`Menu` oferece modos inicial e pausa, com teclado e mouse. O menu solicita acoes;
ele nao le arquivos nem altera diretamente o progresso.

`ArquivoProgresso`, em `persistencia/`, recebe um caminho e trata somente leitura,
validacao e escrita. O esquema JSON v1 tem `versao` e `fases_concluidas` (0 a 5).
O leitor e especifico para esse esquema, sem biblioteca externa; um esquema mais
complexo deve motivar a revisao dessa escolha. O arquivo temporario e renomeado
somente apos fechar a escrita com sucesso, preservando o save anterior em erros.
Isso protege contra escritas incompletas, mas nao promete durabilidade contra
queda de energia, pois nao utiliza fsync.

A camada de aplicacao resolve `saves/progresso.json` ao lado do executavel e
salva ao iniciar um jogo novo e quando o contador de fases concluidas muda.
O checkpoint nao armazena entidades, salas intermediarias ou posicoes. Carregar
reconstroi o inicio da proxima fase; concluir cinco fases registra a vitoria.
Falhas sao exibidas na pausa e nos menus e podem ser repetidas por F5, sem
interromper o jogo. Durante a partida os avisos permanecem ocultos.

## Geometria das entidades

`DimensoesEntidade` separa o tamanho de colisao do quadro visual por composicao,
sem misturar dimensoes com TADs ou armazenamento. `Jogador` e `Inimigo` guardam
seus perfis; suas posicoes sao relativas a hitbox. `get_retanguloVisual()` calcula
a area centralizada para desenho, e `get_retangulo()` continua sendo a colisao.
Detalhes e medidas para artistas estao no [guia de sprites](sprites.md).

## Recursos graficos

`Sprite` possui uma textura SDL, carrega PNG RGBA via libpng e desenha a imagem
centralizada dentro de um quadro, preservando proporcoes e transparencia.
`Jogo` possui o sprite do Tux e o fornece ao desenho de `Jogador`. A textura
nao faz parte das entidades copiadas/reiniciadas, e e liberada antes do renderizador.
O arquivo fica em `assets/sprites/personagens/tux.png`, relativo ao executavel.

## Tipos de armas e objetos de sala

`Armas` guarda `TipoArma`, municao, dano, quantidade de projeteis e cadencia;
continua independente de SDL. `Combate` implementa shotgun, laser, tiro duplo,
ricochete, espada, arco e boomerang. Laser e espada aplicam dano imediato e
armazenam rastros visuais breves. Projeteis usam passos de no maximo 1/240 s
para evitar saltar sobre hitboxes. Boomerangs registram indices atingidos por
trajeto; inimigos mortos permanecem inativos na lista ate reiniciar a sala,
preservando esses indices. Apenas projeteis inativos e rastros vencidos sao removidos.

`Sala` possui `ListaSequencial<ArmaChao>` com armas coletaveis e descartadas.
A busca da arma mais proxima custa O(n); as coletas ficam marcadas para preservar
o estado local. Revisitar nao recria itens. A fase 1 usa baus aleatorios; as
fases 2–5 mantem a distribuicao inicial deterministica. Nenhum TAD conhece essas regras.

## Inimigos, recompensas e defesa

`TipoInimigo` distingue virus, bruta-montes e boss com vida base 2/12/65,
perfis de colisao proprios e tiros simples de dano 8/12/18. `Combate` guarda
tiros hostis separados dos projeteis do jogador e verifica colisoes em passos
de ate 1/240 s. Inimigos param a 180 pixels e atiram na direcao do jogador.
`Jogador` aplica absorcao pelo escudo, protecao de 0,5 s e regeneracao de
10 pontos/s apos 3 s sem dano.

Cada morte produz exatamente um coracao, inclusive ataques instantaneos.
`Combate` controla coleta e expiracao; `Fase` envelhece coletas inativas.
`Sala` possui baus e armas no chao; abrir um bau sorteia duas armas uniformemente
entre os oito tipos, com reposicao. B/E possuem um/dois baus. Nenhum TAD depende
dessas regras de jogo; armazenamento permanece encapsulado.

A pausa apresenta somente o titulo PAUSADO, fase/nome da sala e quatro opcoes:
Continuar, Reiniciar, Menu inicial e Sair. O menu inicial mantem tres opcoes.
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
