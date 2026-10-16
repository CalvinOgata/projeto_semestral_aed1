# Medidas para sprites e hitboxes

Todas as medidas abaixo sao em pixels logicos, na resolucao de referencia
960x540. O redimensionamento da janela escala o jogo inteiro. Um quadro de
48x48 e desenhado com 48x48 no mundo; nao depende do tamanho fisico do monitor.

## Padrao de producao

| Categoria | Hitbox (largura x altura) | Quadro PNG ideal | Inicio da hitbox dentro do quadro (x, y) | Estado |
| --- | --- | --- | --- | --- |
| Herois (Tux, Android, etc.) | 32x32 | 48x48 | (8, 8) | Perfil usado pelo jogador atual; personagens ainda genericos |
| Inimigos comuns (virus, malware) | 28x28 | 48x48 | (10, 10) | Perfil usado pelos inimigos atuais |
| Guarda-costas | 44x44 | 64x64 | (10, 10) | Bruta-montes em C/D; visual provisorio |
| Bosses | 88x88 | 128x128 | (20, 20) | Boss em F; visual provisorio |

A hitbox e a area solida usada nas colisoes, nao o tamanho minimo do arquivo.
O quadro e o espaco completo reservado para a figura e a animacao, incluindo
pixels transparentes. Use o tamanho ideal da tabela para exportar cada quadro.
Os perfis sao padroes iniciais editaveis, nao limites obrigatorios para todo
personagem futuro. Um personagem especifico pode receber dimensoes proprias.

## Como desenhar e exportar

- Exporte PNG RGBA com fundo transparente, sem fundo branco ou quadriculado gravado.
- Desenhe o corpo principal sobre a hitbox central. Acessorios, orelhas e efeitos
  podem ocupar a margem externa; essa margem visual nao recebe tiros.
- Mantenha a figura centralizada: o centro do quadro coincide com o centro da
  hitbox. O alinhamento atual **nao** e pelos pes. Para o heroi, o centro e (24,24).
- Mantenha todos os quadros e direcoes do mesmo personagem com o mesmo tamanho e
  ponto de alinhamento. Nao recorte automaticamente cada quadro ao conteudo opaco.
- Use a mesma escala e perspectiva de cima com leve visao frontal em todos os
  personagens. Para pixel art, desenhe diretamente na grade final, sem suavizacao.
- Deixe a arma do jogador separada do corpo: o jogo ja gira a arma em direcao ao
  mouse. Nao desenhe barras de vida ou HUD dentro dos sprites.
- Por enquanto, envie quadros individuais ou uma folha regular sem espacamento.
  Exemplo: oito quadros de 48x48 lado a lado formam uma folha de 384x48.
  A ordem das animacoes, direcoes e duracao dos quadros sera definida na integracao.
- Se a ferramenta gerar imagens maiores, reduza cada quadro exatamente ao tamanho
  escolhido antes da entrega, mantendo proporcoes e transparencia. A resolucao
  de geracao nao muda a hitbox do jogo.

## Convencao tecnica

`src/include/config/DimensoesEntidade.h` centraliza os quatro perfis. Cada perfil
possui largura/altura da hitbox e largura/altura do quadro visual. O quadro deve
conter a hitbox, e todas as dimensoes precisam ser positivas.

`Jogador` e `Inimigo` recebem esse perfil no construtor. `x` e `y` continuam sendo
o canto superior esquerdo da hitbox. `get_retangulo()` retorna a colisao;
`get_retanguloVisual()` retorna o destino da imagem, ambos no espaco do mundo.
A camera converte o retangulo visual para a tela ao desenhar.

Exemplo para preparar um guarda-costas (perfil usado pelos bruta-montes):

```cpp
Inimigo guarda(100.0f, 120.0f, 12, 80.0f, DIMENSOES_GUARDA_COSTAS);
```

Um perfil retangular tambem funciona, por exemplo `{24, 32, 48, 64}`. A separacao
entre inimigos considera largura e altura dos dois corpos. Tiros, paredes e
portas usam a hitbox; mira e disparos usam seu centro. A barra de vida fica acima
do retangulo visual. Nas bordas da sala a imagem pode ultrapassar a parede pela
margem visual, pois quem limita movimento e a hitbox.

Pressione **F2 durante o jogo** para mostrar/esconder o contorno verde das
hitboxes. O retangulo colorido maior representa o quadro visual futuro.

O jogador usa agora `assets/sprites/personagens/tux.png`, carregado por
`graficos/Sprite` via libpng. O PNG original mede 48x61 e foi preservado: o desenho
e encaixado proporcionalmente no quadro 48x48 (aproximadamente 37,8x48),
centralizado e com transparencia. A hitbox permanece 32x32. Para ocupar todo o
quadro com outra arte, exporte diretamente no formato ideal da tabela.

`Jogo` possui a textura, carrega uma vez e libera antes de destruir o renderizador.
Reiniciar o jogador ou trocar de fase nao recarrega a imagem. A arma permanece
separada e gira com a mira. Inimigos ainda usam retangulos; animacoes e folhas de
sprites ainda nao foram integradas. Os reinicios ainda criam o heroi padrao;
selecao de personagem fica para a mecanica de selecao futura.
