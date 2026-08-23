# Contributing to magnolia

## Design rule

magnolia holds what more than one game needs. Anything shaped by a single game
belongs in that game. When something looks reusable but has exactly one consumer,
leave it in the game until a second one appears — an abstraction designed against
one example usually fits only that example.

See [README.md](README.md#design-rule) for concrete examples.

## Code style

- **C99** with no compiler extensions beyond what devkitPPC and gcc provide.
- Build with **-Wall -Wextra**, warning clean. If the compiler complains, fix it.
- **No wildcards in Makefile source lists.** The list is the record of which
  modules are host-clean. A wildcard would cheerfully try to link renderer.c
  in a host test binary.
- Keep header comments thorough. They serve as design documents, not just API
  signatures. Explain *why*, not just *what*.

## Testing

`make test` must pass before merging. Every engine module that is free of libogc
runs as its own host-side binary, so a failure in one does not mask another.

When adding a new host-clean module:

1. Add it to `HOST_TESTS` in the Makefile.
2. Write a `tests/test_<module>.c` with the existing `harness.h` framework.
3. Verify it runs on the host: `make test-<module>`.

When adding a new dependency on libogc (GRRLIB, ASND, fat, etc.), the module
cannot be host-tested. That is fine — not every module needs to be — but do not
break the existing host tests.

## Commit messages

Write descriptive commit messages that explain *why* the change was made, not
just *what* changed. The existing history is the style guide:

```
Short summary of the change (imperative mood, <72 chars)

Longer explanation of why this change is needed, what it replaces, and what
trade-offs were considered. Reference specific bugs, games, or design decisions.
```

Keep the subject line under 72 characters. Wrap the body at 80.

## Pull requests

- Engine changes must not break `make` (standalone build) or any game that
  depends on magnolia.
- Run `make test` locally before pushing.
- Keep PRs focused. One logical change per PR makes review easier and the
  changelog cleaner.

## AI attribution

No AI attribution in commits, PRs, or release notes. Do not append
`Co-Authored-By: Claude ...`, "Generated with ...", or any similar trailer. See
[AGENTS.md](AGENTS.md) for the full policy.

## What belongs where

**In the engine:**

- Sprite loading and the origin concept
- High-score tables and persistence
- Game state machine (title, menu, play, game over, initials, leaderboard)
- Grid/list cursor navigation
- PCM audio playback
- HSL theme generation
- Design-space to TV-safe layout
- Frame timing and easing

**In the game:**

- A particular character roster
- Player physics (gravity, thrust, collision)
- What happens during gameplay
- Which items are in a grid
- Which track plays when
- What colour the player is
- Where a specific UI element sits

If you are unsure whether something belongs in the engine, the answer is
probably "not yet." Leave it in the game until a second consumer appears.
