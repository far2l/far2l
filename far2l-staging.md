---
type: Concept
title: Fork far2l/far2l — Goals and Rules
description: Fork maintenance guide — goals, branch model, three-phase workflow, and commit conventions
tags: [fork, workflow, git, far2l]
timestamp: 2026-06-30T00:00:00Z
---

# Fork far2l/far2l — Goals and Rules

The `far2l/far2l` fork (GitHub: `far2l/far2l`, push remote `far2l`)
maintains custom work on top of the upstream `elfmz/far2l` (remote `origin`).

## Fork Goals

- **Maintain additional functionality** on top of `elfmz/far2l` without
  permanent divergence: `master` tracks `origin/master`, `staging` accumulates
  custom changes and tests, `release` stabilizes a snapshot for packaging.
- **Quality control**: regression and smoke tests (see `testing/README.md`),
  CI, static analysis (CodeRabbit, SonarQube).
- **Respect authorship**: every ported change preserves the original author's
  `Author`/`AuthorDate`; the fork adds a `Signed-off-by` trailer.

## Repositories and Branches

| Role    | Remote   | Branch          | Purpose                                        |
|---------|----------|-----------------|------------------------------------------------|
| Upstream| `origin` | `origin/master` | `elfmz/far2l` — source of truth                |
| Fork    | `far2l`  | `far2l/master`  | Mirror of `origin/master` for receiving PRs    |
| Working | local    | `staging`       | Accumulator of changes and tests atop master   |
| Release | `far2l`  | `release`       | Stabilized snapshot of `staging` for packaging |

> `far2l/master` and `origin/master` are kept identical; divergence is allowed
> only while a PR is being received (phase 1) and is resolved in phase 3.

## Workflow

### Phase 1 — Merge PR into master

1. An external (or internal) PR targets `far2l/master`.
2. The PR is reviewed and merged into `far2l/master` via GitHub (merge-commit).
3. Commit authorship stays with the original author; the fork **does not
   rewrite** `Author`/`AuthorDate`.

### Phase 2 — Analysis, port to staging, custom changes and tests

1. Analyze the merged commit: scope of changes, dependencies, potential
   regressions.
2. Port the change into `staging` (cherry-pick or a topic branch later merged
   into `staging`):
   ```sh
   git checkout staging
   git cherry-pick -x <commit-sha>
   ```
   The `-x` flag records a reference to the original commit in the message.
3. Add custom changes and regression tests (see `testing/README.md` and
   `CODESTYLE.md`). Build and test:
```sh
cmake -B build -DCMAKE_BUILD_TYPE=Debug -DTESTING=Yes
cmake --build build -j$(nproc)
testing/far2l-smoke-run.sh --all build/install/far2l
```
4. Commits for custom changes follow Conventional Commits (see "Commit
   Conventions" below).

### Phase 3 — Sync with elfmz/master

Goal: `master = elfmz/master`, `staging` rebased onto the new `master`.

1. Update upstream and align the fork branches:
```sh
git fetch origin
git checkout master
git reset --hard origin/master          # master = elfmz/master
git push far2l master --force-with-lease

git checkout far2l-master               # mirror for receiving future PRs
git reset --hard origin/master
git push far2l far2l-master --force-with-lease
```

2. Rebase `staging` onto the updated `master`:
```sh
git checkout staging
git rebase master --signoff
```
Conflicts are resolved iteratively (`git rebase --continue`); only the
fork-specific part is rewritten, upstream commits stay untouched.

3. Update push remotes as needed:
```sh
git push far2l staging --force-with-lease
```
4. Verify build and tests after the rebase (commands from phase 2, step 3).

### Release Branch

`release` is a stabilized snapshot of `staging` used for packaging and
distribution. It is cut from `staging` after phase 2 and phase 3 are complete
and the full test suite passes.

1. Ensure `staging` is rebased on the latest `master` (phase 3 done) and all
   tests pass.
2. Cut the release branch from `staging`:
```sh
git checkout staging
git checkout -b release
git push far2l release --force-with-lease
```
3. Only release-critical fixes (e.g., `fix:`) are cherry-picked onto
   `release` after the cut; feature commits stay on `staging`.
4. Tag the release:
```sh
git tag -a v<version> -m "far2l fork v<version>"
git push far2l v<version>
```
5. After release, `release` is not rebased on `master`; the next release is cut
   fresh from the updated `staging`.

## Commit Conventions

- Format: **Conventional Commits** (`feat`, `fix`, `docs`, `refactor`, `test`,
  `chore`, `build`, `ci`, `perf`, `style`, `revert`). Subject ≤ 72 characters,
  imperative mood, no trailing period.
- Scope follows the subsystem (`tty`, `netrocks`, `editor`, `tests`, `ci` …).
- For commits ported from `far2l/master`, use `git cherry-pick -x` and **add**
  trailers while preserving the original author:
  ```
  Signed-off-by: <Original Author> <original@example.com>
  Signed-off-by: Mikhail Lukashov <michael.lukashov@gmail.com>
  ```
  The first `Signed-off-by` is the commit's original author (certifying their
  authorship); the second is the fork maintainer. `Author`/`AuthorDate` are not
  changed.
- Amending the last commit: `git commit --amend --no-edit` (keeps the message
  unchanged).

## Quality Control

- Code style: `CODESTYLE.md`; architecture notes: `HACKING.md`.
- Tests: `testing/README.md`; build with `-DTESTING=Yes`, run
  `testing/far2l-smoke-run.sh --all`.
- CI: Github Actions
- Default test screen: 80×25.
