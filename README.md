# High-Frequency Limit Order Book (LOB)

A sub-microsecond matching engine implemented in C++17, architected for low-latency electronic trading. This project demonstrates core quantitative engineering principles, including zero-allocation memory management, custom intrusive data structures, and kernel-level socket programming.

## Core Architecture

To achieve extreme deterministic performance, this engine bypasses standard library abstractions in favor of highly optimized, custom memory structures that guarantee $O(1)$ or $O(\log n)$ operations for all critical path executions.

* **Template Memory Pool:** Eliminates all runtime OS heap allocations (`new`/`delete`) during live trading. A massive block of `Order` memory is pre-allocated at startup, guaranteeing $O(1)$ allocation and deallocation via pointer arithmetic.
* **Intrusive Doubly Linked List:** Orders act as their own nodes in the Price-Time priority queue. This enables $O(1)$ cancellations and modifications without the latency spikes associated with heap scanning.
* **Red-Black Tree Price Levels:** Buy and Sell price levels are managed via `std::map`, ensuring $O(\log n)$ price discovery and matching. 
* **Pre-reserved Hash Maps:** $O(1)$ order ID lookups using `std::unordered_map`, with all buckets pre-reserved at startup to physically prevent the engine from pausing to rehash during peak market volatility.
* **POSIX TCP Socket Ingest:** A raw kernel-level socket listener paired with a zero-allocation C-style parser (`sscanf`) for parsing incoming binary-style network strings.

## Performance Benchmark

Tick-to-trade latency is measured using the `<chrono>` high-resolution hardware clock. The benchmark captures the exact time elapsed from the moment a network packet is parsed to the moment the internal engine completes the matching execution and memory updates.

* **User-Space Latency:** ~61 microseconds (61,000 nanoseconds) on a standard Linux kernel without hardware acceleration, CPU pinning, or kernel-bypass networking (DPDK).

## Building the Engine

This project uses CMake as its build system and requires a compiler supporting C++17.

```bash
# Clone the repository
git clone [https://github.com/yourusername/lob-engine.git](https://github.com/aara-dhya/lob-engine.git)
cd lob_engine

# Generate the build system and compile
cmake -S . -B build
cmake --build build