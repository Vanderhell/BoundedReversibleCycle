# Bounded Reversible Cycle

Native C17 implementation of BRC16-V1 and BRC32-V1. BRC conjugates a modular domain rotation by a private pair of BRP-style involution rounds. A coprime step gives one full cycle; explicit non-coprime steps produce multiple cycles. This repository is a mathematical traversal primitive and makes no burst-interleaver quality claim. Contexts are caller-owned and immutable after initialization; see `include/brc.h` for exact argument behavior.

Build and run tests with CMake and CTest. The core has no BRP dependency, heap use, or mutable global state.
