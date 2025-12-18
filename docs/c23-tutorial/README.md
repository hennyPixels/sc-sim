# Advanced C23 Systems Programming Tutorial

## A Professional Guide to Secure, High-Performance C

This comprehensive tutorial explores C23 through the PartsDB CLI codebase, synthesizing wisdom from fifteen foundational texts on systems programming, security, and optimization. Designed for professional environments with strict security requirements.

> **Target Audience**: Senior developers, security engineers, embedded systems programmers, and architects building mission-critical systems.

---

## Source Texts

| # | Book | Author(s) | Focus Area |
|---|------|-----------|------------|
| 1 | **Expert C Programming: Deep C Secrets** | Peter van der Linden | Compiler internals, memory layout, obscure language corners |
| 2 | **21st Century C** | Ben Klemens | Modern tooling, build systems, C99/C11 idioms |
| 3 | **Hacker's Delight** | Henry S. Warren Jr. | Bit manipulation, low-level arithmetic, optimization tricks |
| 4 | **Effective C** | Robert C. Seacord | Modern C11/C17/C23 best practices with security emphasis |
| 5 | **Secure Coding in C and C++** | Robert C. Seacord | Buffer overflows, integer vulnerabilities, defensive techniques |
| 6 | **C Traps and Pitfalls** | Andrew Koenig | Subtle language gotchas and common mistakes |
| 7 | **C Interfaces and Implementations** | David R. Hanson | Data structures, API design, reusable library patterns |
| 8 | **Modern C** | Jens Gustedt | C11/C17 features, atomics, threads, type-generic macros |
| 9 | **Computer Systems: A Programmer's Perspective** | Bryant & O'Hallaron | Memory hierarchy, caching, linking, performance optimization |
| 10 | **The Practice of Programming** | Kernighan & Pike | Style, debugging, testing, portability |
| 11 | **Writing Solid Code** | Steve Maguire | Defensive programming, assertions, compile-time bug detection |
| 12 | **Pointers on C** | Kenneth Reek | Pointer mechanics, arrays, dynamic allocation |
| 13 | **The C Puzzle Book** | Alan R. Feuer | Exercises and puzzles to deepen language intuition |
| 14 | **SEI CERT C Coding Standard** | Software Engineering Institute | Security rules and guidelines |
| 15 | **The C Programming Language (K&R)** | Kernighan & Ritchie | The canonical reference, C's design philosophy |

---

## Tutorial Parts

### Foundation
- [Part 1: Compiler Internals & Memory Layout](part-01-compiler-internals.md) *(Deep C Secrets, CS:APP)*
- [Part 2: Modern Tooling & Build Systems](part-02-modern-tooling.md) *(21st Century C)*

### Optimization
- [Part 3: Bit Manipulation & Low-Level Optimization](part-03-bit-manipulation.md) *(Hacker's Delight)*
- [Part 8: Memory Hierarchy, Caching & Linking](part-08-memory-hierarchy.md) *(CS:APP)*

### Security
- [Part 4: Modern C Best Practices & Security](part-04-security.md) *(Effective C, Secure Coding in C)*
- [Part 5: C Traps and Pitfalls](part-05-traps-pitfalls.md) *(C Traps and Pitfalls)*

### Architecture
- [Part 6: API Design & Reusable Libraries](part-06-api-design.md) *(C Interfaces and Implementations)*
- [Part 7: Concurrency, Atomics & Type-Generic Programming](part-07-concurrency.md) *(Modern C)*

### Professional Practice
- [Part 9: Style, Debugging & Portability](part-09-style-debugging.md) *(Practice of Programming)*
- [Part 10: Defensive Programming & Assertions](part-10-defensive-programming.md) *(Writing Solid Code)*
- [Part 11: Pointer Mechanics & Dynamic Allocation](part-11-pointer-mechanics.md) *(Pointers on C, C Puzzle Book)*

### Standards & Philosophy
- [Part 12: SEI CERT C Coding Standard](part-12-cert-c-standard.md) *(SEI CERT)*
- [Part 13: K&R Design Philosophy](part-13-kr-philosophy.md) *(K&R)*

### Appendices
- [Appendix A: Security Hardening Checklist](appendix-a-security-checklist.md)
- [Appendix B: Comprehensive Review Questions](appendix-b-review-questions.md)
- [Appendix C: Quick Reference](appendix-c-quick-reference.md)

---

## Reading Paths

**For Security Engineers:**
Parts 4 → 5 → 12 → 10 → Appendix A

**For Performance Engineers:**
Parts 3 → 8 → 1 → 11

**For API/Library Designers:**
Parts 6 → 7 → 9 → 13

**For Comprehensive Study:**
Read sequentially, completing review questions for each part.

---

## Code Examples

All code examples reference the PartsDB CLI codebase:
- `src/partsdb-cli/partsdb.h` - Public API header
- `src/partsdb-cli/database.c` - Implementation
- `src/partsdb-cli/main.c` - CLI interface

---

## Review Questions

Each part ends with review questions that:
- Have **no provided answers** - research required
- Reference specific books for deeper study
- Suggest PartsDB codebase expansions
- Test both theoretical knowledge and practical application

---

*This tutorial is part of the PartsDB CLI project. For build instructions, see the main README.*
