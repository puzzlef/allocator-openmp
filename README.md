Design of multi-threaded memory allocator for supporting the implementation of dynamic graphs.

Research in graph-structured data has grown rapidly due to graphs' ability to represent complex real-world information and capture intricate relationships, particularly as many real-world graphs evolve dynamically through edge/vertex insertions and deletions. This has spurred interest in programming frameworks for managing, maintaining, and processing such dynamic graphs. In our report, we evaluate the performance of [PetGraph (Rust)], [Stanford Network Analysis Platform (SNAP)], [SuiteSparse:GraphBLAS], [cuGraph], [Aspen], and [our custom implementation] in tasks including loading graphs from disk to memory, cloning loaded graphs, applying in-place edge deletions/insertions, and performing a simple iterative graph traversal algorithm. Our implementation demonstrates significant performance improvements: it outperforms PetGraph, SNAP, SuiteSparse:GraphBLAS, cuGraph, and Aspen by factors of `177x`, `106x`, `76x`, `17x`, and `3.3x` in graph loading; `20x`, `235x`, `0.24x`, `1.3x`, and `0x` in graph cloning; `141x`/`45x`, `44x`/`25x`, `13x`/`11x`, `28x`/`34x`, and `3.5x`/`2.2x` in edge deletions/insertions; and `67x`/`63x`, `86x`/`86x`, `2.5x`/`2.6x`, `0.25x`/`0.24x`, and `1.3x`/`1.3x` in traversal on updated graphs with deletions/insertions.

Below, we plot the relative runtime of various memory allocators across three different workloads: allocation-only, deallocation-only, and mixed. In the *allocation-only* workload, `2^28` allocations of `64` bytes each are performed, while in the *deallocation-only* `2^28` deallocations of previously allocated objects are performed. Finally, the *mixed* workload consists of `2^22` allocations followed by `2^22` deallocations, repeated `64` times. The allocators compared include the *C library allocator* (`malloc()/free()`), *C++ runtime allocator* (`new[]/delete[]`), *Fixed Arena Allocator (FAA)*, *variable-capacity Arena Allocator (AA)*, and **Concurrent Power-of-2 Arena Allocator (CP2AA)**.

<img src="https://github.com/user-attachments/assets/6b78b775-1e07-4b66-9c81-3d11de2b8982" width="60%"><br>
<img src="https://github.com/user-attachments/assets/3cf35aea-c81a-4a3c-900d-47c7343e1fc7" width="60%"><br>
<img src="https://github.com/user-attachments/assets/5b4b8a28-1832-4084-8e6f-8274ce1e813e" width="60%"><br>

Our results show that specialized allocators like *FAA*, *AA*, and **CP2AA** significantly outperform *general-purpose allocators* (`malloc()`/`new[]`), especially in allocation-intensive workloads, achieving around a `4×` speedup in mixed workloads. While AA delivers the best performance, its lack of thread safety makes it unsuitable for parallel applications. In contrast, **CP2AA** balances performance with thread safety, making it a suitable choice.

Refer to our technical report for more details: \
[Performance Comparison of Graph Representations Which Support Dynamic Graph Updates][report].

<br>

> [!NOTE]
> You can just copy `main.sh` to your system and run it. \
> For the code, refer to `main.cxx`.

<br>
<br>


### Code structure

The code structure is as follows:

```bash
- inc/_cmath.hxx: Math functions
- inc/_memory.hxx: The memory allocators (FAA, AA, CP2AA)
- inc/_main.hxx: Main utility functions header
- inc/main.hxx: Main header (yes, two main headers)
- main.cxx: Experimentation code
- process.js: Node.js script for processing output logs
```

Note that each branch in this repository contains code for a specific experiment. The `main` branch contains code for the final experiment. If the intention of a branch in unclear, or if you have comments on our technical report, feel free to open an issue.

<br>
<br>


## References

- [Algorithm 1037: SuiteSparse:GraphBLAS: Parallel Graph Algorithms in the Language of Sparse Linear Algebra; Timothy A. Davis et al. (2023)](https://dl.acm.org/doi/full/10.1145/3577195)
- [Low-latency graph streaming using compressed purely-functional trees; Laxman Dhulipala et al. (2019)](https://dl.acm.org/doi/abs/10.1145/3314221.3314598)
- [cuGraph C++ primitives: vertex/edge-centric building blocks for parallel graph computing; Seunghwa Kang et al. (2023)](https://ieeexplore.ieee.org/abstract/document/10196665)
- [SNAP: A General-Purpose Network Analysis and Graph-Mining Library; Jure Leskovec et al. (2016)](https://dl.acm.org/doi/abs/10.1145/2898361)
- [The University of Florida Sparse Matrix Collection; Timothy A. Davis et al. (2011)](https://doi.org/10.1145/2049662.2049663)

<br>
<br>


[![](https://img.youtube.com/vi/yqO7wVBTuLw/maxresdefault.jpg)](https://www.youtube.com/watch?v=yqO7wVBTuLw)<br>
[![ORG](https://img.shields.io/badge/org-puzzlef-green?logo=Org)](https://puzzlef.github.io)
![](https://ga-beacon.deno.dev/G-KD28SG54JQ:hbAybl6nQFOtmVxW4if3xw/github.com/puzzlef/allocator-openmp)

[PetGraph (Rust)]: https://github.com/petgraph/petgraph
[Stanford Network Analysis Platform (SNAP)]: https://github.com/snap-stanford/snap
[SuiteSparse:GraphBLAS]: https://github.com/GraphBLAS/LAGraph
[cuGraph]: https://github.com/rapidsai/cugraph
[Aspen]: https://github.com/ldhulipala/aspen
[our custom implementation]: https://github.com/puzzlef/graph-openmp
[sheets-o1]: https://docs.google.com/spreadsheets/d/1aEU4GK9SKdOHSU7-RXMufqYGTeJVMOn9zyf7tj4LO-s/edit?usp=sharing
[report]: https://arxiv.org/abs/2502.13862
