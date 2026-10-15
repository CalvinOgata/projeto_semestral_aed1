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
  |-- Combate (disparos, perseguicao, colisoes e limpeza)
  |     |-- ListaSequencial<Inimigo>
  |     `-- ListaSequencial<Projetil>
  |-- Camera
  `-- Menu
```

`Jogo` controla a cadencia dos tiros e chama `Combate`. Este consulta a arma
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
| `ListaSequencial<T>` | Armazenamento dinamico via `std::vector` | Inimigos e projeteis de `Combate` | Acesso O(1), inserir O(1) amortizado, remover por criterio O(n) |

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
