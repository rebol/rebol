# Security Policy

## Supported versions

Only the **latest release** and the current development branch receive security fixes. Older versions are not patched; please upgrade.

## Reporting a vulnerability

**Please do not open a public issue for security problems.**

Report privately via either:

- GitHub **[Private vulnerability reporting](https://github.com/Oldes/Rebol3/security/advisories/new)** (preferred), or
- email: `oldes.huhuman@gmail.com`

Please include:

- the Rebol3 version and build (`r3 --version`) and platform,
- a minimal reproducer: a Rebol script, plus any hostile input (inline as `#{...}` where possible),
- the exact command line, **including security flags** (`+s`, `-s`, `--secure ...`),
- what happened, and what you expected.

## What counts as a vulnerability

Rebol3 treats **Rebol code as trusted, except where restricted by `secure` policies**, and treats **data as untrusted**. In short:

- **In scope**:
  - memory corruption, out-of-bounds access or hangs caused by untrusted *data* (`load`, codecs, `decompress`, TLS/HTTP responses, …),
  - bypassing an active `secure` policy (e.g. writing a file or running a program the policy should have denied or asked about).
- **Out of scope**:
  - anything the active security policy allows,
  - crashes reachable only by deliberately writing unusual Rebol code (please report those as normal issues).

The full scope and severity guidance is in [`.oss-scanner/threat_model.md`](.oss-scanner/threat_model.md).

Vulnerabilities in vendored third-party code (e.g. mbedTLS) should also be reported upstream. Please mention the upstream advisory if one exists.

## What to expect

Rebol3 is maintained by a single volunteer, so handling is **best effort, with no fixed response time**. Typically:

1. Acknowledgement of your report.
2. Investigation and a fix in the development branch, with a regression test.
3. A release containing the fix, with credit to you in the release notes (unless you prefer otherwise).

Please allow reasonable time for a fix before public disclosure.