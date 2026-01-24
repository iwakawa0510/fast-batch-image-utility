# Git Commit Message Convention

This project follows a **simple, standard, and widely accepted** commit message convention.
The primary goals are:

- Easy history tracking
- Clear identification of bug fixes and undefined-behavior avoidance
- Compatibility with common OSS tooling and workflows

This document intentionally avoids cultural or language-specific assumptions.

---

## 1. Basic Format

```
<type>: <short summary>
```

- The **title line is mandatory**
- Use **ASCII characters only**
- Maximum length: **72 characters** (recommended ≤ 60)

### Examples

```
fix: avoid undefined behavior in pixel stride calculation
perf: reduce memory copies in batch conversion loop
refactor: simplify image loading error paths
```

---

## 2. Commit Types

Use one of the following lowercase types:

| Type | Purpose |
|---|---|
| `fix` | Bug fix, logic error correction, UB avoidance |
| `perf` | Performance improvement without behavior change |
| `refactor` | Code restructuring without functional change |
| `feat` | New user-visible functionality |
| `build` | Build system, toolchain, or dependency changes |
| `ci` | CI configuration changes |
| `docs` | Documentation only |
| `test` | Tests only |
| `chore` | Maintenance tasks not affecting runtime behavior |

---

## 3. Short Summary Rules

- Use **imperative mood** ("fix", not "fixed" or "fixes")
- Describe **what the commit does**, not how
- Be specific about the affected area

### Good

```
fix: prevent signed overflow in alpha conversion
perf: parallelize image decode using worker threads
```

### Avoid

```
fix: bug
update stuff
misc changes
```

---

## 4. Commit Body (Optional)

A longer explanation **may be added** after a blank line:

```
fix: clamp alpha values before LUT lookup

The previous implementation allowed negative values to propagate,
causing undefined behavior on some compilers.
```

Guidelines:

- Wrap lines at 72 characters
- Explain **why**, not just **what**
- Use English technical language

---

## 5. Bug Fixes and Undefined Behavior

When addressing bugs or UB:

- Prefer `fix:`
- Mention the **root cause** if known
- Mention affected platforms or compilers if relevant

Example:

```
fix: avoid UB from left-shift on signed integer
```

---

## 6. What This Convention Does NOT Do

- No semantic version enforcement
- No mandatory scopes
- No ticket or issue ID requirement
- No emojis or prefixes beyond `<type>`

This is a **deliberately minimal** convention.

---

## 7. Rationale

This convention is inspired by common practices used in:

- Linux kernel (simplified)
- LLVM / Clang
- FFmpeg
- Many small-to-medium OSS utilities

The focus is **clarity, grep-ability, and long-term maintainability**.

---

## 8. Summary

- One line, ASCII, imperative
- `<type>: <what changed>`
- Optional body for rationale
- Optimized for humans *and* tools

---

## 9. Enforcement

This convention applies to all commits starting from the revision where this document is added.
Previous history remains untouched.