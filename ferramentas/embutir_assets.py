#!/usr/bin/env python3
"""Gera um .cpp com os bytes dos assets, para embuti-los no executavel.

    uso: embutir_assets.py <saida.cpp> NOME=caminho [NOME=caminho ...]

Cada NOME deve estar declarado em src/include/graficos/AssetsEmbutidos.h.
O Makefile chama este script; o arquivo gerado fica na pasta de build.
"""

import sys
from pathlib import Path


def main() -> int:
    if len(sys.argv) < 3:
        print(__doc__.strip(), file=sys.stderr)
        return 1

    linhas = [
        "// Gerado por ferramentas/embutir_assets.py; nao editar.",
        '#include "graficos/AssetsEmbutidos.h"',
        "",
    ]
    definicoes = []
    for indice, par in enumerate(sys.argv[2:]):
        nome, separador, caminho = par.partition("=")
        if not separador or not nome or not caminho:
            print(f"argumento invalido: {par}", file=sys.stderr)
            return 1
        dados = Path(caminho).read_bytes()
        if not dados:
            print(f"asset vazio: {caminho}", file=sys.stderr)
            return 1
        linhas.append(f"// {caminho}")
        linhas.append(f"static const unsigned char DADOS_{indice}[] = {{")
        for inicio in range(0, len(dados), 24):
            trecho = dados[inicio : inicio + 24]
            linhas.append("    " + ",".join(str(byte) for byte in trecho) + ",")
        linhas.append("};")
        linhas.append("")
        definicoes.append(
            f"const AssetEmbutido AssetsEmbutidos::{nome} = "
            f"{{DADOS_{indice}, sizeof(DADOS_{indice})}};"
        )

    saida = Path(sys.argv[1])
    saida.parent.mkdir(parents=True, exist_ok=True)
    saida.write_text("\n".join(linhas + definicoes) + "\n", encoding="ascii")
    return 0


if __name__ == "__main__":
    sys.exit(main())
