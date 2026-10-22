# Project context and working instructions

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

- The build uses C++17, SDL2, libpng, and a Makefile. Text uses an embedded bitmap font;
  SDL2_ttf is not required.
- The game starts at a main menu. Three phases remain (reduced from five by user request), all
  branching graphs of 936x516 rooms with lobby, chest rooms and a final boss. Phase 1 has seven
  936x516 rooms in a directional graph: Lobby–A, A–B (south), A–C (east),
  A–D (north), D–E (east), C–E (north), E–F (north).
  A has 3 viruses; C/D each have 4 viruses and 2 brutes. B has one chest,
  E two chests; lobby/B/E have no enemies. F has one boss.
- Each room owns its combat and persistent state in ListaSequencial<Sala>.
  Four neighbor indices describe phase 1 connections. Doors lock only while their own room is in active combat; empty/cleared rooms
  stay open, including E–F (the previous A/C/D prerequisite was removed). Defeating F's boss
  creates a central portal; touching it completes the phase. Phase 2 (9 rooms, difficulty 2):
  Lobby–A, A–B (north, 1 chest), A–C (south), A–D, C–E, D–F, E–G (2 chests), F–G, F–H (north, boss).
  Phase 3 (11 rooms, difficulty 3) climbs north from the lobby: Lobby–A, A–B/C (west/east),
  B–D, C–E, C–F (1 chest), D–G, G–E, E–H (2 chests), G–I, I–J (boss + 2 viruses).
  Layouts are constant tables (`PlantaSala`) in Fase.cpp; per-room counts are initial balancing choices.

## Phase themes

Each phase combines a computing theme with a game reference (names chosen by Claude
on user request; open to change). Inimigo carries the phase number as `tema`.
- 1 FASE DOS COMPILADORES / SPEEDRUN DO BUILD: bodyguard SEGFAULT, boss COMPILADOR FECHADO.
- 2 FASE DA REDE / MULTIPLAYER COM LAG: bodyguard SPAMBOT, boss REDE ANTISSOCIAL.
- 3 FASE DO KERNEL / GAME OVER EM TELA AZUL: bodyguard PROCESSO ZUMBI, boss TELA AZUL DA MORTE.
Rooms have themed names; boss room names equal the boss name. The phase name and
subtitle appear for 3 s (fading) on phase entry/restart; pause and minimap show the name.
- Pause-menu Restart and player death reset the current phase/player without losing completed
  phases. Cleared enemies/chests never respawn on revisits. Projectiles are
  discarded on leaving. Only active combat updates; hearts in inactive rooms
  still expire. Pause freezes simulation. Room arrivals preserve position across the connecting corridor.
- Difficulty increases enemy count (configured per room), health, and speed.
  Walls constrain players/enemies and discard projectiles. The camera follows the player continuously within phase bounds, with 12-pixel outer wall margins. The gameplay HUD shows health, shield,
  energy in one compact translucent top-left box, ammunition as a bare
  number in the bottom-right corner, and three weapon slots at top right; phase and room name
  are available during pause. No permanent footer or save banners during play.
- Room layouts are authored, not procedural. Enemy contact damage is not implemented.
- Movement, automatic nearest-enemy aiming (mouse fallback in empty rooms), sustained shooting, projectiles, enemy pursuit,
  enemy damage, and a pause menu already exist.
- `Jogador` models health, shield (initial/max 50), and stamina and provides methods to change them.
  Shield absorbs incoming shots and regenerates; stamina costs and powers remain unspecified.
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

- `make`: build `./jogo` and object files under `build/`.
- `make run`: build and launch the game.
- `make format-check`: check source formatting using clang-format.
- `make format`: apply `.clang-format`; avoid unrelated formatting changes.
- `make clean`: remove generated build objects and the executable.

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

- Save at new-game start and after each completed phase. Continue resumes at the
  beginning of the next unfinished phase, not the exact room/position.
- Every phase starts with fresh health, shield and stamina. Weapons and ammo carry
  over when advancing (user request, 2026-10-22); Campanha snapshots the FilaArmas at
  phase start, and Restart/death restore that snapshot. An empty carried inventory
  receives the starting pistol. Loading a checkpoint starts with the pistol because the
  v1 save does not store weapons (open question whether to persist them). Pause-menu Restart resets only the current
  phase. Completing phase three shows victory and records campaign completion.
- `saves/progresso.json` is relative to the executable directory, resolved via
  SDL_GetBasePath. Schema v1: `versao` = 1, `fases_concluidas` = integer 0..3. Old saves above 3 are rejected as invalid.
  A completed campaign disables Continue. New game replaces the old checkpoint.
- Persistence is SDL-independent and accepts an explicit file path. Its narrow
  JSON reader validates the entire two-integer schema, version, and range.
  Unknown/duplicate fields and malformed data are rejected. No external JSON
  dependency has been added; extend/replace the parser if the schema grows.
- Saves use a temporary file in the same directory followed by rename; failed
  writes preserve the previous checkpoint. Show failures in menus/pause (hidden during gameplay); F5 retries
  pending writes. Saves are ignored by Git and preserved by make clean.
- Pause shows PAUSADO, phase name and room name (two lines), followed by Continue, Restart,
  Main menu and Exit. Restart resets the current phase/player and resumes gameplay;
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
- Current open decisions include the final hero/boss/bodyguard rosters, powers,
  further weapon balancing/variety, branching/procedural stages, further progression, and stamina rules.
  Procedural generation remains unspecified. Death currently restarts the active phase.
- This document preserves context; it does not authorize implementing the
  entire roadmap without a corresponding user request.

## Workspace preferences

- Keep reusable ad hoc tests, verification scripts, screenshots, logs, and other
  diagnostic artifacts outside repositories in a persistent directory under
  `~/.codex`. Reuse existing scripts where available.
- Run project commands from their required working directory, but store ad hoc
  test files and generated diagnostic output under `~/.codex` whenever possible.
- Do not add test or verification files to the repository unless the user
  explicitly requests them as deliverables.
- During cleanup, move only artifacts created by Codex. Preserve existing and
  tracked project files.
- When a reported issue is resolved, remove its entry from the relevant review
  document. Keep review documents focused on unresolved issues instead of
  appending resolved notes.

## HUD decisions

- Each living enemy draws a simple red health bar above its body, with no icon
  or text. Its fill uses current health divided by that enemy's initial maximum,
  including difficulty scaling; the bar disappears when the enemy dies.

- The user wants nearly the whole window usable for gameplay, health,
  shield, energy, ammunition, and the three-slot weapon inventory visible while playing. Hide tutorial footers,
  room status, and save messages during gameplay; pause shows only phase and room name.
- `interface/Hud` owns drawing of one translucent 200x104 top-left box containing three
  stacked stat bars with custom icons and values. Ammunition is only a number
  at bottom right, without a card or label. `Fase` no longer draws UI. The first room is 936x516 so
  the floor plus 12-pixel walls fills the 960x540 logical viewport.
- Shield starts at 50/50, absorbs shots before health, and regenerates 10/s after
  3 seconds without damage. Hits grant 0.5 s invulnerability. Energy reflects stamina.

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

- The user-provided root `tux.png` was moved intact to
  `assets/sprites/personagens/tux.png`. It is a 48x61 RGBA PNG, rendered with
  preserved aspect ratio inside the 48x48 hero visual frame (about 37.8x48).
  Hitbox remains 32x32; weapon aiming/drawing remains separate.
- `graficos/Sprite` loads PNG via libpng into an SDL texture, with alpha blending
  and nearest-neighbor scaling. Makefile now requires pkg-config sdl2 libpng.
- Jogo owns the noncopyable texture resource; load once at startup and free before
  destroying the renderer. Player resets must not own/copy/reload textures.
- Assets resolve relative to SDL_GetBasePath, alongside the executable. Missing
  or invalid required Tux assets produce a startup error. Distribute assets/ with jogo.

## Weapon inventory and combat

- Seven weapon types now extend the existing `Armas` class (the repository name,
  retained instead of renaming to singular): shotgun (three pellets at 0/+15/-15
  degrees), piercing instant laser, forward/backward double shot, wall ricochet
  (up to four bounces), 80-pixel/90-degree sword fan, bow/arrow, and returning boomerang.
- `FilaArmas` holds three weapons. F collects the nearest floor weapon within
  54 pixels into the tail; full inventory leaves it untouched. Q moves the front
  to the tail through FIFO operations; G drops the front with its remaining ammo.
  These supplementary controls and floor distribution are initial implementation choices.
- All phases use proximity-opened chests in their chest rooms, each dropping two random weapons
  uniformly from all eight types (duplicates allowed), with no inventory insertion. `Sala` owns floor
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
- interface/Minimapa draws all rooms and actual directional connections using
  read-only Fase room/neighbor queries for every branching phase.
  Current room has a gold border and a position marker; cleared/unvisited/combat
  rooms are green/gray/red, with chest/boss labels. No exploration fog is applied.

- The user limited inventory to three weapons. `MAX_ARMAS` controls capacity,
  HUD slot count and the pause counter. Full inventory still refuses collection.
- Top-right HUD uses a 64x64 equipped slot on the left with a double gold border,
  then two 48x48 reserve slots in FIFO order, vertically centered, with 8-pixel
  gaps and 16-pixel window margins. Empty slots stay visible without an icon.
- `FilaArmas::consultar` delegates read-only logical indexing to `Fila`; no mutable
  indexing or backing storage is exposed. Collect/drop/rotate update the HUD directly.
- `graficos/IconeArma` draws distinct embedded 12x12 pixel-art bitmap sprites for
  all eight weapon types, scaled inside slots. No new external asset dependency.
- The player now holds the equipped weapon's same bitmap icon, drawn at 3x scale
  near the hand and rotated with the current aim. Local vertical mirroring keeps grips
  upright when facing left; sword/bow/boomerang use appropriate grip/orientation
  adjustments. An empty inventory draws no held weapon. Shooting rules are unchanged.
- The user chose to retain the existing embedded virus visual. The generated
  virus-conceito.png is unused concept art, not the active enemy sprite.

## Phase 1 combat and rewards

- Viruses, brutes and boss have base health 2/12/65 and simple aimed shots dealing
  8/12/18 damage at 2.4/1.8/0.85-second intervals after an initial 1.5-second delay.
  They stop pursuing within 180 pixels; hostile shots travel at 230 pixels/s.
  Bodyguard/boss artwork follows the phase theme (see Phase themes).
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
