#include "xxhash.h"
#include "LogLogBeta.h"
#include "LogLogErtl.h"
#include "PerfHelpers.h"
#include <benchmark/benchmark.h>

#include <emmintrin.h>
#include <immintrin.h>
#include <xmmintrin.h>

static void AddString(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogBeta llb;
  std::vector<std::string> strs;
  for (auto ix = 0u; ix < count; ++ix) {
    strs.push_back(random_string((random() % llb::k_string_length) + 1));
  }

  uint64_t i = 0;
  for (auto _ : state) {
    llb.add(strs[i++]);
    i = i >= count ? 0 : i + 1;
  }
}

static void AddHash(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogBeta llb;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.c_str(), str.size()));
  }

  uint64_t i = 0;
  for (auto _ : state) {
    llb.add_hash(hashes[i++]);
    i = i >= count ? 0 : i + 1;
  }
}

static void Cardinality(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogBeta llb;
  for (auto ix = 0u; ix < count; ++ix) {
    llb.add(random_string((random() % llb::k_string_length) + 1));
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(llb.cardinality());
  }
}

static void FullTest(benchmark::State &state) {
  const auto count = 100000;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.data(), str.size()));
  }

  for (auto _ : state) {
    llb::LogLogBeta llb;
    for (const auto hash : hashes) {
        llb.add_hash(hash);
    }
    benchmark::DoNotOptimize(llb.cardinality());
  }
}

static void Merge(benchmark::State &state) {
  uint64_t count = 10000;

  llb::LogLogBeta llb1;
  llb::LogLogBeta llb2;
  for (auto ix = 0u; ix < count; ++ix) {
    llb1.add(random_string((random() % llb::k_string_length) + 1));
    llb2.add(random_string((random() % llb::k_string_length) + 1));
  }

  for (auto _ : state) {
    llb1.merge(llb2);
  }
}

static void CardinalityNonAvx(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogBeta llb;
  for (auto ix = 0u; ix < count; ++ix) {
    llb.add(random_string((random() % llb::k_string_length) + 1));
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(llb.cardinality_nonavx());
  }
}

static void FullTestNonAvx(benchmark::State &state) {
  const auto count = 100000;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.data(), str.size()));
  }

  for (auto _ : state) {
    llb::LogLogBeta llb;
    for (const auto hash : hashes) {
        llb.add_hash(hash);
    }
    benchmark::DoNotOptimize(llb.cardinality_nonavx());
  }
}

static void MergeNonAvx(benchmark::State &state) {
  uint64_t count = 10000;

  llb::LogLogBeta llb1;
  llb::LogLogBeta llb2;
  for (auto ix = 0u; ix < count; ++ix) {
    llb1.add(random_string((random() % llb::k_string_length) + 1));
    llb2.add(random_string((random() % llb::k_string_length) + 1));
  }

  for (auto _ : state) {
    llb1.merge_nonavx(llb2);
  }
}

static void AddHashErtl(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogErtl llb;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.c_str(), str.size()));
  }

  uint64_t i = 0;
  for (auto _ : state) {
    llb.add_hash(hashes[i++]);
    i = i >= count ? 0 : i + 1;
  }
}

static void CardinalityErtl(benchmark::State &state) {
  const auto count = 10000;
  llb::LogLogErtl llb;
  for (auto ix = 0u; ix < count; ++ix) {
    llb.add(random_string((random() % llb::k_string_length) + 1));
  }

  for (auto _ : state) {
    benchmark::DoNotOptimize(llb.cardinality());
  }
}

static void FullTestErtl(benchmark::State &state) {
  const auto count = 100000;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.data(), str.size()));
  }

  for (auto _ : state) {
    llb::LogLogErtl llb;
    for (const auto hash : hashes) {
        llb.add_hash(hash);
    }
    benchmark::DoNotOptimize(llb.cardinality());
  }
}

BENCHMARK(AddHashErtl);
BENCHMARK(CardinalityErtl);
BENCHMARK(FullTestErtl);
/* BENCHMARK(AddString); */
BENCHMARK(AddHash);
BENCHMARK(Cardinality);
BENCHMARK(CardinalityNonAvx);
/* BENCHMARK(Merge); */
/* BENCHMARK(MergeNonAvx); */
BENCHMARK(FullTest);
BENCHMARK(FullTestNonAvx);
