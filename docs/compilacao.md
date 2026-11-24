# Compilacao, distribuicao e publicacao

Guia completo de compilacao. O [README](../README.md) traz apenas o resumo.

## Compilar e rodar no desktop

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

## Executaveis para distribuir (Linux e Windows)

Precisa apenas do Docker; cada build roda num container com as ferramentas certas:

```sh
make dist-linux     # gera dist/jogo      (Linux x86-64, glibc 2.35+)
make dist-windows   # gera dist/jogo.exe  (Windows 64 bits, compilado com MinGW-w64)
make dist           # os dois
```

- `dist/jogo` e compilado no Ubuntu 22.04 e roda nele e em distros mais novas
  (Debian 12+, Fedora 36+, Arch...). So depende da libc; X11, Wayland, OpenGL e
  audio sao abertos pelo SDL a partir do sistema, como em qualquer jogo Linux.
- `dist/jogo.exe` so usa DLLs que acompanham o proprio Windows. O icone (Tux) e as
  informacoes de versao vem de `ferramentas/windows/jogo.rc`, compilado pelo
  `windres` do MinGW. O icone `assets/icone/jogo.ico` (256 a 16 px) foi gerado do
  primeiro quadro do Tux por `python3 ferramentas/gerar_icone.py` (requer Pillow);
  rode de novo se a arte do Tux mudar.
- Basta enviar o arquivo. Os saves sao criados em `saves/` ao lado dele.

Sem Docker, `make ALVO_SO=windows` compila o `.exe` localmente se o
`x86_64-w64-mingw32-g++` estiver instalado.

## Downloads automaticos (GitHub Actions)

O workflow `.github/workflows/executaveis.yml` compila Linux, Windows e web no GitHub:

- **Push no `main`:** atualiza a pre-release `desenvolvimento` (aba *Releases*) com
  `MCGA-windows-x86_64.exe`, `MCGA-linux-x86_64.tar.gz` e `MCGA-web.zip` do ultimo commit.
- **Tag `v*`:** cria uma release oficial com os mesmos arquivos:
  `git tag v1.0 && git push origin v1.0`.
- **Pull requests:** so compilam (os arquivos ficam em *Actions → execucao → Artifacts*).

As bibliotecas de `externo/` e os ports do Emscripten ficam em cache, entao so a
primeira execucao (ou uma troca de versao) recompila o SDL2.

## Deploy automatico da versao web (nginx)

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

## Versao web (opcional)

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

## Compilacao local alternativa (Linux)

Se CMake nao estiver instalado, mas SDL2 e libpng de desenvolvimento estiverem
disponiveis via pkg-config, use `make USAR_LIBS_SISTEMA=1` e depois `./jogo`.
Esta opcao nao baixa dependencias e gera um binario ligado as bibliotecas locais.
O modo padrao continua usando as dependencias empacotadas em `externo/`.
