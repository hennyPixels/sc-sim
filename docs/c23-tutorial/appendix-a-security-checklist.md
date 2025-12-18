# Appendix A: Security Hardening Checklist

## Pre-Deployment Security Checklist

Use this checklist before deploying any C application to production.

### A.1 Compiler and Build Configuration

- [ ] **Enable all warnings**: `-Wall -Wextra -Wpedantic`
- [ ] **Treat warnings as errors**: `-Werror` (at least in CI)
- [ ] **Enable format string checking**: `-Wformat=2 -Wformat-security`
- [ ] **Enable stack protection**: `-fstack-protector-strong`
- [ ] **Enable position-independent code**: `-fPIE -pie`
- [ ] **Enable RELRO**: `-Wl,-z,relro,-z,now` (full RELRO)
- [ ] **Enable FORTIFY_SOURCE**: `-D_FORTIFY_SOURCE=2`
- [ ] **Disable executable stack**: `-Wl,-z,noexecstack`
- [ ] **Enable Control Flow Integrity** (Clang): `-fsanitize=cfi`
- [ ] **Strip symbols in release builds**: `-s`

### A.2 Memory Safety

- [ ] **No buffer overflows**: All array accesses bounds-checked
- [ ] **No use-after-free**: All freed pointers set to NULL
- [ ] **No double-free**: Ownership model clearly defined
- [ ] **No memory leaks**: All allocations freed on all paths
- [ ] **No uninitialized reads**: All variables initialized before use
- [ ] **Safe string handling**: Using bounded functions (snprintf, strncpy)
- [ ] **Integer overflow protection**: Checked arithmetic for untrusted input
- [ ] **Proper alignment**: No unaligned memory access

### A.3 Input Validation

- [ ] **Validate all external input**: Command line, files, network, environment
- [ ] **Whitelist validation**: Accept known-good, reject everything else
- [ ] **Length limits enforced**: Maximum size for all inputs
- [ ] **Character validation**: Only expected character sets accepted
- [ ] **Encoding validation**: UTF-8 properly validated if used
- [ ] **Path traversal prevention**: Canonicalize and validate file paths
- [ ] **SQL injection prevention**: Parameterized queries only
- [ ] **Command injection prevention**: No system(), use exec family

### A.4 Format String Security

- [ ] **No user input in format strings**: Always use `printf("%s", user_input)`
- [ ] **Format string warnings enabled**: `-Wformat-security`
- [ ] **Audit all printf-family calls**: fprintf, sprintf, snprintf, syslog
- [ ] **Use `__attribute__((format))` on custom formatters**

### A.5 Integer Security

- [ ] **Signed/unsigned handling**: Explicit conversions with range checks
- [ ] **Overflow detection**: Check before operations, not after
- [ ] **Size type consistency**: Use size_t for sizes, ptrdiff_t for differences
- [ ] **Avoid truncation**: Check range before narrowing conversions
- [ ] **No negative to unsigned**: Validate non-negative before conversion

### A.6 Cryptographic Security

- [ ] **No custom crypto**: Use established libraries (OpenSSL, libsodium)
- [ ] **Secure random numbers**: Use /dev/urandom or equivalent, not rand()
- [ ] **Constant-time comparisons**: For secrets and passwords
- [ ] **Key zeroization**: Overwrite secrets before freeing
- [ ] **No ECB mode**: Use authenticated encryption (GCM, ChaCha20-Poly1305)
- [ ] **Proper IV/nonce handling**: Never reuse with same key

### A.7 File and I/O Security

- [ ] **Check all return values**: fopen, fread, fwrite, fclose
- [ ] **Secure file permissions**: Use appropriate modes (0600 for sensitive files)
- [ ] **No TOCTOU races**: Open files once, use file descriptors
- [ ] **Temporary file safety**: Use mkstemp(), not mktemp()
- [ ] **Symlink protection**: Use O_NOFOLLOW where appropriate
- [ ] **Resource limits**: Limit file sizes and counts

### A.8 Concurrency Security

- [ ] **Thread safety**: Document and enforce thread-safety guarantees
- [ ] **No data races**: Use proper synchronization
- [ ] **Deadlock prevention**: Consistent lock ordering
- [ ] **Atomic operations**: Use C11 atomics for shared data
- [ ] **Signal handler safety**: Only async-signal-safe functions in handlers

### A.9 Error Handling

- [ ] **All errors handled**: No ignored return values
- [ ] **Fail securely**: Deny access on error, don't expose information
- [ ] **Meaningful error messages**: For debugging, not for attackers
- [ ] **No sensitive data in errors**: Don't leak paths, keys, or internal state
- [ ] **Logging security events**: Authentication failures, access violations

### A.10 Static Analysis

Run these tools and fix all findings:

- [ ] **Clang Static Analyzer**: `scan-build make`
- [ ] **Cppcheck**: `cppcheck --enable=all src/`
- [ ] **Clang-Tidy**: `clang-tidy -checks='*' src/*.c`
- [ ] **PVS-Studio** (if available)
- [ ] **Coverity** (if available)

### A.11 Dynamic Analysis

Test with these sanitizers:

- [ ] **AddressSanitizer**: `-fsanitize=address` (memory errors)
- [ ] **UndefinedBehaviorSanitizer**: `-fsanitize=undefined`
- [ ] **ThreadSanitizer**: `-fsanitize=thread` (data races)
- [ ] **MemorySanitizer**: `-fsanitize=memory` (uninitialized reads)
- [ ] **Fuzzing**: AFL, libFuzzer, or similar

### A.12 Documentation

- [ ] **Security architecture documented**: Trust boundaries, threat model
- [ ] **API security contracts**: Preconditions, postconditions, ownership
- [ ] **Known limitations documented**: What the code doesn't protect against
- [ ] **Incident response plan**: What to do if vulnerability is found

---

## Quick Reference: Secure Function Alternatives

| Unsafe | Safe Alternative | Notes |
|--------|------------------|-------|
| `gets()` | `fgets()` | Never use gets() |
| `strcpy()` | `strncpy()` + null | Or `strlcpy()` if available |
| `strcat()` | `strncat()` + size | Or `strlcat()` if available |
| `sprintf()` | `snprintf()` | Always pass buffer size |
| `scanf("%s")` | `scanf("%Ns")` | Specify maximum width |
| `system()` | `exec*()` family | Avoid shell entirely |
| `mktemp()` | `mkstemp()` | Creates file atomically |
| `rand()` | `arc4random()` / `getrandom()` | Cryptographic quality |
| `memcpy()` (overlap) | `memmove()` | Handles overlap |

---

## Minimum Secure Compiler Flags

```makefile
# GCC/Clang security flags
CFLAGS = -Wall -Wextra -Werror -Wpedantic \
         -Wformat=2 -Wformat-security \
         -Wconversion -Wsign-conversion \
         -Wstack-protector \
         -D_FORTIFY_SOURCE=2 \
         -fstack-protector-strong \
         -fPIE

LDFLAGS = -pie \
          -Wl,-z,relro,-z,now \
          -Wl,-z,noexecstack

# Debug build (with sanitizers)
DEBUG_FLAGS = -g -O0 \
              -fsanitize=address,undefined \
              -fno-omit-frame-pointer

# Release build
RELEASE_FLAGS = -O2 -DNDEBUG -s
```
