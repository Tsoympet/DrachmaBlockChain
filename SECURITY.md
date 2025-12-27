# Security Policy

We take security and consensus safety seriously. Please report suspected vulnerabilities **privately** and **do not** open public issues.

## Reporting a Vulnerability
- **Preferred contact:** security@drachma.org (request GPG fingerprint in your initial email; encrypted reports encouraged)
- **Alternate:** GitHub "Report a vulnerability" (Security tab) to open a private advisory with maintainers
- **Include:** impact description, reproduction steps, logs/vectors, affected versions/commit hashes, environment, and disclosure plans.

### Response Targets
- Acknowledgement within **3 business days**
- Triage update within **7 days**
- Fix/mitigation plan for critical issues within **30 days** (may defer public details to protect users)

## Bug Bounty Program
- **Status:** Active for consensus, networking, wallet, miners, and build/release pipelines (all tiers of the repo are in scope).
- **Rewards:**
  - Critical consensus faults (chain split, inflation) — up to **15,000 USD** equivalent in stablecoin.
  - Remote DoS on default-config nodes — up to **3,000 USD**.
  - Wallet key/seed exfiltration or signing-key compromise — up to **6,000 USD**.
- **Eligibility:** First-come disclosure with a working proof-of-concept; exploits of outdated dependencies without user impact are excluded.
- **Process:**
  1. Submit privately with "Bounty" in the subject and include a suggested CVSS or severity rating.
  2. Maintain embargo until coordinated release; payouts follow patch availability and maintainer validation.
  3. Optionally request public acknowledgment in release notes or remain anonymous.
  4. Payments can be settled in USD stablecoin or BTC at the recipient's preference.

## Scope
- Consensus logic, networking, wallet services, miners, build scripts, release artifacts, configuration defaults, and documentation that could mislead users in a way that impacts security.

### Out of Scope
- Third-party pools or forks not maintained here
- Hypothetical issues without actionable details
- Vulnerabilities requiring full host compromise with no network effect

## Audit Invitations
We welcome independent audits prior to each tagged release candidate. Auditors can open a private issue titled "Security Audit Request" to coordinate scope, timelines, sample binaries, and key exchange. Release candidates will publish binary hashes and detached GPG signatures to simplify verification.
