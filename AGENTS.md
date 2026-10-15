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

As inspected on 2026-10-14:

- The build uses C++17, SDL2, and a Makefile. Text uses an embedded bitmap font;
  SDL2_ttf is not required.
- The current world is open and borderless, with a following camera and a
  reference grid. This is the prototype's current behavior; the intended
  Soul Knight-inspired stages and challenge rooms are future work.
- Movement, mouse aiming, sustained shooting, projectiles, enemy pursuit,
  enemy damage, and a pause menu already exist.
- `Jogador` models health and stamina and provides methods to change them.
  Their presence does not imply a complete damage, stamina-cost, or powers system.
- `FilaArmas` is an array-backed FIFO weapon queue with capacity `MAX_ARMAS`
  (currently five). `Armas` holds ammunition and shooting/damage properties.
- Rendering uses a logical resolution of 960x540 in a resizable window.

Inspect the relevant code before changing behavior; update this baseline when
implementation changes make it stale.

## Repository map and commands

- `src/include/`: headers; `src/source/`: implementations.
- `Jogo`: SDL lifecycle, main loop, game states, entities, and collisions.
- `Jogador`, `Inimigo`, `Projetil`: player, enemies, and projectiles.
- `Armas`, `FilaArmas`: weapons and the player's weapon queue.
- `Camera`: world/screen coordinates and camera tracking.
- `Menu`, `Texto`, `Desenho`: pause menu, bitmap text, and drawing helpers.
- `src/include/Config.h`: resolution, colors, and tuning constants.
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

## Maintaining context

- Keep this file in **English**, even when conversations are in Portuguese.
- Update it as user interactions establish or change project goals, decisions,
  constraints, architecture, or implemented features. Record useful rationale
  and outstanding design questions, not a transcript of every conversation.
- Distinguish confirmed decisions, proposed ideas, and implemented behavior.
  Replace stale information instead of accumulating contradictory notes.
- Current open decisions include the final hero/boss/bodyguard rosters, powers,
  weapon variety, stage and room structure, progression, and stamina rules.
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
