Project: bareiron (mcu-mc)

Goal
----
Help contributors (and AI agents) make small, safe, and correct changes to a minimalist Minecraft server targeted at PCs and microcontrollers (ESP-IDF).

Quick context the agent must know
---------------------------------
- This repo implements a tiny Minecraft server prioritizing memory and performance over full vanilla compatibility. See `README.md`.
- Two main targets: native PC (Cosmopolitan exe) and ESP-IDF (PlatformIO board: `seeed_xiao_esp32s3`).
- Registries (Minecraft ID lists and tags) are generated into `include/registries.h` and `src/registries.c` by `build_registries.js` using dumped data under `notchian/generated` (see `build_registries.js`).

Essential workflows (commands an agent can suggest or run)
--------------------------------------------------------
- Generate registries: run `node build_registries.js` (or `bun run build_registries.js`). It expects `notchian/generated` populated by a vanilla server data dump (see README). CI uses Bun.
- Build native binary locally: `./build.sh` (Linux/MSYS) — compiles `src/*.c` with `-Iinclude` into `bareiron`/`bareiron.exe`.
- CI build: GitHub Actions uses Cosmopolitan and Bun to dump registries and compile `bareiron.exe` (see `.github/workflows/build.yml`).
- Build for ESP: use PlatformIO with `platformio.ini` (`env:seeed_xiao_esp32s3`, framework `espidf`). Typical flow: create PlatformIO project, copy repo on top, enable LittleFS if desired and adjust `include/globals.h` macros.

Important files and what they contain
-----------------------------------
- `README.md` — high-level goals, compilation notes, and configuration guidance.
- `build_registries.js` — converts Mojang data generator output into C headers/source. Inspect this when fixing registry-related bugs or changing supported blocks/biomes.
- `include/registries.h`, `src/registries.c` — generated; treat as derived artifacts. Don't hand-edit them; instead modify generator or the input `notchian` data.
- `include/globals.h` — primary configuration knobs (feature flags like `ALLOW_CHESTS`, movement scaling, tick timing). Many behaviors are controlled by these macros.
- `src/main.c` and `src/*.c` — server core, packet handling, worldgen, serialization. Packet handlers live here and call helpers in `packets.c`, `procedures.c`, etc.

Conventions and patterns to follow
---------------------------------
- Performance and memory efficiency are first-class constraints. Prefer small data structures and in-place updates over heap allocations.
- Feature toggles are implemented as `#define` macros in `include/globals.h`. To change a build-time feature, modify those macros and recompile.
- Generated files under `include/` and `src/` (registries) are considered build artifacts; modify generation scripts and input data rather than editing them manually.
- Keep changes small and self-contained. The maintainer requests pre-discussion for larger PRs (see `README.md`).

Integration points and external dependencies
------------------------------------------
- Java/Mojang data generation: CI downloads a vanilla server JAR and runs the data generator to populate `notchian/generated`.
- Tools: `node`/`bun` required for `build_registries.js`. CI uses Bun; recommend Bun or Node.js locally.
- Cosmopolitan (`cosmocc`) is used in CI to create single-file cross-platform executable.
- PlatformIO/ESP-IDF used for embedded builds (`platformio.ini` present). For ESP builds the code uses `ESP_PLATFORM` guards to include FreeRTOS and ESP headers.

Testing and validation hints
---------------------------
- Small edits: run `./build.sh` on a Unix-like shell (or use the GitHub Actions workflow steps locally) to compile and smoke-test the native binary.
- Registry changes: run `bun run build_registries.js` and confirm `include/registries.h` and `src/registries.c` update. The generator expects `notchian/generated` created by the Minecraft data pack run.
- ESP changes: use PlatformIO (`pio run --environment seeed_xiao_esp32s3`) or the VSCode PlatformIO extension.

Examples from this codebase
--------------------------
- Feature flag: toggling `ALLOW_CHESTS` in `include/globals.h` disables chest code paths and reduces memory.
- Movement scaling: `SCALE_MOVEMENT_UPDATES_TO_PLAYER_COUNT` and `BROADCAST_ALL_MOVEMENT` live in `include/globals.h` and change how `src/main.c` broadcasts player positions.
- Registry generation: `build_registries.js` writes `registries.c/.h`; CI calls `bun run build_registries.js` before compiling.

Safety and edit guidance
------------------------
- Do not commit generated `notchian/generated` dumps. Commit only the generated `include/registries.h`/`src/registries.c` if and when the generator is intended to run in CI for releases.
- When changing protocol handling (`src/*.c`, `packets.c`), prefer minimal behavioural changes and add CI-proven smoke tests where possible; packet parsing is fragile and performance-sensitive.

When in doubt
------------
- Read `README.md` and `include/globals.h` for design intent and configuration.
- For registry or protocol changes, prefer modifying `build_registries.js` or generator inputs instead of editing generated C headers.

If you need more
--------------
Ask for examples of a specific change (small refactor, feature flag tweak, or registry update) and I will create a minimal, tested PR with build steps.
