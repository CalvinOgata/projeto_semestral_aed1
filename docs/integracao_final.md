# Arte e combate integrados em 19/11/2026

Os PNG aprovados foram reaproveitados, sem gerar novas artes. As faixas de Tux e
dos nove inimigos possuem tres quadros iguais na horizontal. O ciclo 0,1,0,2 usa
0,16 s por pose para Tux/comuns, 0,22 s para brutos e 0,28 s para chefes. Tux
fica parado no quadro zero. Pausar congela as animacoes da simulacao.

`Sprite` carrega a faixa uma vez, recorta um quadro e o desenha com filtro nearest.
`CatalogoSprites` centraliza nove inimigos, nove armas e 22 pisos; `Jogo` controla
carregamento/liberacao antes de destruir o renderizador. Todos os PNG sao
embutidos pelo Makefile; o executavel nao precisa abrir assets externos ao rodar.

Escalas visuais: Tux 64x64; comum 48x48; bruto 128x128; chefe 256x256. Hitboxes
permanecem 32x32, 28x28, 44x44 e 88x88, respectivamente. F2 permite conferi-las.
As salas medem 936x516; a arte maior nao altera portas, obstaculos nem a arvore.

Pisos: Compiladores em azul-cinza; Rede em verde escuro; Kernel em violeta
escuro. Os motivos ficam nas bordas para separar personagens do plano de fundo.
Cada sala de combate/lobby/boss tem seu piso, e todas as salas de bau compartilham
`bau_compartilhado.png`. Os nomes `faseN_XX` enumeram salas sem bau em ordem.

Para substituir a arte, mantenha dimensoes, nomes, transparencia e tres quadros
por personagem. Recompile depois. Armas usam 64x64 e pisos 936x516. Os PNG
anteriores foram preservados; a imagem especifica de game over continua sendo
a versao anterior, enquanto menu, HUD, vitoria e jogador usam o novo Tux.

Os nove IDs numericos de armas foram preservados para compatibilidade com saves.
O arsenal e as medidas exatas constam da tabela atualizada do README. Corpo a
corpo usa setores finitos a frente do Tux, sem municao; medio e longo usam
alcances e velocidades diferentes. Um inimigo atingido por varios raios do mesmo
golpe recebe dano uma unica vez. Avira e instantaneo, Apache perfura dois alvos
distintos e CRT ricocheteia ate quatro vezes, consumindo alcance em todo o trajeto.
Afinidade igual dobra dano; soco continua neutro, dano 1, apenas com fila vazia.

Verificacao local: compilacao Linux com bibliotecas do sistema; testes de fila,
alcances das nove armas, multiplicadores, bloqueio por cobertura, golpe frontal,
soco, perfuracao, ricochete e revelacao das arvores das tres fases. Renderizacao
controlada de menu, combate, chefes, minimapas, resumo e game over pela SDL.
Nao equivale a uma campanha completa jogada por pessoa nem a teste Windows/Web.
