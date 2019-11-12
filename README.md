# loglogbeta

A C++ implementation of LogLogBeta cardinality estimation algorithm making usee of AVX-512 intrinsics.

Original code was ported from a go implementation by seiflotfy, [loglogbeta](https://github.com/seiflotfy/loglogbeta).

The algorithm comes from a paper by Jason Qin, Denys Kim, Yumei Tung [LogLog-Beta and More: A New Algorithm for Cardinality Estimation Based on LogLog Counting](https://arxiv.org/pdf/1612.02284.pdf).

## Build

    git clone git@github.com:dpayne/loglogbeta.git

    # clone the hashing library xxHash and the testing/benchmark libraries
    git submodule update --init

    mkdir build
    cd build
    cmake ../

    # To enable unit tests use
    #cmake -DBUILD_PERF_TESTS=ON ../

    make


## Results

#### Error Rate

|Actual       | Estimated       | Error Rate |
|-------|-------|----------|
|10     |10     |0         |
|100    |99     |0.01      |
|1000   |999    |0.001     |
|10000  |10054  |0.0054    |
|100000 |99408  |0.00592   |
|1000000|1017794|0.017794  |


#### Performance

|                        | Non-Avx Time (ns) | Avx Time (ns) |
|------------------------|-------------------|---------------|
| Add Hash               |  2                | 2             |
| Estimate Cardinality   |  62,381           | 1,555         |
| Merge                  |  9,133            | 192           |
| Add 100,000 Hashes and Estimate  |  237,703       | 142,38        |

All test were run on skylake CPU running at 4.6 GHz.

## Performance Comparison to other implementations

Note: most of these comparisons are for comparison sake only, this is not really a fair comparison since they are often making different tradeoffs. Namely using the more standard and well tested HLL++ algorithm.

The Java version tested is the HLL++ version taken from Facebook's jcommons library. This version also makes the trade off of calculating the sum and zero counts as it adds hashes. This makes for a fairly fast cardinality check but slows down adding of a hash.

The other C++ implement tested here, libcount, uses the HLL++ algorithm.

The Rust implementation could probably be greatly improved. It is my own version and am I not an expert in rust. As of now, rust stable does not have good intel intrinsics support yet so it uses non-avx code.


| Add 100,000 Hashes and Estimate | Time (ns) |
|------------|-----------|
| LogLogBeta |  142,389  |
| LibCount   |  806,143  |
| Rust      |  224,659  |
| Java HLL   |  8,448,932  |

| Estimate Cardinality | Time (ns) |
|--------------|-----------|
| LogLogBeta   |  2,256    |
| LibCount     |  15,564   |
| Rust        |  77,948   |
| Java HLL     |  2,256 (uses cached sum) |

| Add Single Hash | Time (ns) |
|------------|-----------|
| LogLogBeta |  2         |
| LibCount   |  6         |
| Rust      | 1          |
| Java HLL   |  1,184         |


| Merge Two Results | Time (ns) |
|--------------|-------------|
| LogLogBeta   | 192         |
| LibCount     | 6,303            |
| Rust        | 13,052      |
| Java HLL*   | 503,358,957 |

* The Java HLL used here did not provide a merge operation so I wrote a simple merge function in Java.

All test were run on skylake CPU running at 4.6 GHz.

### Build Perf Tests

    # To enable perf tests and unit tests use, firstt checkout the libcount library
    git clone git@github.com:dialtr/libcount.git perf_tests/extern/libcount
    # Then enable perf tests
    mkdir -p build && cd build
    cmake -DBUILD_PERF_TESTS=ON ../
    make
