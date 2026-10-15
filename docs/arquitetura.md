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
na frente da fila, consome municao e insere o projetil. Depois atualiza os
projeteis e inimigos, resolve colisoes e remove os inativos. A remocao acontece
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
- `FilaArmas` limita a capacidade a cinco e oferece apenas operacoes de
  inventario FIFO. O tiro altera a municao da arma da frente, sem remove-la.
  Troca automatica de arma e coleta de armas ainda nao sao mecanicas implementadas.
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
Apenas a sala ativa recebe atualizacoes, interrompidas durante a pausa.

`Fase` guarda a sequencia em `ListaSequencial<Sala>` e acessa a atual por indice.
Avancar/voltar nao remove salas da lista; isso preserva seu estado. A navegacao
ocorre automaticamente ao tocar o retangulo da porta, inclusive suas bordas,
e exige liberacao da sala atual. `Jogo` verifica a transicao apos atualizar o
combate. A posicao de chegada fica afastada da porta para evitar retorno imediato. A porta final marca a fase
como concluida; `Campanha` avanca para a seguinte e `Jogo` grava o checkpoint.
`R` recria somente a fase atual e o jogador. Essa lista sequencial serve a uma
fase linear; ramificacoes futuras poderao justificar um grafo de conexoes.

Para configurar a fase inicial, edite os registros no construtor de `Fase`, em
`src/source/mundo/Fase.cpp`. As dimensoes minimas sao 400x300, dificuldade pelo
menos 1, quantidade de inimigos nao negativa. Os inimigos nascem na
metade direita, afastados da entrada inicial. Dificuldade `d` aplica vida
`VIDA_INIMIGO + d - 1` e velocidade `VELOCIDADE_INIMIGO * (1 + 0.12 * (d - 1))`.

O piso define a area caminhavel. Jogador e inimigos sao limitados ao piso;
projeteis saindo dele sao desativados. A camera respeita os limites com margem de 12 pixels
para paredes. `Sala` desenha piso, portas e entidades; `interface/Hud` desenha
vida, escudo e energia em uma caixa unica e compacta no canto superior esquerdo
e somente o numero de municao no canto inferior direito, sobre o mundo. `Fase` nao desenha interface. Novos objetos de sala devem ter seu ciclo
de vida associado a `Sala`, evitando listas globais compartilhadas entre salas.

## Campanha, menus e persistencia

`Campanha` conhece o numero de fases concluidas e a fase atual. Ela controla a
passagem entre cinco fases e reinicializa o jogador em cada etapa. `Fase(numero)`
aplica a dificuldade e quantidade de inimigos daquele estagio, mantendo a
estrutura de tres salas. Nao e preciso armazenar todas as fases simultaneamente.

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
