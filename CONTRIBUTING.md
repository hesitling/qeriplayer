# Contributing to QeriPlayer Qt

## Git Hooks

This repo ships pre-commit hooks in `.githooks/` that run format checks and tests automatically. Enable them after cloning:

```bash
just setup-hooks
```

Or manually:

```bash
git config core.hooksPath .githooks
```

### What the pre-commit hook does

1. **Format check** — runs `clang-format` on staged `.h`/`.cpp` files
2. **Build + test** — runs the full test suite if any source files changed

Skip with `git commit --no-verify` when needed.

## Building

```bash
just build        # configure + build
just test         # build + run all tests
```

Use a tmp directory for faster dev builds:

```bash
just build_dir=/tmp/qeriplayer-build build
```

## Code Style

See [AGENTS.md](AGENTS.md) for naming conventions, coroutine style, commit format, and more.
