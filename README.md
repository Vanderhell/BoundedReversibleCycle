# BoundedReversibleCycle

A bounded reversible cycle primitive for deterministic full-cycle traversal of integer domains in portable C17.

## Overview

BRC conjugates a bounded modular rotation by an internal reversible mapping. For the resulting bijection `Q`, the forward operation is `Q^-1((Q(x) + delta) mod N)`. The implementation is standalone and does not depend on the BRP repository.

## Properties

- A step `delta` coprime to `N` produces one cycle of length `N`.
- For `N > 1` and a non-coprime valid step, the cycle length is `N / gcd(N, delta)`; the domain is partitioned into that many cycles.
- Forward and exact inverse operations are available.
- Seeded initialization deterministically selects a coprime step. Explicit-step initialization accepts `delta = 0` for `N = 1`; for `N > 1`, it accepts any `1 <= delta < N`.
- Supports domains from 1 through `UINT16_MAX` or `UINT32_MAX`, respectively.
- Fixed-size caller-owned context; no heap, domain-sized table, or mutable global state.

This mathematical traversal primitive makes no burst-error dispersion guarantee and is not a cryptographic primitive.

## API

Include `brc.h`. Use `brc16_init_v1` or `brc32_init_v1` for deterministic seeded initialization, or `brc*_init_with_delta_v1` to select the step explicitly. Apply the matching `brc*_forward_v1` and `brc*_inverse_v1` functions. Keep the initialized context immutable. Invalid-input behavior is documented in the header.

## Example

[`examples/basic.c`](examples/basic.c) prints the first values of a seeded traversal. Build it as the `brc_basic` CMake target.

## Build

Requires CMake and a C17 compiler. CMake explicitly requests ISO C17 without compiler extensions.

```sh
cmake -S . -B build -DCMAKE_BUILD_TYPE=Release
cmake --build build
```

For a fresh build directory, choose `-DCMAKE_C_COMPILER=gcc` or `clang` to select a compiler.

## Test

```sh
ctest --test-dir build --output-on-failure
```

Tests cover inverse behavior, full-cycle uniqueness and closure through domain 4096, and frozen compatibility vectors in `tests/vectors_v1.csv`.

## Embedded characteristics

The production core is freestanding-compatible C17 with fixed-width integer types, caller-owned fixed-size contexts, no heap allocation, no O(N) table, no BRP dependency, and no mutable global state. A host object measurement with GCC 13.3 and Clang 18.1 on x86-64 Linux using `-Os -ffreestanding` reports `.text` sizes of 1225 and 1341 bytes, respectively, with `.data` and `.bss` both zero. These object-level figures are toolchain-specific and exclude caller-owned context storage.

## Algorithm notes

The step is reduced modulo the domain as part of the bounded rotation. Coprimality gives a single cycle; a shared factor divides the domain into shorter cycles. Interleaver quality is outside the primitive's correctness contract.

## Compatibility / stability

`BRC16-V1` and `BRC32-V1` define deterministic compatibility behavior, including seeded initialization and explicit-step handling. Frozen vectors are kept as compatibility oracles. Repository version `0.1.0` is separate from the algorithm version macros.

## License

No license has been selected yet.
