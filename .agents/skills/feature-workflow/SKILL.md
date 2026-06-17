---
name: feature-workflow
description: End-to-end feature workflow — explore, propose, implement with TDD, update docs, create PR. Use when the user wants to build a complete feature from scratch through to merge.
license: MIT
metadata:
  author: neriplayer
  version: "1.0"
---

Full feature workflow: explore → propose → implement (TDD) → docs → PR.

**Input**: A description of the feature to build. Can be vague — step 1 will clarify.

---

## Step 1: Explore

Understand the codebase and existing docs before proposing anything.

1. Read relevant source files in `src/` for the affected module(s).
2. Read existing specs in `openspec/specs/` for the affected area.
3. Read existing docs in `docs/modules/` for the affected area.
4. Check for related archived changes in `openspec/changes/archive/`.
5. Identify gaps, dependencies, and constraints.

**Output**: Brief summary of what exists, what's missing, and what the feature needs.

---

## Step 2: Propose

Create an OpenSpec change with proposal, design, specs, and tasks.

1. Derive a kebab-case change name from the feature description.
2. Create the change:
   ```bash
   openspec new change "<name>"
   ```
3. Generate all artifacts following the spec-driven schema:
   - **proposal.md** — what & why, with a "Non-goals" section
   - **design.md** — how, referencing actual class names from `src/`
   - **specs/** — one spec per affected component, with Requirements and Scenarios
   - **tasks.md** — grouped implementation tasks, each independently testable

4. Use `openspec instructions <artifact> --change "<name>" --json` for each artifact.
5. Follow `openspec/config.yaml` context and rules.

**Output**: All artifacts created. Show task count and summary.

---

## Step 3: Implement (TDD Loop)

For each **group** of related tasks in `tasks.md`:

### 3a. Write tests

Write test cases for the entire group **before** writing any implementation.

- Place tests in `tests/<module>/` following existing naming conventions.
- Test files: `Test<ClassName>.cpp` using Qt Test (`QTest::qCompare`, `QVERIFY`, etc.).
- Register test targets in `CMakeLists.txt` with `add_executable` + `add_test`.
- Tests should compile but may fail at this point (expected).

### 3b. Write implementation

Implement all tasks in the group to make the tests pass.

- Follow code conventions in `AGENTS.md` (naming, includes, coroutine style).
- Keep changes minimal and focused on the task group.
- Mark each task complete in `tasks.md`: `- [ ]` → `- [x]`

### 3c. Run tests

```bash
just test
```

All tests must pass. If a test fails, fix the implementation (not the test) unless the test itself is wrong.

### 3d. Run format

```bash
just format
```

### 3e. Commit

```bash
git add -A
git commit -m "<type>(<scope>): <subject>"
```

Follow conventional commits. Reference the change name in the commit body if helpful.

**Repeat** 3a–3e for each task group until all tasks are complete.

---

## Step 4: Update docs

Update documentation to match what was built.

1. Update the relevant module doc in `docs/modules/` (e.g., `core/database.md`, `api/netease.md`).
2. If new types were added, update or create `docs/modules/domain.md`.
3. If new repos were added, update `docs/modules/repo/index.md`.
4. Update `docs/TODO.md` — check off completed items.
5. Update `docs/modules/index.md` if the module map changed.
6. Ensure all class names, field names, and enum values in docs match the actual code.

```bash
just format
git add -A
git commit -m "docs: update docs for <feature>"
```

---

## Step 5: Create PR

1. Push the branch:
   ```bash
   git push -u origin <branch>
   ```

2. Create the PR:
   ```bash
   gh pr create --base main --title "<type>(<scope>): <subject>" --body "..."
   ```

3. PR body should include:
   - What the feature does
   - Which modules are affected
   - Link to the OpenSpec change (if applicable)
   - Testing notes

---

## Output

```text
## Feature Complete

**Change:** <name>
**Branch:** <branch>
**Tasks:** N/N complete ✓
**PR:** <url>

### Summary
- <what was built>
- <tests added>
- <docs updated>
```

---

## Guardrails

- **Always explore before proposing** — don't guess about existing code.
- **Tests before implementation** — TDD within each task group.
- **All tests must pass** before committing a group.
- **Format before committing** — `just format` then commit.
- **Docs must match code** — field names, enum values, class names.
- **Commit per task group** — not per task (too granular), not per feature (too coarse).
- **Pause if blocked** — unclear requirements, design issues, or test failures that need discussion.
- **Use existing tools** — `just test`, `just format`, `just check`, `openspec` CLI.
