---
name: review-fix
description: Review findings workflow — verify against codebase, fix valid issues, skip invalid ones with reasons, and validate each change with tests and formatting.
license: MIT
metadata:
  author: qeriplayer
  version: "1.0"
---

# Review-Fix Workflow

## Purpose

Verify a set of review findings against the current codebase, fix valid issues, skip invalid ones with reasons, and validate each change with tests and formatting.

## Review Focus Areas

Always scan for these categories, regardless of what triggered the review:

| Category | What to look for |
|----------|------------------|
| **Exception handling** | Repo/API calls without try-catch, fire-and-forget coroutines that can throw, catch blocks that swallow exceptions silently |
| **Logging** | `qWarning()` in non-core layers (should use `Logger::get()`), missing log statements in catch blocks, inconsistent log categories |
| **Database transactions** | Multi-statement writes without `beginTransaction`/`commitTransaction`, missing rollback in catch blocks, cross-repo operations that should be atomic |
| **Memory safety** | Raw `this` captures in coroutines/lambdas (use `QPointer`), discarded `QCoro::Task` return values, dangling pointers after `deleteLater()` |
| **Thread safety** | Shared mutable state accessed from coroutines, missing guards for cross-thread signal/slot connections |

## Workflow

### Phase 1: Analyze

1. **Read all referenced files** — specs, source, headers, tests, docs
2. **Verify each finding** against the actual current code (not assumptions)
3. **Classify each finding**:
   - ✅ Valid — issue exists in current code
   - ⏭️ Skip — already fixed, spec mismatch, needs larger refactor, or out of scope
   - 📝 Partial — can improve but full fix requires broader changes
4. **Record skip reasons** — brief, specific (e.g., "Transaction API not exposed via repo interfaces")

### Phase 2: Baseline

Before any changes:

```bash
just build   # confirm clean build
just test    # confirm all tests pass — this is the baseline
```

### Phase 3: Fix + Validate Loop

For each valid finding, in order of dependency (leaf changes first):

1. **Implement the fix** — minimal, targeted change
2. **Build** — `just build`
3. **Test** — `just test` (full suite, not just affected tests)
4. **Commit** — one commit per logical fix with conventional commit message

Commit message format:
```
<type>(<scope>): <imperative description>

<body explaining what and why, not how>
```

### Phase 4: Regression Tests

After all code fixes, add regression tests for:

- Bug fixes that could regress (null checks, bounds validation, race conditions)
- Behavioral changes (validation logic, state resets)
- Skip tests for: doc-only changes, trivial fixes, already-covered behavior

Test naming: `<method>_<scenario>` (e.g., `setPlayingIndex_invalidIndex`)

### Phase 5: Format & Final Validation

```bash
just format-against <base-ref>   # fix any violations
just check-against <base-ref>    # verify clean
just test                        # final full suite run
git commit                       # commit formatting if needed
```

## Key Principles

- **Test before fix** — establish baseline to catch regressions
- **One commit per fix** — clean history, easy to review/revert
- **Fix leaf dependencies first** — e.g., fix SongListModel before ViewModels that use it
- **Skip with reasons** — don't silently drop findings; explain why
- **Minimal changes** — fix the reported issue, don't refactor adjacent code
- **Validate everything** — build + test after each change, format at the end

## Commit Types

| Type | When |
|------|------|
| `fix` | Bug fix, null check, validation, race condition |
| `style` | Formatting only (clang-format) |
| `test` | Adding/updating tests |
| `docs` | Documentation corrections |
| `build` | Build system / tooling changes |

## Example Skip Reasons

| Reason | Example |
|--------|---------|
| Already fixed | "Code already handles this case" |
| Spec mismatch | "Spec says QString, matches code — spec comment not code bug" |
| Needs larger refactor | "Transaction API not exposed via repo interfaces" |
| Out of scope | "Complex feature requiring API response structure understanding" |
| Intentional TODO | "TODO is intentional, ServiceLocator exists but integration pending" |
