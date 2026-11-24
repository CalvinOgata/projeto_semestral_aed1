# Project context and working instructions

## Final integration, 2026-11-19 (current behavior)

The approved art is now embedded from `assets/sprites/final` and
`assets/sprites/pisos`. Character strips contain three equal horizontal frames;
the loop is 0,1,0,2. Tux uses a blue cap, a 64px visual frame and the existing
32px hitbox. Common/brute/boss visual frames are 48/128/256px; hitboxes remain
28/44/88px. `CatalogoSprites` owns shared enemy, weapon and floor textures.

The current arsenal replaces the older names below without changing save IDs:
0 Zapper (green/medium), 1 ClamAV (red/medium), 2 Avira (red/long),
3 Fork (blue/melee), 4 CRT (green/long), 5 Malwarebytes (red/melee),
6 Apache feathers (blue/medium), 7 Macintosh mouse whip (green/melee),
8 Wireshark (blue/long). Melee weapons have infinite ammunition and limited
forward sectors. Avira is hitscan; Apache pierces two distinct enemies; CRT
bounces four times. All traveling shots have explicit distance limits.
Matching affinity still doubles damage; empty-hand punch remains neutral.

Enemies by red/blue/green affinity: ILOVEYOU/Mirai/WannaCry,
EULA/DRM/Windows Update, NPU/GPU H100/hyperscale datacenter. The physical stage
trees, save formats and existing synthesized SDL audio were preserved.
`make USAR_LIBS_SISTEMA=1` is an offline Linux alternative using installed
SDL2/libpng development packages when the bundled static build cannot run.
The default vendored build is unchanged. These facts supersede older roster,
sprite, stamina and weapon notes below; stamina/energy no longer exists in code or HUD.
On 2026-11-23 README, docs/arquitetura.md (now documenting `Arvore`) and docs/sprites.md were
rewritten to match this behavior; `make format` was applied to the whole tree.
Later on 2026-11-23 (user request) README.md became a short overview titled "Make Computers
Great Again" (AED1 context, team, game summary, TAD usage, tools, simple run steps); its former
detailed content moved verbatim to docs/compilacao.md (build/dist/web/CI/deploy), docs/jogo.md
(controls, rules, weapons, HUD, audio, saves) and docs/desenvolvimento.md (naming, module map,
embedded assets). Keep detailed behavior notes in those docs, not in README.

## Purpose and direction

This is a semester project for Algorithms and Data Structures 1 (AED1): a
top-down roguelike written in C++ with SDL2, inspired by **Soul Knight**. Build
on the existing prototype and introduce features incrementally as the team
defines them. Keep solutions understandable for the course and consistent with
the existing code; do not assume course requirements that have not been stated.

The intended game includes bosses, stages, challenge rooms, selectable playable
characters, enemies, weapons, powers, health, and energy/stamina. These are the
design direction, not a claim that all systems are implemented.

## Computing theme

The fictional conflict is between "good code" and "evil systems":

- Playable heroes represent the good-code side. Linux's Tux and the Android
  mascot are proposed characters; the roster and abilities are not finalized.
- Bosses represent the evil-systems side, initially associated with closed-source
  systems. Ideas include Windows, old processor architectures, and social
  networks. Treat these as thematic suggestions, not a finalized roster or a
  technical claim that every example is closed-source.
- Common enemies are viruses and malware, intended to appear throughout all
  stages.
- Bodyguards are stronger, tougher, less numerous enemy units inspired by
  closed-source programs. Specific programs and behaviors remain undecided.

Preserve this computing theme when proposing content. Do not silently turn
brainstorming examples into approved mechanics, characters, or assets.

## Verified prototype baseline

As updated on 2026-10-22:

- The build uses C++17, SDL2, libpng, zlib and a Makefile. The libraries are vendored and
  linked statically (see "Native distribution builds"). Text uses an embedded bitmap font;
  SDL2_ttf is not required.
- The game starts at a loading screen, then a main menu (see "Startup, menus and audio"). Three phases remain (reduced from five by user request), all
  trees of 936x516 rooms with lobby, chest rooms and a final boss. The lobby is at the top.
  Phase 1 has seven rooms: Lobby–A, A–B/C/D, C–E, E–F. Physical exits from A are
  west to B, east to C and south to D; C–E and E–F go south.
  A has 4 viruses; C/D each have 5 viruses and 2 brutes; A/C/D each get one reinforcement wave. B has one chest,
  E two chests; lobby/B/E have no enemies. F has one boss.
- Each room owns its combat and persistent state in ListaSequencial<Sala>.
  Four neighbor indices describe phase 1 connections. Doors lock only while their own room is in active combat; empty/cleared rooms
  stay open, including E–F (the previous A/C/D prerequisite was removed). Defeating F's boss
  creates a central portal; touching it completes the phase. Phase 2 (9 rooms, difficulty 2):
  Lobby–A, A–B/C/D, C–E, D–F, F–G/H (G has 2 chests; H is the boss).
  Phase 3 (11 rooms, difficulty 3) progresses from the top lobby: Lobby–A, A–B/C,
  B–D, C–E/F (F has 1 chest), D–G, E–H (2 chests), G–I, I–J (boss + 3 viruses).
  Layouts are constant tables (`PlantaSala`) in Fase.cpp; per-room counts
  (viruses, brutes, extra waves) are initial balancing choices, raised on 2026-10-22.
- Room navigation correction (2026-11-12): PAIS_FASE_n is the single source of
  parent-child edges. construir_conexoes derives reciprocal directional neighbors
  from parents and grid coordinates; no separate authored neighbor table remains.
  It rejects invalid parents/cycles, overlapping rooms, nonadjacent pairs and occupied
  door slots. Only parent/child transitions are accepted, even for already revealed
  nodes. Removed cross-links D–E (phase 1) and E–G (phases 2 and 3). Room positions,
  enemy/chest counts and canonical parents are unchanged. Tests live outside the repo.

## Web build (2026-11-16)

- User request: run the game in the browser and publish it on itch.io (HTML project,
  zip with index.html at its root). Emscripten ports provide SDL2, libpng and zlib.
- `Jogo::passo()` runs one frame; desktop `executar()` loops over it, while web `main`
  hands it to `emscripten_set_main_loop_arg`. Only the web build uses `#ifdef
  __EMSCRIPTEN__`, confined to `main.cpp` and `nucleo/Plataforma`.
- Web saves: `Plataforma` mounts IDBFS at `/saves` (SDL_GetBasePath is `/`), waits for
  the initial sync before `Jogo::inicializar`, and syncs after each save/delete.
- `Plataforma::PERMITE_SAIR` hides the last menu option (SAIR) on the web.
- `web/shell.html` is a minimal page that scales the canvas to the largest 16:9 box
  (SDL then matches the backbuffer to the CSS size) and shows abort errors instead of
  a stuck loading message. `-sSINGLE_FILE=1` produces one
  self-contained `index.html` because the user opens it directly via file://,
  where browsers block fetching separate .wasm/.data files. Since 2026-11-17 the sprites come
  from the generated embedded-asset source (no `--embed-file`), so it shrank from ~6 to ~3.3 MB. SDL already prevents browser defaults for F-keys, so F5 does not reload.
- Browser checks live in the agent folder's `web-teste/` (e.g. `~/.codex/web-teste/`, Playwright
  scripts `pw_*.mjs` using system Chromium). If emsdk is not activated, `make web` works with
  `EM_CONFIG=<agent folder>/emsdk/emconfig.py PATH=$HOME/emsdk/upstream/emscripten:$PATH`.

## Native distribution builds (2026-11-17)

- Git remotes (2026-11-17): `origin` is the user's fork jsantos43/MCGA, where development,
  Actions and Releases happen; `upstream` is the team repo CalvinOgata/projeto_semestral_aed1,
  updated by merges/PRs from the fork.

- User request: single-file executables for Linux and Windows, with libraries and assets
  inside, nothing to install or download; saves stay in `saves/` next to the executable.
- `externo/` holds the pinned upstream tarballs (SDL2 2.32.10, libpng 1.6.59, zlib 1.3.2;
  ~10 MB, verified by `SHA256SUMS`) instead of 80 MB of extracted sources. `externo/compilar.sh
  <linux|windows> <dir>` builds them with CMake as static libraries into
  `build/<target>/externo/instalado` on the first `make`; the game never uses system SDL2/libpng.
  The web build still uses Emscripten ports.
- Sprites are embedded: `ferramentas/embutir_assets.py` turns the Makefile list
  `ASSETS_EMBUTIDOS` (NAME=path) into a generated `.cpp` under the build dir; names are declared
  in `graficos/AssetsEmbutidos.h`, and `Sprite::carregar` decodes them from memory. `assets/` is
  only needed to compile. Native, Windows and web builds share this mechanism.
- Linux: `-static-libstdc++ -static-libgcc`; only libc/libm remain dynamic, and SDL dlopens
  X11/Wayland/GL/audio at runtime. `make dist-linux` builds `dist/jogo` in Docker (Ubuntu 22.04,
  glibc 2.35 baseline). Windows: MinGW-w64 cross-compilation, `-static -mwindows`,
  `-D_USE_MATH_DEFINES` (M_PI); `make dist-windows` builds `dist/jogo.exe` in Docker (Ubuntu
  24.04), importing only Windows system DLLs. Dockerfiles live in `ferramentas/docker/`.
  `make ALVO_SO=windows` builds locally if MinGW is installed.
- `make clean` keeps compiled libraries; `make clean-tudo` removes all of `build/`.
- Verified with a clean Ubuntu 22.04 container (Xvfb) and Wine: menu, embedded Tux, movement and
  save creation work. Test container/scripts: agent folder's `dist-teste/`. The team tested on real
  Windows (user report 2026-11-23). Exe icon (user request 2026-11-23): `ferramentas/windows/jogo.rc`
  (ICON + VERSIONINFO 1.0.0) compiled by MinGW windres into `$(DIR_BUILD)/recursos`, linked only
  for ALVO_SO=windows. `assets/icone/jogo.ico` (PNG entries 256..16) is generated from Tux frame 0
  by `ferramentas/gerar_icone.py` (Pillow, run manually, not part of the build); it is the one
  file in `assets/` that is not embedded through `ASSETS_EMBUTIDOS`.
- GitHub Actions (`.github/workflows/executaveis.yml`, user request 2026-11-17): jobs linux,
  windows (both via `make dist-*`) and web (emsdk pinned to 7.0.0, the developer's local version,
  `make web-zip`). Push to main replaces the prerelease `desenvolvimento`; tags `v*` create an
  official release (`gh release create`, generated notes); PRs/manual runs only upload artifacts.
  Assets: MCGA-linux-x86_64.tar.gz (keeps exec bit), MCGA-windows-x86_64.exe, MCGA-web.zip.
  Caches `build/dist-*/externo` (key: externo/* + Dockerfile) and the Emscripten cache; after a
  cache hit `pronto` is touched because checkout mtimes would otherwise force a rebuild.
- Web deploy (user request 2026-11-17): job `deploy-web` (needs web) runs on push to main only when
  repo variable DEPLOY_HOST exists, so upstream without it skips. Uploads index.html via scp to
  `index.html.novo` then `mv` (atomic), using secret DEPLOY_SSH_KEY and optional secret DEPLOY_KNOWN_HOSTS
  (user chose to run without it on 2026-11-17: the job then ssh-keyscans the host each run and
  emits a warning; no MITM protection) and variables DEPLOY_USER/DEPLOY_DIR/DEPLOY_PORT/DEPLOY_URL; then curls DEPLOY_URL
  and checks for `id="canvas"`. Target: user's nginx at <domain>/meujogo (alias + no-cache + gzip;
  snippet in README). GitHub environment `servidor-web`. Tested against the real server (user report 2026-11-23).

## Startup, menus and audio (2026-11-18)

- User request: a polished loading screen "for the good-game feeling", then a main menu
  with JOGAR, MANUAL, CONFIGURACOES, SOBRE (+ SAIR on desktop); saves/slots only after
  JOGAR (Novo jogo / Continuar / Voltar). Settings may grow later.
- `EstadoJogo::CARREGANDO` shows `interface/TelaCarregamento`: themed boot log, segmented
  bar with Tux walking, spinner, rotating tips. Real progress is the audio synthesis
  (`Som::carregar_etapa`, 8 ms per frame) combined with a 3.6 s minimum; a key/click
  only shortens the wait after it is ready. It fades into the menu background.
- `interface/FundoMenu` draws the procedural synthwave/computing background (binary
  rain, striped sun, server racks with LEDs, perspective grid, CRT scanlines), the
  glitchy logo and glass panels. Menu screens: PRINCIPAL, JOGAR, SLOTS_*, MANUAL
  (tabs CONTROLES / COMO JOGAR), CONFIGURACOES, SOBRE, PAUSA. Navigation between
  informational screens is internal to `Menu`; Esc/VOLTAR from slots returns to JOGAR.
- Settings (`persistencia/ArquivoConfiguracoes`, `saves/configuracoes.txt`, `chave=valor`,
  temp+rename, invalid lines ignored): music/effects volume 0..10 (defaults 7/8),
  fullscreen (desktop only, `Plataforma::PERMITE_TELA_CHEIA`), show hitboxes (F2 also
  toggles and saves it). Saved on every change.
- SOBRE lists technologies and team names (`INTEGRANTES` in Menu.cpp): Calvin da Serra Ogata
  de Paula, Felipe Baratela Pimenta (git author felipebp14-207), Joao Pedro Tomaz dos Santos,
  Pedro Testa Lourenco. Credits Tux to Larry Ewing.
- Audio is fully synthesized, no audio files: `audio/Sintetizador` (SDL-independent notes ->
  samples, 44.1 kHz mono), `audio/Repertorio` (effects catalog `EfeitoSonoro` and three looping
  tracks MENU / FASE / CHEFE composed with a small step sequencer), `audio/Som` (SDL mixer:
  32 voices, max 4 copies per effect plus minimum retrigger intervals, crossfade between
  tracks, ducking during pause/minimap, master gain + tanh limiter). Without an audio
  device the game stays silent. Web uses a 2048-sample buffer (`Plataforma::AMOSTRAS_AUDIO`).
- Music: MENU on menus/summary/victory, FASE during play, CHEFE while the boss room is in
  combat, none on loading/game over. Effects are triggered at the event sites (Combate,
  Sala, Jogador, Inimigo, Obstaculo, Jogo, Menu): per-weapon shots, punch, empty ammo,
  hits, deaths per enemy class, enemy/boss volleys, waves, summons, shield/health damage,
  hearts, door lock, room cleared, boss portal, chest, collect/drop/swap weapon, phase
  complete, player death, game over, continue, victory, cheat and menu navigation.
- Bitmap font gained `[ ] ( ) < > % =`. `Texto::quebrar_linhas` wraps text by pixel width.

## Touch controls (user request 2026-11-20)

- Goal: the web build is playable on phones/tablets. Same code on every platform (no ifdef);
  touch also works on desktop touchscreens.
- `SDL_HINT_TOUCH_MOUSE_EVENTS` is 0: SDL_FINGER* events are the only touch input. During
  JOGANDO without the minimap they go to `interface/ControlesToque`; in every other state
  `Jogo::tratar_eventos` rewrites them into left-button mouse events (which =
  SDL_TOUCH_MOUSEID), so menus, pause, slots and manual tabs work by tap. Resume, game over,
  defeat, minimap and victory also accept left click/tap (resume ignores clicks in its
  first second so a fire tap does not skip it). Victory accelerates while a finger is held.
- `Jogo::modoToque` turns on at the first FINGERDOWN and off on a key press or real mouse
  click/motion. It shows the touch overlay and swaps prompts: manual CONTROLES tab lists touch
  commands (`COMANDOS_TOQUE`, tab hint hidden), "ENTER:" becomes "TOQUE:" in resume, game
  over, defeat and victory, minimap says "TOQUE PARA FECHAR".
- `ControlesToque` tracks up to 10 fingers by id. Left half (not on a button): floating
  joystick (radius 64, dead zone 15%, analog speed via `Jogador::mover_analogico`), resting
  at (130,410). Right half: fire button (858,418) r62, but any free right-half point also
  fires. Buttons PEGAR (glows when `Sala::tem_armaProxima`), TROCAR, SOLTAR, pause (II) and
  MAPA at top center; tapping the hotbar/cycle-arrow area also swaps. Without enemies the aim
  follows the last joystick direction. Fingers are released whenever play is not active.
- The window uses `SDL_WINDOW_ALLOW_HIGHDPI` (fix 2026-11-23): without it the browser canvas
  backbuffer matched CSS pixels (e.g. 640x360 on a DPR-3 phone), so the 960x540 frame was
  downscaled and 1-px bitmap-font strokes vanished. Now the backbuffer uses device pixels.
  Sprite resolution was not the cause.
- `web/shell.html`: no zoom/scroll/selection (`touch-action: none`, viewport
  `user-scalable=no`), portrait overlay "GIRE O APARELHO PARA JOGAR" on coarse pointers,
  first touchend requests fullscreen + `screen.orientation.lock('landscape')` (fails
  silently, e.g. iPhone), and every touch/click/key resumes `Module.SDL2.audioContext`.

## Defeat and victory sequences (user request 2026-11-18)

- Death no longer restarts instantly: `EstadoJogo::DERROTA` shows `interface/TelaDerrota`
  over the frozen scene (white/red flash, glitch bars, fake error windows like TROJAN.DLL,
  a virus swarm from the edges, then "OH NAO! OS CODIGOS MALICIOSOS TOMARAM CONTA DO SEU
  SISTEMA", "NAO DESISTA, GUERREIRO!", a random programming pun and remaining lives). The
  life is removed and saved at death; the phase is recreated only when the animation ends
  (8.5 s, Enter skips after 2.4 s: first press shows the full message, second continues).
  With no lives left it then goes to the existing game-over countdown.
- Campaign completion uses `interface/TelaVitoria` (inspired by Minecraft's ending, with
  original text): celebration (fireworks, confetti, jumping Tux, congratulation about freeing
  the world from closed/malicious code, final score), then a slow upward scroll of two
  alternating "voices" (cyan/gold) with life reflections built from programming puns
  (commits, git reset, warnings, sleep, dependencies...), ending with "SEUS COMMITS NA VIDA
  VALEM A PENA" and credits. The scroll runs at 16 px/s (slowed by user request 2026-11-19);
  held Down/Up arrows add/subtract 110 px/s while the automatic scroll continues (Up from
  the credits returns to the text; no on-screen hint). Hold Space/mouse to speed up, Esc
  skips to credits, Enter in credits returns to the main menu. New track `Musica::FINAL` (slow C major) plays throughout;
  new effects ERRO_SISTEMA, ENCORAJAR, FOGOS.

## Room cover (obstacles)

- User request (2026-10-22): crates/barriers inside rooms for cover; only player attacks
  break them. Interpreted as: enemy shots are absorbed without damage.
- `entidades/Obstaculo`: CAIXOTE 40x40 (VIDA_CAIXOTE 5) and BARREIRA, a 24x24 firewall
  block (VIDA_BLOCO_BARREIRA 3). Authored barriers expand in Sala into a horizontal or
  vertical row of BLOCOS_POR_BARREIRA (5) independent blocks, so hits break one block at
  a time (user request). Procedural SDL drawing; cracks at half health.
- Each room's `Combate` owns `ListaSequencial<Obstaculo>`; broken pieces stay inactive in
  the list and persist on revisits until the phase is recreated.
- Pieces block player and enemy bodies (minimum-axis push-out; initial placement pushes
  enemies by visual frame). Player projectiles damage and stop; ricochet bounces (uses a
  bounce); boomerang turns back on its outbound leg and passes through returning. A projectile
  contact damages only one piece, even on a seam. Laser and sword rays stop at the first
  piece; each attack damages only the nearest piece touched by any of its rays.
- Authored patterns in Sala.cpp (`cobertura` in PlantaSala): 1 quadrants, 2 trench,
  3 fortress (boss rooms; right side kept clear for the boss), 4 maze, 5 depot (chest
  rooms, corners only). Lobbies have none. Patterns avoid the central door cross.

## Phase themes

Each phase combines a computing theme with a game reference. Enemy strength class and color
affinity are independent. Red malware uses ILOVEYOU.EXE / MIRAI SWARM / WANNACRY PRIME;
blue closed software uses EULA-BOT / SENTINELA DRM / COMPILADOR FECHADO; green AI hardware
uses NPU DRONE / TPU SENTINEL / H100 COLOSSUS. Phase 1 introduces single-affinity rooms,
phase 2 starts mixing affinities, and phase 3 uses mostly mixed encounters. Rooms have themed
names; the phase name and
subtitle appear for 3 s (fading) on phase entry/restart; pause and minimap show the name.
- Pause-menu Restart and player death reset the current phase/player without losing completed
  phases. Cleared enemies/chests never respawn on revisits. Projectiles are
  discarded on leaving. Only active combat updates; hearts in inactive rooms
  still expire. Pause freezes simulation. Room arrivals preserve position across the connecting corridor.
- Difficulty (= phase number) increases raw enemy health by 20%, speed by 12%, fire rate by
  12% and hostile shot speed by 8% per level. A global ease adjustment first keeps 75%,
  then 90% of the remaining calculated health and hostile damage, flooring each step;
  counts are per room.
  Walls constrain players/enemies and discard projectiles. The camera follows the player continuously within phase bounds, with 12-pixel outer wall margins. The gameplay HUD shows lives, health,
  shield, score and phase time at top left (no panel), ammunition as a bare
  number in the bottom-right corner, and three weapon slots at top right; phase and room name
  are available during pause. No permanent footer or save banners during play.
- Room layouts are authored, not procedural. Enemy contact damage is not implemented.
- Fire is held with the left mouse button or Space (user request 2026-11-23); both share
  `Jogo::atirando`. Space on the phase summary obeys the same 1 s wait as clicks.
- Movement, automatic nearest-enemy aiming (mouse fallback in empty rooms), sustained shooting, projectiles, enemy pursuit,
  enemy damage, and a pause menu already exist.
- `Jogador` models health and shield (initial/max 50). Stamina/energy was removed from the code;
  shield absorbs incoming shots and regenerates. Powers remain unspecified.
- `FilaArmas` adapts a reusable circular-array FIFO weapon queue with capacity `MAX_ARMAS`
  (currently three). `Armas` holds ammunition and shooting/damage properties.
- Rendering uses a logical resolution of 960x540 in a resizable window.

Inspect the relevant code before changing behavior; update this baseline when
implementation changes make it stale.

## Repository map and commands

- `src/include/`: headers; `src/source/`: implementations, grouped by responsibility.
- `estruturas/`: SDL-independent TADs (`Fila`, `ListaSequencial`); template code
  stays in headers. `ListaSequencial` currently uses `std::vector`, not linked nodes.
- `inventario/`: game-specific adapter `FilaArmas`.
- `sistemas/`: each room owns a `Combate` with its enemies/projectiles and combat rules.
- `mundo/`: `Sala`, `Fase` navigation, and `Campanha` phase progression.
- `persistencia/`: `ArquivoProgresso` reads/writes versioned JSON checkpoints.
- `nucleo/`, `entidades/`, `graficos/`, `interface/`, `config/`: application,
  game entities, rendering helpers, UI, and configuration.
- `Jogo`: SDL lifecycle, main loop, input, game states, and system coordination.
- `Jogador`, `Inimigo`, `Projetil`: player, enemies, and projectiles.
- `Armas`, `FilaArmas`: weapons and the player's weapon queue.
- `Camera`: world/screen coordinates and camera tracking.
- `Hud`: translucent stat bars and ammunition; `Menu`, `Texto`, `Desenho`: menus,
  bitmap text, and drawing helpers.
- `src/include/config/Config.h`: resolution, colors, and tuning constants.
- `README.md`: setup, controls, and naming conventions.

Run commands from the repository root:

- `make`: build `./jogo` and object files under `build/linux/` (first run compiles SDL2,
  libpng and zlib from `externo/`; needs g++, make, cmake, pkg-config, python3).
- `make run`: build and launch the game.
- `make format-check`: check source formatting using clang-format.
- `make format`: apply `.clang-format`; avoid unrelated formatting changes.
- `make clean`: remove game objects, executables, `dist/` and web outputs (keeps compiled
  vendored libraries); `make clean-tudo` also removes them.
- `make dist-linux` / `make dist-windows` / `make dist`: single-file executables in `dist/`
  via Docker.
- `make web` / `make web-zip`: browser build in `build-web/` via Emscripten `em++`
  (emsdk installed by each developer outside the repo; optional for desktop builds),
  and `jogo-web.zip` for itch.io upload.

Preserve existing Portuguese identifiers and follow README naming conventions:
PascalCase classes, lowercase-starting methods, snake_case verb phrases,
camelCase attributes/compound-name suffixes, and UPPER_SNAKE_CASE constants.
Use the existing `.clang-format` configuration.

## Architecture decisions

- The user explicitly wants clear separation of abstract data types and their
  applications in the game for AED1. Keep storage algorithms in `estruturas/`,
  domain adapters in their modules, and gameplay rules in entities/systems.
- Prefer composition and focused classes. TADs must not depend on SDL or game
  types. Do not expose their internal arrays, nodes, or backing containers.
- The weapon queue exposes FIFO operations and front access; arbitrary mutable
  indexing was removed. Empty front access returns nullptr.
- Preserve prototype gameplay during structural refactors. Do not add a stack,
  registry, or linked list solely to claim coverage: introduce each when an
  agreed mechanic needs its semantics. No course ban on STL has been stated.
- `docs/arquitetura.md` explains implemented TADs, complexity, responsibilities,
  and tentative future uses. Keep it and README consistent with this file.

## Campaign and persistence decisions

- Save at new-game start, after each lost life and after each completed phase. The campaign has
  3 lives (`VIDAS_INICIAIS`) and one continue: game over offers it for 10 s (restores 3 lives and
  restarts the phase); otherwise the slot's save is deleted. Score: 100 per basic enemy, 250 per
  brute, 2000 per boss plus a speed bonus 1000*300 s/phase time (`CampanhaConfig.h`). Continue resumes at the
  beginning of the next unfinished phase, not the exact room/position.
- Every phase starts with fresh health and shield. Weapons and ammo carry
  over when advancing (user request, 2026-10-22); Campanha snapshots the FilaArmas at
  phase start, and Restart/death restore that snapshot. An empty carried inventory
  receives the starting Zapper (TipoArma::PISTOLA). The checkpoint stores that snapshot, so Continue restores
  the weapons/ammo the next phase starts with. Pause-menu Restart resets only the current
  phase. Completing phase three shows victory and records campaign completion.
- Three slots `saves/progresso_1..3.json`, relative to the executable directory, resolved via
  SDL_GetBasePath. Schema v3 (written): `versao` = 3, `fases_concluidas` 0..3, `vidas`,
  `continue_disponivel` (0/1), `pontuacao_total`, `armas` = array of
  0..3 `{"tipo": TipoArma index 0..8, "municao": int >= 0}` in FIFO order. v1 (no `armas`)
  and v2 (no lives/continue/score) are still accepted. Saves above phase 3 are rejected.
  A completed campaign disables Continue. New game replaces the old checkpoint.
- Persistence is SDL-independent and accepts an explicit file path. Its narrow
  JSON reader validates the entire v1/v2/v3 schema, version, and ranges. Persistence uses the
  neutral `ArmaSalva`/`ProgressoSalvo` structs; Jogo converts FilaArmas, and
  CampanhaConfig limits are static_asserted against TipoArma/MAX_ARMAS.
  Unknown/duplicate fields and malformed data are rejected. No external JSON
  dependency has been added; extend/replace the parser if the schema grows.
- Saves use a temporary file in the same directory followed by rename; failed
  writes preserve the previous checkpoint. Show failures in menus/pause (hidden during gameplay); F5 retries
  pending writes. Saves are ignored by Git and preserved by make clean.
- Pause shows PAUSADO, phase name and room name (two lines), followed by Continue, Manual, Restart,
  Main menu and Exit. Manual opens the same player manual over the veiled game and returns
  to pause (user request 2026-11-18); the lobby floor no longer draws a controls tutorial,
  controls live only in the manual. Restart resets the current phase/player and resumes gameplay;
  R has no action. Gameplay HUD, inventory details and navigation/checkpoint hints
  are hidden during pause. Keyboard/mouse navigation remains available. Main-menu Continue uses the disk
  checkpoint; returning there abandons unsaved progress within the current phase.

## Maintaining context

- Keep this file in **English**, even when conversations are in Portuguese.
- Update it as user interactions establish or change project goals, decisions,
  constraints, architecture, or implemented features. Record useful rationale
  and outstanding design questions, not a transcript of every conversation.
- Distinguish confirmed decisions, proposed ideas, and implemented behavior.
  Replace stale information instead of accumulating contradictory notes.
- Mobile/touch play (user request 2026-11-20, implemented): see "Touch controls". Not yet
  tested on real phones/tablets, only Chromium mobile emulation (`web-teste/pw_toque.mjs`).
- Current open decisions include powers, final numerical balancing, branching/procedural stages,
  and further progression.
  Procedural generation remains unspecified. Death currently restarts the active phase.
- This document preserves context; it does not authorize implementing the
  entire roadmap without a corresponding user request.

## Workspace preferences

- Keep reusable ad hoc tests, verification scripts, screenshots, logs, and other
  diagnostic artifacts outside repositories, in a persistent directory belonging to
  the coding agent you are (user request 2026-11-18). Each agent uses its own folder:
  - Codex: `~/.codex/` (existing folders such as `~/.codex/aed1/`, `web-teste/`,
    `dist-teste/`, `audio-teste/`, `telas-teste/`).
  - Claude Code: `~/.claude/aed1-testes/` (a copy of the reusable scripts above, with a
    README describing each tool).
  - OpenCode or any other agent: the equivalent persistent folder of that agent (for
    example under its config/data directory, such as `~/.config/opencode/`), in an
    `aed1-testes/` subfolder. If it has no such folder, ask the user.
  Paths in this document written as `~/.codex/...` refer to Codex's copy; other agents
  should look for the same-named subfolder in their own directory, and may copy scripts
  from another agent's folder into theirs when useful. Never read or copy another
  agent's credentials, sessions or history.
  Reuse existing scripts where available.
- Run project commands from their required working directory, but store ad hoc
  test files and generated diagnostic output in your own agent folder whenever possible.
- Do not add test or verification files to the repository unless the user
  explicitly requests them as deliverables.
- During cleanup, move only artifacts created by you (the current agent). Preserve existing and
  tracked project files.
- When a reported issue is resolved, remove its entry from the relevant review
  document. Keep review documents focused on unresolved issues instead of
  appending resolved notes.

## HUD decisions

- Each living enemy draws a simple red health bar above its body, with no icon
  or text. Its fill uses current health divided by that enemy's initial maximum,
  including difficulty scaling; the bar disappears when the enemy dies.

- The user wants nearly the whole window usable for gameplay, health,
  shield, lives, score, time, ammunition, and the three-slot weapon inventory visible while playing. Hide tutorial footers,
  room status, and save messages during gameplay; pause shows only phase and room name.
- `interface/Hud` draws, without a background panel, lives (Tux heads), health and shield
  bars with custom icons and values, score and phase time at top left. Ammunition is only a number
  at bottom right, without a card or label. `Fase` no longer draws UI. The first room is 936x516 so
  the floor plus 12-pixel walls fills the 960x540 logical viewport.
- Shield starts at 50/50, absorbs shots before health, and regenerates 10/s after
  3 seconds without damage. Hits grant 0.5 s invulnerability.

## Sprite sizing and hitboxes

- `config/DimensoesEntidade.h` separates per-entity collision width/height from
  visual frame width/height. Defaults: hero 32x32 / 48x48; common enemy 28x28 /
  48x48; bodyguard 44x44 / 64x64; boss 88x88 / 128x128. Brutes and boss use the larger profiles in phase 1.
- Positions refer to the hitbox's top-left corner. Visual frames are centered
  on hitboxes, not foot-anchored. World coordinates use logical pixels at 960x540.
  Collision rectangles retain the original player/common-enemy sizes.
- Entity constructors accept custom positive rectangular profiles whose visual
  frame contains the hitbox. Enemy separation handles differing rectangle sizes.
  Walls/doors/projectiles use get_retangulo(); rendering uses get_retanguloVisual().
  Aim/fire origins remain at the hitbox center; health bars sit above the visual frame.
- F2 toggles green hitbox outlines during gameplay. Viruses use an embedded chip bitmap; bodyguards (brutes) and bosses use
  per-phase embedded pixel art in `graficos/SpriteInimigo` (16x16 and 24x24 bitmaps). The player uses
  the Tux PNG. Animation is not implemented.
- `docs/sprites.md` defines transparent PNG frame sizes, offsets and alignment for
  user-created art. Keep it synchronized with the size profiles. Character selection
  is not implemented; phase resets still create the default hero profile.

## Player sprite asset

- The player uses `assets/sprites/final/00_tux.png` (see "Final integration"); game over
  uses `personagens/tux-gameover-v2.png`. On 2026-11-23 (user request) every PNG not listed
  in `ASSETS_EMBUTIDOS` was deleted (tux.png, tux1/2, tux-pixel, tux-pixel-v2,
  virus-conceito); they remain in git history. Keep `assets/` limited to files the build uses.
- `graficos/Sprite` decodes an embedded PNG via libpng into an SDL texture, with alpha
  blending and nearest-neighbor scaling.
- Jogo owns the noncopyable texture resource; load once at startup and free before
  destroying the renderer. Player resets must not own/copy/reload textures.
- Sprites are embedded in the executable (see "Native distribution builds"); invalid
  embedded PNG data produces a startup error. Saves still resolve via SDL_GetBasePath.

## Weapon inventory and combat

- Nine weapon types form an affinity/range matrix in `Armas`: red antivirus has
  Escopeta ClamAV / Pulso Malwarebytes / Raio VirusTotal; blue open source has
  Lamina de Torvalds / Garfo Git / Arco Apache; green retro hardware has
  Disquete Bumerangue / Zapper 8-Bits / Canhao CRT. Each trio is short/medium/long.
  Matching weapon and enemy affinity doubles base damage; mismatches and the neutral
  empty-hand punch retain base damage.
- `FilaArmas` holds three weapons. F collects the nearest floor weapon within
  54 pixels into the tail; full inventory leaves it untouched. Q moves the front
  to the tail through FIFO operations; G drops the front with its remaining ammo.
  These supplementary controls and floor distribution are initial implementation choices.
- All phases use proximity-opened chests in their chest rooms. The first drop matches the phase
  boss affinity and the second is uniform across all nine types (duplicates allowed), with no inventory insertion. `Sala` owns floor
  items and preserves collections/drops on revisits. Phase reset/advancement recreates
  the player and items. Existing phase checkpoint semantics remain unchanged.
- Each finite-ammo attack costs one unit, regardless of projectile count. New weapons
  start with 40 units; sword/boomerang are reusable (HUD displays `--`). Each weapon
  has its own cadence; switching does not bypass the current firing cooldown.
- Laser/sword damage is immediate with brief visual traces. Projectiles update in
  steps no longer than 1/240 second. Boomerangs return toward the player, damage once
  per enemy per outbound/return leg, and expire if unable to return. Dead enemies
  remain inactive in the room list to preserve hit indices until room reset.
- Floor weapons reuse the inventory's IconeArma sprites in centered 36x36 frames,
  with names below, including chest drops and weapons dropped by the player.
  Weapon icons appear in the gameplay HUD; pause omits inventory details.

## Inventory HUD

- M toggles a phase minimap during gameplay or pause; M/Esc closes it and preserves
  the prior gameplay/pause state. While open, simulation and gameplay actions stop,
  held fire is cleared, and repeated keydown events do not toggle it. Entry resets it.
- interface/Minimapa draws the revealed progression tree using the real room origins
  exposed read-only by Fase::get_origemSala. Positive axis scales preserve all
  left/right/up/down relations and alignments. Full-phase bounds keep revealed nodes
  stationary as exploration progresses. Pre-order selects visible nodes only; it
  must not order siblings geometrically. Each parent-child edge is drawn straight
  between the corresponding real doors, and the child label includes the same
  ESQ/DIR/CIMA/BAIXO direction. In phase 1 B is left of A, C right, D below;
  inserting labels alone did not fix the former misleading hierarchical layout.
  The root exists on phase construction; clearing a room inserts/prepares its fixed
  children before visiting them. M never inserts nodes. All physical corridors are
  exactly parent-child edges of the same canonical tree.
  Current room has a gold border and a position marker; cleared/unvisited/combat
  rooms are green/gray/red, with chest/boss labels. Unrevealed nodes are not displayed.

- The user limited inventory to three weapons. `MAX_ARMAS` controls capacity,
  HUD slot count and the pause counter. Full inventory still refuses collection.
- Top-right HUD uses a 64x64 equipped slot on the left with a double gold border,
  then two 48x48 reserve slots in FIFO order, vertically centered, with 8-pixel
  gaps and 16-pixel window margins. Empty slots stay visible without an icon. A thick
  counterclockwise white cycle arrow with a filled head, subtle shadow and centered Q
  appears immediately to the left of the hotbar.
- `FilaArmas::consultar` delegates read-only logical indexing to `Fila`; no mutable
  indexing or backing storage is exposed. Collect/drop/rotate update the HUD directly.
- `graficos/IconeArma` draws distinct embedded 12x12 pixel-art bitmap sprites for
  all nine weapon types, scaled inside slots. No new external asset dependency.
- The player now holds the equipped weapon's same bitmap icon, drawn at 3x scale
  near the hand and rotated with the current aim. Local vertical mirroring keeps grips
  upright when facing left; sword/bow/boomerang use appropriate grip/orientation
  adjustments. An empty inventory draws no held weapon. Shooting rules are unchanged.

## Combat, enemy AI and rewards

- Basic, medium and boss enemies have effective phase-1 health 2/13/67 and shots dealing
  5/8/11 damage after the 25%-then-10% ease adjustment. They fire at
  2.4/1.8/0.85-second base intervals (+-15% jitter; random 1.1-2.1 s first delay); the
  boss speeds up to 0.62 s below half health. Hostile shots travel at 230 px/s (base).
- AI (user request 2026-10-22): enemies keep preferred distances (virus 150, brute 110,
  boss 230), strafe around the player, flip direction when stuck, and when cover blocks
  the line of sight (checked per frame, widened by the shot size) they hold fire and
  circle around it. Shots lead the player's estimated velocity (virus 30%, brute 60%,
  boss 75%). Brutes fire a 3-shot fan every third attack. The boss cycles an aimed
  3-shot fan, a rotating 12-shot ring every fourth attack, and summons two telegraphed
  viruses every fifth attack while fewer than four minions exist.
- Reinforcement waves (`ondasExtras` per room): when at most one enemy remains, the next
  wave (2/3 of the viruses, min 2, and half the brutes) is announced by pink frames for
  1 s at points >= 260 px from the player and clear of cover. A room clears only when
  `Combate::concluido()` (no living enemies, pending waves or announcements).
  Each strength/affinity combination has provisional embedded artwork (see Phase themes).
- Every enemy drops one heart once, including instant-attack deaths. Hearts heal
  10 health on contact if health is missing and expire after 10 seconds of gameplay,
  including time in other rooms. They blink in the final two seconds.
- Chests open automatically within 64 pixels, with gold/wood embedded SDL artwork.
  Weapons stay on the floor for F collection; full inventory never consumes them.
- These counts, timings, stats, chest-opening interaction and automatic phase
  restart on death are initial implementation choices, open to balancing.

## Continuous corridors and environment

- All room connections are walkable, enemy-free corridors, 96 logical pixels wide
  and 240 long. Doors lock only during combat in their own room; room
  ownership changes only at the far end, preserving world position. Enemies remain
  confined to room floors. Corridors do not add rooms to the minimap or storage TAD.
- Fase stores room origins and renders the connected environment using translated
  cameras while gameplay entities retain room-local coordinates. Only the active
  room simulates combat; neighboring rooms are visible during traversal.
- graficos/Ambiente draws deterministic plate floors with subtle circuit details,
  segmented relief walls, gate lights/grilles, and a dark violet floor portal with
  a stone rim. No external texture dependency was added.
- All three phases finish through a central portal after defeating their boss. Checkpoint and phase-reset semantics are unchanged.

- Room/corridor collision uses overlapping valid hitbox-position rectangles, so
  both outward travel and reversal preserve movement through the doorway. The
  last movement step is preserved when switching room-local coordinates; ordinary
  room changes also preserve held fire and the firing cooldown.
- Clearing the current room prepares its adjacent rooms' enemies, chests and floor
  items once, before the player reaches them. The initial cleared lobby does this
  too. Preparation keeps rooms unvisited and enemies idle; entering activates
  combat without recreating entities or temporarily relocating the player.
