# Contributing to DRACHMA

Thank you for your interest in contributing to DRACHMA (DRM). The project prioritizes correctness, reviewability, and long-term maintainability—especially for consensus-critical code in **Layer 1**. Contributions are welcome from everyone, provided they follow the guidelines below.

## Ways to Contribute

- **Bug reports:** Use the issue templates and include reproduction steps, logs, and whether consensus behavior might be affected.
- **Feature requests:** Focus on usability, observability, testing, and tooling. Consensus changes are accepted only with broad review and clear motivation.
- **Pull requests:** Small, well-scoped PRs are easier to review and merge. Draft PRs are encouraged for early feedback.
- **Documentation:** Improvements to guides, specs, and diagrams are highly valued.

## Code of Conduct

By participating, you agree to abide by the [Code of Conduct](CODE_OF_CONDUCT.md).

## Development Workflow

1. Fork the repository and create a topic branch off `main`.
2. Keep commits **small**, **atomic**, and **well-described**. Avoid mixing refactors with behavior changes.
3. Ensure the code builds and tests pass locally. Add or update tests with every behavior change.
4. Submit a pull request using the provided template. Clearly mark whether consensus-critical code is touched.
5. Participate in review. Be prepared to revise; consensus and security reviews can be stringent.

## Review Expectations

- **Layer 1 (Consensus):** Requires multiple reviewers, explicit ACKs/NACKs, and thorough test coverage. Avoid sweeping refactors that obscure logic. Backward compatibility and determinism are mandatory.
- **Layer 2 (Services):** Changes must not alter consensus behavior. Focus on robustness, P2P safety, and DoS resistance.
- **Layer 3 (Desktop):** Prioritize UX clarity and user safety. Avoid adding implicit trust assumptions.
- **Scripts/Miners/Docs:** Ensure transparency and reproducibility. Avoid opaque binaries or vendor-specific dependencies without justification.

## Coding Style

- **Language Standard:** C++17 for core components. Use modern idioms conservatively and prefer clarity over cleverness.
- **Formatting:** Follow existing conventions; prefer clang-format where available. Keep line lengths reasonable (≤ 100 columns when practical).
- **Safety:** Avoid global mutable state. Handle errors explicitly; never swallow exceptions or errors silently. Do not wrap imports in try/catch blocks.
- **Dependencies:** Minimize new dependencies. Any new third-party library requires justification and license compatibility.

## Testing

- Add unit or integration tests alongside code changes (see `tests/`).
- Run the full test suite and relevant sanitizers when available:
  ```bash
  cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
  cmake --build build --parallel
  ctest --test-dir build --output-on-failure
  ```
- For networking or miner changes, include reproducible test vectors or pcap/log snippets when possible.

## Commit Guidelines

- Use descriptive commit messages (e.g., `layer1: tighten block header checks`).
- Reference issues when applicable: `Fixes #123`.
- Avoid commits that break bisectability or introduce formatting-only changes mixed with behavior changes.

## Security & Consensus Safety

- Treat consensus code as immutable unless there is strong, peer-reviewed rationale.
- Never introduce hidden parameters, soft-forks, or policy changes without documentation and broad review.
- Report vulnerabilities privately per [SECURITY.md](SECURITY.md); do not open public issues.

## Continuous Integration

- CI must pass before merge. If CI is failing for unrelated reasons, note it in the PR and provide local test evidence.

## Documentation Updates

- Update relevant docs (e.g., `docs/`, `README.md`, `scripts/`) when behavior changes.
- Keep diagrams and examples synchronized with code.

Thank you for helping make DRACHMA reliable, auditable, and fair.
