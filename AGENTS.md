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

As inspected on 2026-10-15:

- The build uses C++17, SDL2, libpng, and a Makefile. Text uses an embedded bitmap font;
  SDL2_ttf is not required.
- The game starts at a main menu (New game, Continue, Exit). The campaign has
  five sequential authored phases, each with three bounded rooms.
  Each `Sala` owns its `Combate` and persists its state when revisited. `Fase`
  stores rooms in `ListaSequencial<Sala>` and controls navigation.
- Rooms have configurable name, size, difficulty, enemy count, and release
  requirements (currently eliminate all enemies, without a timer). Phase 1 rooms:
  936x516 / difficulty 1 / 4 enemies; 1120x640 / difficulty 2 / 7 enemies;
  1440x800 / difficulty 3 / 10 enemies. Each subsequent phase adds 1 to
  difficulty and 2 enemies per room.
- Doors remain locked until requirements are met. Touching the right/left door
  automatically advances/returns (no interaction key or proximity radius); the last right door completes the phase. R resets the entire
  current phase/player without discarding completed phases. Cleared rooms do not respawn enemies; projectiles are discarded
  on leaving. Only the active room updates. Arrival positions are away from doors to avoid
  immediate return transitions. The user explicitly removed minimum room timers.
- Difficulty increases enemy count (configured per room), health, and speed.
  Walls constrain players/enemies and discard projectiles. The camera is bounded
  with only 12-pixel wall margins. The gameplay HUD shows only health, shield,
  energy in one compact translucent top-left box, and ammunition as a bare
  number in the bottom-right corner; room/difficulty/enemy
  details are available during pause. No permanent footer or save banners during play.
- This first room system is deterministic, not procedural. Enemy contact damage,
  bosses, non-weapon pickups, and death rules are still not implemented.
- Movement, mouse aiming, sustained shooting, projectiles, enemy pursuit,
  enemy damage, and a pause menu already exist.
- `Jogador` models health, shield (initial/max 50), and stamina and provides methods to change them.
  Their presence does not imply a complete damage, stamina-cost, or powers system.
- `FilaArmas` adapts a reusable circular-array FIFO weapon queue with capacity `MAX_ARMAS`
  (currently five). `Armas` holds ammunition and shooting/damage properties.
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
- Every phase starts with a fresh player (health, stamina, starting weapon/ammo),
  consistently for advancement, restart, and loading. R restarts only the current
  phase. Completing phase five shows victory and records campaign completion.
- `saves/progresso.json` is relative to the executable directory, resolved via
  SDL_GetBasePath. Schema v1: `versao` = 1, `fases_concluidas` = integer 0..5.
  A completed campaign disables Continue. New game replaces the old checkpoint.
- Persistence is SDL-independent and accepts an explicit file path. Its narrow
  JSON reader validates the entire two-integer schema, version, and range.
  Unknown/duplicate fields and malformed data are rejected. No external JSON
  dependency has been added; extend/replace the parser if the schema grows.
- Saves use a temporary file in the same directory followed by rename; failed
  writes preserve the previous checkpoint. Show failures in menus/pause (hidden during gameplay); F5 retries
  pending writes. Saves are ignored by Git and preserved by make clean.
- Pause offers Resume, Main menu, and Exit. Main-menu Continue uses the disk
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
  Procedural generation and death/restart rules have not yet been specified.
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

- The user wants nearly the whole window usable for gameplay, only health,
  shield, energy, and ammunition visible while playing. Hide tutorial footers,
  room status, and save messages during gameplay; keep details accessible in pause.
- `interface/Hud` owns drawing of one translucent 200x104 top-left box containing three
  stacked stat bars with custom icons and values. Ammunition is only a number
  at bottom right, without a card or label. `Fase` no longer draws UI. The first room is 936x516 so
  the floor plus 12-pixel walls fills the 960x540 logical viewport.
- Shield is a real bounded player attribute, initially 50/50, with recovery/loss
  methods. No damage absorption, regeneration, or contact-damage mechanics were
  introduced by the HUD change. Energy still reflects existing stamina state.

## Sprite sizing and hitboxes

- `config/DimensoesEntidade.h` separates per-entity collision width/height from
  visual frame width/height. Defaults: hero 32x32 / 48x48; common enemy 28x28 /
  48x48; bodyguard 44x44 / 64x64; boss 88x88 / 128x128. Bodyguard/boss profiles
  are reserved, not spawned or implemented as new gameplay types.
- Positions refer to the hitbox's top-left corner. Visual frames are centered
  on hitboxes, not foot-anchored. World coordinates use logical pixels at 960x540.
  Collision rectangles retain the original player/common-enemy sizes.
- Entity constructors accept custom positive rectangular profiles whose visual
  frame contains the hitbox. Enemy separation handles differing rectangle sizes.
  Walls/doors/projectiles use get_retangulo(); rendering uses get_retanguloVisual().
  Aim/fire origins remain at the hitbox center; health bars sit above the visual frame.
- F2 toggles green hitbox outlines during gameplay. Enemy placeholder rectangles show visual frame sizes; the player now uses
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
  retained instead of renaming to singular): shotgun (three pellets at 0/+45/-45
  degrees), piercing instant laser, forward/backward double shot, wall ricochet
  (up to four bounces), 80-pixel/90-degree sword fan, bow/arrow, and returning boomerang.
- `FilaArmas` still holds five weapons. F collects the nearest floor weapon within
  54 pixels into the tail; full inventory leaves it untouched. Q moves the front
  to the tail through FIFO operations; G drops the front with its remaining ammo.
  These supplementary controls and floor distribution are initial implementation choices.
- Each room initially offers all seven new types on its left side. `Sala` owns floor
  items and preserves collections/drops on revisits. Phase reset/advancement recreates
  the player and items. Existing phase checkpoint semantics remain unchanged.
- Each finite-ammo attack costs one unit, regardless of projectile count. New weapons
  start with 40 units; sword/boomerang are reusable (HUD displays `--`). Each weapon
  has its own cadence; switching does not bypass the current firing cooldown.
- Laser/sword damage is immediate with brief visual traces. Projectiles update in
  steps no longer than 1/240 second. Boomerangs return toward the player, damage once
  per enemy per outbound/return leg, and expire if unable to return. Dead enemies
  remain inactive in the room list to preserve hit indices until room reset.
- Floor weapons have provisional labeled drawings. Equipped weapon and inventory
  count/controls are shown in pause; no extra permanent gameplay HUD was added.
