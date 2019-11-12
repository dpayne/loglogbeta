#include "extern/libcount/include/count/hll.h"
#include <benchmark/benchmark.h>

#include "LogLogBeta.h"
#include "PerfHelpers.h"
#include "xxhash.h"

static void LibCountAddString(benchmark::State &state) {
  const auto count = 10000;
  libcount::HLL *hll =
      libcount::HLL::Create(llb::constants::k_default_precision);
  std::vector<std::string> strs;
  for (auto ix = 0u; ix < count; ++ix) {
    strs.push_back(random_string((random() % llb::k_string_length) + 1));
  }

  uint64_t i = 0;
  for (auto _ : state) {
    const auto &str = strs[i++];
    hll->Update(XXH3_64bits(str.c_str(), str.size()));
    i = i >= count ? 0 : i;
  }

  delete hll;
}

static void LibCountAddHash(benchmark::State &state) {
  const auto count = 10000;
  libcount::HLL *hll =
      libcount::HLL::Create(llb::constants::k_default_precision);
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.c_str(), str.size()));
  }

  uint64_t i = 0;
  for (auto _ : state) {
    hll->Update(hashes[i++]);
    i = i >= count ? 0 : i;
  }
  delete hll;
}

static void LibCountCardinality(benchmark::State &state) {
  const auto count = 10000;
  libcount::HLL *hll =
      libcount::HLL::Create(llb::constants::k_default_precision);
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hll->Update(XXH3_64bits(str.c_str(), str.size()));
  }

  for (auto _ : state) {
    hll->Estimate();
  }
  delete hll;
}

static void LibCountMerge(benchmark::State &state) {
  uint64_t count = 10000;

  libcount::HLL *hll1 =
      libcount::HLL::Create(llb::constants::k_default_precision);
  libcount::HLL *hll2 =
      libcount::HLL::Create(llb::constants::k_default_precision);
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str1 = random_string((random() % llb::k_string_length) + 1);
    const auto str2 = random_string((random() % llb::k_string_length) + 1);
    hll1->Update(XXH3_64bits(str1.c_str(), str1.size()));
    hll2->Update(XXH3_64bits(str2.c_str(), str2.size()));
  }

  for (auto _ : state) {
    hll1->Merge(hll2);
  }

  delete hll1;
  delete hll2;
}

static void LibCountFullTest(benchmark::State &state) {
  const auto count = 100000;
  std::vector<uint64_t> hashes;
  for (auto ix = 0u; ix < count; ++ix) {
    const auto str = random_string((random() % llb::k_string_length) + 1);
    hashes.push_back(XXH3_64bits(str.data(), str.size()));
  }

  for (auto _ : state) {
    libcount::HLL *hll =
        libcount::HLL::Create(llb::constants::k_default_precision);
    for (const auto hash : hashes) {
      hll->Update(hash);
    }
    benchmark::DoNotOptimize(hll->Estimate());
    delete hll;
  }
}

BENCHMARK(LibCountAddString);
BENCHMARK(LibCountAddHash);
BENCHMARK(LibCountCardinality);
BENCHMARK(LibCountMerge);
BENCHMARK(LibCountFullTest);
