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
- **Status:** Active for consensus, networking, wallet, miners, and build/release pipelines.
- **Rewards:**
  - Critical consensus faults (chain split, inflation) — up to **10,000 USD** equivalent in stablecoin.
  - Remote DoS on default-config nodes — up to **2,000 USD**.
  - Wallet key/seed exfiltration — up to **5,000 USD**.
- **Eligibility:** First-come disclosure, must provide working proof-of-concept; exploits of outdated dependencies without impact are excluded.
- **Process:**
  1. Submit privately with "Bounty" in the subject.
  2. Maintain embargo until coordinated release; payment follows patch availability and validation.
  3. Optionally request public acknowledgment in release notes or remain anonymous.

## Scope
- Consensus logic, networking, wallet services, miners, build scripts, release artifacts, configuration defaults, and documentation that could mislead users in a way that impacts security.

### Out of Scope
- Third-party pools or forks not maintained here
- Hypothetical issues without actionable details
- Vulnerabilities requiring full host compromise with no network effect

## Audit Invitations
We welcome independent audits prior to each tagged release candidate. Auditors can open a private issue titled "Security Audit Request" to coordinate scope, timelines, sample binaries, and key exchange. Release candidates will publish binary hashes and detached GPG signatures to simplify verification.
