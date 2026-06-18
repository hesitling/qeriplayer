---
name: feature-workflow
description: End-to-end feature workflow — explore, propose, implement with TDD, update docs, create PR. Use when the user wants to build a complete feature from scratch through to merge.
license: MIT
metadata:
  author: qeriplayer
  version: "2.0"
---

PR-based feature workflow: explore → propose → plan PRs → implement PR → create PR → repeat.

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

## Step 3: Plan PRs

Group tasks into reviewable PRs. Each PR must be a focused, logical unit of change.

### PR Sizing Guidelines

| Criteria | Target |
|----------|--------|
| Lines changed | 200-400 (excluding generated code) |
| Files touched | 5-10 |
| Logical changes | 1 |
| Review time | 15-30 minutes |

### Grouping Rules

1. **One logical change per PR** — a PR should be describable in a single sentence.
2. **Dependencies flow forward** — PR N+1 depends on PR N, not the reverse.
3. **Each PR is independently testable** — tests pass after each PR merges.
4. **Infrastructure before features** — core/utility changes come first.
5. **API before UI** — data layer before presentation layer.

### Splitting Strategy

If tasks would create a PR that's too large:

- Split by module boundary (e.g., core changes vs. API changes)
- Split by layer (e.g., data model → repository → service → UI)
- Split by functionality (e.g., CRUD operations: create → read → update → delete)

### Output

Update `tasks.md` with PR groupings:

```markdown
## PR 1: <title>
- [ ] Task A
- [ ] Task B

## PR 2: <title> (depends on PR 1)
- [ ] Task C
- [ ] Task D
```

Show PR plan summary before proceeding.

---

## Step 4: Implement PR (TDD Loop)

For the current PR's task group:

### 4a. Create branch

```bash
git checkout -b feat/<change>/pr-<n>
```

### 4b. Write tests

Write test cases for the entire PR **before** writing any implementation.

- Place tests in `tests/<module>/` following existing naming conventions.
- Test files: `Test<ClassName>.cpp` using Qt Test (`QTest::qCompare`, `QVERIFY`, etc.).
- Register test targets in `CMakeLists.txt` with `add_executable` + `add_test`.
- Tests should compile but may fail at this point (expected).

### 4c. Write implementation

Implement all tasks in the PR to make the tests pass.

- Follow code conventions in `AGENTS.md` (naming, includes, coroutine style).
- Keep changes minimal and focused on the PR's scope.
- Mark each task complete in `tasks.md`: `- [ ]` → `- [x]`

### 4d. Run tests

```bash
just test
```

All tests must pass. If a test fails, fix the implementation (not the test) unless the test itself is wrong.

### 4e. Run format

```bash
just format
```

### 4f. Commit

```bash
git add -A
git commit -m "<type>(<scope>): <subject>"
```

Follow conventional commits. Reference the change name and PR number in the commit body.

---

## Step 5: Create PR

1. Push the branch:
   ```bash
   git push -u origin feat/<change>/pr-<n>
   ```

2. Create the PR:
   ```bash
   gh pr create --base main --title "<type>(<scope>): <subject>" --body "..."
   ```

3. PR body should include:
   - What this PR does (one sentence)
   - Which modules are affected
   - Link to the OpenSpec change
   - Part N of M for this feature
   - Testing notes

4. Add to PR body:
   ```markdown
   ## Part of: <feature name>
   This is PR <N> of <M>. See [change spec](openspec/changes/<name>/).

   ## What
   <one sentence summary>

   ## Modules affected
   - `<module>`

   ## Testing
   <how to verify>
   ```

---

## Step 6: Repeat or Finalize

### If more PRs remain

1. Merge the current PR (or wait for review).
2. Return to **Step 4** for the next PR group.

### If all PRs complete

1. Update documentation:
   - Update relevant module docs in `docs/modules/`
   - Update `docs/TODO.md` — check off completed items
   - Update `docs/modules/index.md` if the module map changed
   - Ensure all class names, field names, and enum values in docs match the actual code

2. Archive the change:
   ```bash
   openspec archive change "<name>"
   ```

3. Final commit:
   ```bash
   git add -A
   git commit -m "docs: update docs for <feature>"
   ```

---

## Output

```text
## Feature Complete

**Change:** <name>
**PRs:** <N> PRs created
**Tasks:** N/N complete ✓

### PR Summary
| PR | Title | Status |
|----|-------|--------|
| 1  | <title> | ✓ Merged |
| 2  | <title> | ✓ Merged |
| 3  | <title> | ⏳ Pending review |

### What was built
- <summary>

### Tests added
- <test files>

### Docs updated
- <doc files>
```

---

## Guardrails

- **Always explore before proposing** — don't guess about existing code.
- **Plan PRs before implementing** — know the full scope before starting.
- **One logical change per PR** — resist the urge to bundle.
- **Tests before implementation** — TDD within each PR.
- **All tests must pass** before committing a PR.
- **Format before committing** — `just format` then commit.
- **Docs must match code** — field names, enum values, class names.
- **Pause if blocked** — unclear requirements, design issues, or test failures that need discussion.
- **Use existing tools** — `just test`, `just format`, `just check`, `openspec` CLI.
