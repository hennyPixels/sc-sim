# Appendix B: Comprehensive Review Questions

These questions require searching through the source books for answers.
No answers are provided—use this as a study guide and research exercise.

## Part 1: Compiler Internals & Memory Layout

### From "Expert C Programming" (van der Linden)

1. What is the "bus error" that can occur on SPARC systems when accessing misaligned
   data? How does x86 handle the same situation differently?

2. Explain the difference between a "declaration" and a "definition" in C. Give an
   example of a symbol that can be declared multiple times but defined only once.

3. What is "interpositioning" and how can it be used to replace library functions?
   What are the security implications?

4. Describe the "clockwise spiral rule" for reading complex C declarations. Apply it
   to: `char *(*(*fp)(int))[10]`

5. What happens in memory when you call `setjmp()` and `longjmp()`? Why is this
   mechanism dangerous and when might you use it anyway?

### From "CS:APP" (Bryant & O'Hallaron)

6. Explain the difference between `.data`, `.bss`, and `.rodata` sections. Why does
   `.bss` not take up space in the object file?

7. What is ASLR and how does it improve security? What attacks does it prevent and
   what attacks can still succeed despite ASLR?

8. Describe the x86-64 calling convention. Which registers are caller-saved vs
   callee-saved? How are arguments passed?

9. What is the "red zone" on x86-64? Why does it exist and when is it safe to use?

10. Explain how buffer overflow attacks work at the assembly level. What is a "return
    address" and how can it be exploited?

## Part 2: Modern Tooling & Build Systems

### From "21st Century C" (Klemens)

11. What are the advantages of using `pkg-config` in build systems? How does it
    simplify cross-platform compilation?

12. Compare Makefile-based builds with CMake and Meson. What problems do CMake and
    Meson solve that Make doesn't?

13. What compiler flags does GCC's `-Wall` enable? List five specific warnings and
    explain what each detects.

14. How do you use Valgrind to detect memory leaks? What other tools does the
    Valgrind suite include?

15. What is "ccache" and how can it speed up compilation? What are its limitations?

## Part 3: Bit Manipulation & Low-Level Optimization

### From "Hacker's Delight" (Warren)

16. Explain how `x & (x - 1)` clears the lowest set bit. Why does this work?
    Use this to count the number of 1 bits in a word.

17. What is the "population count" (popcount) operation? Describe at least three
    different algorithms for computing it and their trade-offs.

18. How can you compute the absolute value of an integer without branching?
    Why might branchless code be faster?

19. Explain the bit manipulation trick for computing `floor(log2(x))` without
    using floating-point operations.

20. What is "bit reversal" and when is it needed? Describe an efficient algorithm
    for reversing the bits in a 32-bit word.

## Part 4: Security Best Practices

### From "Effective C" (Seacord)

21. What are the "annex K bounds-checking interfaces" (e.g., `strcpy_s`)? Why were
    they added and why are they controversial?

22. Explain the difference between "undefined behavior" and "implementation-defined
    behavior." Give examples of each and explain why the distinction matters.

23. What is the `restrict` keyword and how does it help the compiler optimize?
    When is it safe to use and when might it cause bugs?

24. How do C11 atomics differ from volatile? When should you use each?

25. What are the rules for "sequence points" in C? Why do expressions like
    `i = i++` have undefined behavior?

### From "Secure Coding in C" (Seacord)

26. What is an "integer overflow attack"? Describe a real-world vulnerability
    that exploited integer overflow and how it could be prevented.

27. Explain format string vulnerabilities. How can `%n` be exploited, and what
    mitigations exist?

28. What is a "heap overflow" attack? How does it differ from a stack buffer
    overflow?

29. Describe the "double-free" vulnerability. How can it lead to code execution?

30. What is "race condition" in the context of security? Give an example of a
    TOCTOU (time-of-check-to-time-of-use) vulnerability.

## Part 5: Common Pitfalls & Traps

### From "C Traps and Pitfalls" (Koenig)

31. Why does `if (x = 0)` compile without error in most compilers? How can this
    be caught?

32. Explain the difference between `sizeof(array)` and `sizeof(pointer)` when
    an array is passed to a function.

33. What is the "dangling else" problem? How does C resolve it and how can it
    lead to bugs?

34. Why is `#define SQUARE(x) x*x` dangerous? What inputs will give wrong results?

35. Explain why `"abc" == "abc"` might be true or false depending on the compiler.

## Part 6: API Design & Reusable Libraries

### From "C Interfaces and Implementations" (Hanson)

36. What is an "opaque type" and why is it useful for API design? How do you
    implement one in C?

37. Describe the "first-class ADT" pattern. How does it differ from a simple
    struct definition?

38. What are the trade-offs between returning error codes vs. using a global
    error state (like errno)?

39. Explain the "arena allocation" pattern. When is it appropriate and what
    are its limitations?

40. How do you handle versioning and ABI stability in a C library? What changes
    break binary compatibility?

## Part 7: Concurrency & Modern C

### From "Modern C" (Gustedt)

41. Explain the C11 memory model. What are "memory orders" and when would you use
    each one (relaxed, acquire, release, seq_cst)?

42. What is a "data race" according to the C standard? How does it differ from
    a "race condition"?

43. Describe the `_Generic` selection mechanism. How does it enable function
    overloading in C?

44. What are "compound literals" and how can they simplify code? Give examples
    showing their use.

45. Explain "designated initializers." How do they improve code clarity and
    what are their limitations?

## Part 8: Systems Programming

### From "CS:APP" (Bryant & O'Hallaron)

46. Explain the memory hierarchy (registers, L1/L2/L3 cache, main memory, disk).
    What are typical access times for each level?

47. What is "spatial locality" and "temporal locality"? How do you write code
    that exhibits good locality?

48. Describe how dynamic linking works. What is the GOT (Global Offset Table)
    and PLT (Procedure Linkage Table)?

49. What is a "position-independent executable" (PIE)? How does it work and
    why is it important for security?

50. Explain the virtual memory system. What is a page table and how does
    address translation work?

## Part 9: Style & Debugging

### From "The Practice of Programming" (Kernighan & Pike)

51. What are Kernighan and Pike's rules for choosing variable names? How do
    scope and name length relate?

52. Describe their approach to debugging. What is "scientific debugging"?

53. How do they recommend handling errors in C? Compare this to exception-based
    error handling.

54. What testing strategies do they recommend? How do you test edge cases
    effectively?

55. Explain the "prototype and evolve" development approach. How does it
    compare to "big design up front"?

## Part 10: Defensive Programming

### From "Writing Solid Code" (Maguire)

56. What is Maguire's philosophy on assertions? When should you assert vs.
    when should you handle errors?

57. Describe the "subsystem integrity" checking pattern. How do you implement
    invariant checking?

58. What are "sentinel values" in memory debugging? How do they help detect
    buffer overruns?

59. Explain the concept of "defensive copying." When is it necessary and when
    is it wasteful?

60. How should you handle unexpected conditions that "can never happen"? What
    does Maguire recommend?

## Part 11: Pointer Mechanics

### From "Pointers on C" (Reek)

61. What is the relationship between arrays and pointers in C? When does an
    array "decay" to a pointer?

62. Explain multi-level indirection. When is `**p` or `***p` needed?

63. What are function pointers? Give three practical use cases.

64. Describe the relationship between pointers and const. What does each
    combination mean: `const int*`, `int const*`, `int* const`?

65. How do you implement a generic data structure using void pointers?
    What type safety is lost?

### From "The C Puzzle Book" (Feuer)

66. What does `*p++` do? What about `(*p)++`? Explain the precedence.

67. Why does `a[i]` equal `i[a]` in C? What does this reveal about array
    subscripting?

68. Explain what happens with: `char *s = "hello"; s[0] = 'H';`

69. What is pointer aliasing? Why do compilers need to consider it?

70. Describe the behavior of `p = p + n` where p is a pointer. How does the
    type of p affect this?

## Part 12: CERT C Rules

### From "SEI CERT C Coding Standard"

71. Why does CERT distinguish between "rules" and "recommendations"? Give an
    example of each and explain the difference.

72. Explain INT32-C (signed integer overflow). Why is signed overflow undefined
    behavior while unsigned overflow is defined?

73. What is ARR30-C about? Why is accessing one past the end of an array
    sometimes valid?

74. Describe MEM30-C and MEM31-C. How do you prevent both use-after-free and
    memory leaks?

75. What does FIO30-C prohibit? Explain the attack that motivates this rule.

## Part 13: K&R Philosophy

### From "The C Programming Language" (K&R)

76. What does K&R mean by "trust the programmer"? What are the benefits and
    drawbacks of this philosophy?

77. Study K&R's malloc implementation. How does the free list work? What is
    memory coalescing?

78. Explain the K&R style of code formatting. How does it differ from other
    common styles?

79. What text processing idioms does K&R emphasize? Why is text so central
    to UNIX philosophy?

80. How has C evolved since K&R? List the major additions in C89, C99, C11,
    C17, and C23.

---

## Synthesis Questions

These questions integrate concepts from multiple books:

81. Design a memory-safe string library that prevents buffer overflows (CERT),
    uses opaque types (Hanson), and includes debug assertions (Maguire).

82. Implement a thread-safe hash table using C11 atomics (Modern C), with
    proper error handling (K&R), and defensive checks (Writing Solid Code).

83. Write a parser for a simple configuration format that handles all edge
    cases (Practice of Programming), validates input (CERT), and reports
    meaningful errors.

84. Design an API for PartsDB that demonstrates information hiding (Hanson),
    secure coding practices (Seacord), and K&R-style simplicity.

85. Create a memory allocator that uses bit manipulation for efficient block
    management (Hacker's Delight), includes debugging support (Maguire), and
    handles fragmentation (K&R malloc).
