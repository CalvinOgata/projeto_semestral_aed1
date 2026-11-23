#!/usr/bin/env python3
"""Gera o icone do executavel Windows a partir do primeiro quadro do Tux.

uso: python3 ferramentas/gerar_icone.py [sprite.png] [saida.ico]

Nao roda na compilacao: o .ico gerado fica versionado em assets/icone/, e so
precisa ser refeito se a arte do Tux mudar. Requer Pillow (pip install pillow).
"""
import sys

from PIL import Image

ENTRADA = sys.argv[1] if len(sys.argv) > 1 else "assets/sprites/final/00_tux.png"
SAIDA = sys.argv[2] if len(sys.argv) > 2 else "assets/icone/jogo.ico"

# A faixa tem tres quadros quadrados lado a lado; o primeiro e o Tux parado.
faixa = Image.open(ENTRADA).convert("RGBA")
lado = faixa.height
quadro = faixa.crop((0, 0, lado, lado))

imagens = []
for tamanho in (256, 128, 64, 48, 32, 16):
    # Ampliar por fator inteiro mantem os pixels nitidos; reduzir com filtro
    # suave evita que detalhes de 1 pixel sumam nos tamanhos pequenos.
    filtro = Image.NEAREST if tamanho >= lado else Image.LANCZOS
    imagens.append(quadro.resize((tamanho, tamanho), filtro))

imagens[0].save(SAIDA, format="ICO", append_images=imagens[1:],
                sizes=[im.size for im in imagens])
print(f"{SAIDA}: {', '.join(str(im.width) for im in imagens)} px")
