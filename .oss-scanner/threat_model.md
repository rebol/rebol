# Rebol3 — Threat Model

## What the project is

Rebol3 (Oldes/Rebol3) is an interpreter for the Rebol programming language, written in C. It ships as a standalone `r3` executable, plus optional native extensions (`.rebx`) that can be built embedded or as shared libraries. Rebol scripts can access files, run processes (`call`), use the network, read environment variables and load native extensions. Each of these is subject to the security policies described below.

## Security policies (`secure`)

Rebol3 has a policy system that guards access to host resources.

- **Targets**: `file`, `net`, `call`, `envr`, `browse`, `extension`, `protect`, `debug`, `secure`, plus the resource limits `eval` and `memory`.
- **Levels**: `allow`, `ask` (interactive prompt), `throw` (error), `quit` (exit). These can be set separately for read, write and execute.
- **Exceptions**: per-path (`file`) and per-URL (`net`). The longest matching path wins.
- **Without a TTY**, `ask` is denied automatically.
- **Startup**:
  - `-s` allows everything.
  - `+s` asks for everything.
  - `--secure <policy>` sets an explicit policy.
  - The default allows reading everywhere and asks before write or execute outside trusted folders. Trusted folders are the current directory, the script directory and the data directory. The home directory allows read and execute.
- **Protection of the policy store**: the store (`system/state/policies`) and the prompt function are hidden with `protect/hide`. Changing policies goes through `secure`, which is itself guarded by the `secure` target.

`secure` is documented as *not fully implemented*. It is a **policy boundary**, not a hardened sandbox.

## Trust boundary

There are two kinds of untrusted input:

1. **Untrusted data** processed by a trusted script.
2. **Untrusted Rebol code** running under a restrictive security policy (`+s`, `--secure`, or the default policy). Such code may do anything the active policy permits, and nothing more.

### In scope

**A. Security policy bypass.** Untrusted code performs a guarded action without the policy check being applied, or contrary to the active policy. Examples:

- A native, port actor, scheme, codec or mezzanine that reaches files, network, processes, environment variables, the browser or extensions without consulting the policy, or consults the wrong target or mode (e.g. checks read but performs write).
- Path tricks that make an access land outside the allowed exception, or inside a more permissive one. Examples: `..` segments, symlinks, case folding, trailing slashes, non-existent paths resolved differently at check time and use time, or prefix matching that treats `/data-evil/` as being under `/data/`.
- TOCTOU between the policy check and the actual open, e.g. a path re-resolved after the check.
- Changing policies without passing the `secure` check: reaching hidden `system/state/policies` or `confirm-policy`, rebinding, `unprotect`, reflection, or getting a reference to the internal `set-policies`.
- Escaping `protect`/`protect/hide` on system values that the security system relies on.
- Bypassing the `eval` or `memory` limits.
- Spoofing or manipulating the interactive security prompt, e.g. control or ANSI escape sequences in file names, URLs or command strings shown in the prompt, or pre-fed input that answers the prompt.

**B. Untrusted data.** A reasonable script processing hostile data must not be able to corrupt memory, read out of bounds, execute code, or hang the process indefinitely. Covered functions:

1. **`load` / `transcode`** on untrusted strings or binaries, i.e. the scanner/lexer, *without* evaluating the result. Includes construction syntax and all literal forms.
2. **Codecs** via `decode` / `load %file` (image, audio, archive, ASN.1/DER, X.509, key and text codecs built into the core).
3. **Compression**: `decompress` and archive extraction, including size headers that lie, and decompression bombs.
4. **Network protocols**: the TLS client (handshake, record layer, certificates), the HTTP/HTTPS client (headers, chunked encoding, redirects), DNS and other built-in schemes. A malicious server is the attacker. The vendored **mbedTLS** is in scope as integrated; upstream mbedTLS bugs should reference the upstream advisory.
5. **String and binary processing** on untrusted content: UTF-8 decoding, conversions, `enbase`/`debase`, `checksum`, `parse` with trusted rules on untrusted input, and `mold`/`form`.
6. **Path conversion** on untrusted names: `to-local-file`/`to-rebol-file`, and archive entry names that escape the target directory.

### Out of scope

- Anything untrusted code does that the **active policy allows**. With `-s`, or with `allow` set for a target, there is no boundary for that target. Under the default policy, reading any file is allowed by design.
- Memory-safety bugs reachable **only** by writing unusual Rebol code: invalid arguments to natives, deliberately corrupted series, deep recursion. These are welcome as normal bugs. If you can turn one into a *demonstrated* policy bypass, report it under A.
- Denial of service by resources the code explicitly requests, unless an `eval`/`memory` limit is set and bypassed.
- Behavior of native extensions after `extension` policy permission was granted.
- Third-party extensions not in this repository.
- Host code for platforms other than the scanned build (`rebol3-core-linux-x64`).
- Missing hardening flags or compiler options by themselves.

## Severity guidance

- **Critical**: code execution from untrusted *data* (B) with no special configuration, e.g. a malicious image, archive or TLS server.
- **High**:
  - A policy bypass (A) giving write or execute access (files, `call`, extensions), or silently modifying policies, under a policy that denies or asks for it.
  - An out-of-bounds write or use-after-free reachable from untrusted data.
  - A TLS certificate validation bypass.
- **Medium**:
  - A policy bypass giving read access, network access or environment access.
  - Prompt spoofing that misrepresents what is being approved.
  - An out-of-bounds read from untrusted data.
  - A hang or unbounded memory growth from small untrusted input.
  - Bypassing `eval`/`memory` limits.
- **Low**: assertion failures, NULL dereferences or undefined behavior from untrusted data with no demonstrated memory-safety impact. A guarded resource with no policy check at all, given that `secure` is documented as incomplete; still worth reporting.
- **Not a security issue**: actions permitted by the active policy, and crashes reachable only from crafted Rebol code without a demonstrated bypass.

Buffer overflows without a demonstrated exploit are capped at **High**.

## Reports and patches

- Include a minimal reproducer: a Rebol script, plus any hostile input inline as `#{...}`, plus the exact `r3` command line including the security flags used (`+s`, `--secure ...`).
- For policy bypasses, state the active policy, the guarded action that succeeded, and why the policy should have stopped it.
- Patches should be minimal, follow the existing C and Rebol style, and raise Rebol errors rather than calling `abort()`.
- Each patch must include a regression test in the Rebol test suite.
- One issue per report.