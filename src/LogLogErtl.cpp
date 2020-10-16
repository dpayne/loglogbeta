/**
 * LogLogErtl.cpp
 */

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <numeric>
#include <vector>
#include <iostream>

#include "LogLogErtl.h"
#include "xxhash.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <xmmintrin.h>

namespace {
constexpr int64_t k_one = 1;
constexpr __m512i k_ones = {k_one, k_one, k_one, k_one,
                            k_one, k_one, k_one, k_one};

constexpr double k_error_rate = llb::constants::k_default_error_rate;
constexpr auto k_est_error_rate =
    std::ceil(std::log2(std::pow((1.04 / k_error_rate), 2.0)));
constexpr auto k_precision_bits =
    std::max(static_cast<uint32_t>(k_est_error_rate),
             llb::constants::k_minimum_precision);
constexpr auto k_max_precision_bits = (sizeof(uint64_t) * 8) - k_precision_bits;
constexpr auto k_register_count = 1UL << k_precision_bits;

constexpr double k_m2alpha =
    (k_register_count / (2.0 * std::log(2))) * k_register_count;
const std::vector<double> k_tau_values =
    llb::LogLogErtl::initTau(k_register_count);
const std::vector<double> k_sigma_values =
    llb::LogLogErtl::initSigma(k_register_count);
} // namespace

llb::LogLogErtl::LogLogErtl() noexcept
    : m_precision_bits{k_precision_bits},
      m_max_precision_bits{k_max_precision_bits},
      m_register_count{k_register_count}, m_min_register_val{0UL},
      m_registers{nullptr}, m_tau_values{k_tau_values},
      m_sigma_values{k_sigma_values}, m_m2alpha{k_m2alpha} {

  m_counts = std::vector<uint64_t>(m_max_precision_bits + 2);
  m_counts[0] = m_register_count;
  m_min_count = 0;
  m_register_value_filter = ~uint_fast64_t(0);

  m_register_count = 1UL << m_precision_bits;
  m_registers = reinterpret_cast<uint8_t *>(std::aligned_alloc(
      llb::constants::k_default_alignment, m_register_count));

  std::memset(m_registers, 0, m_register_count);
}

llb::LogLogErtl::~LogLogErtl() noexcept { free(m_registers); }

double llb::LogLogErtl::sigma(double x) {
  if (x == 1.0)
    return std::numeric_limits<double>::infinity();
  auto zPrime = 0.0;
  auto y = 1.0;
  auto z = x;
  do {
    x *= x;
    zPrime = z;
    z += x * y;
    y += y;
  } while (zPrime != z);
  return z;
}

double llb::LogLogErtl::tau(double x) {
  if (x == 0.0 || x == 1.0)
    return 0.0;

  auto zPrime = 0.0;
  auto y = 1.0;
  auto z = 1.0 - x;
  do {
    x = std::sqrt(x);
    zPrime = z;
    y *= 0.5;
    z -= std::pow(1.0 - x, 2.0) * y;
  } while (zPrime != z);
  return z / 3.0;
}

std::vector<double> llb::LogLogErtl::initSigma(int32_t m) {
  std::vector<double> result(m + 1);
  for (int32_t c = 0; c <= m; ++c) {
    result[c] = m * sigma(static_cast<double>(c) / static_cast<double>(m));
  }
  return result;
}

std::vector<double> llb::LogLogErtl::initTau(int32_t m) {
  std::vector<double> result(m + 1);
  for (auto c = 0; c <= m; ++c) {
    result[c] = m * tau(static_cast<double>(m - c) / static_cast<double>(m));
  }
  return result;
}

void llb::LogLogErtl::add(const uint8_t *value, uint64_t length) {
  add_hash(XXH3_64bits(value, length));
}

void llb::LogLogErtl::add_hash(uint64_t hashValue) {
  if ((m_register_value_filter | hashValue) == (~uint_fast64_t(0))) {
    const std::size_t registerIdx = hashValue >> m_max_precision_bits;
    unsigned char runLength = 1;
    while (runLength <= m_max_precision_bits && (hashValue & 1)) {
      runLength += 1;
      hashValue >>= 1;
    }

    unsigned char oldRunLength = m_registers[registerIdx];
    if (oldRunLength < runLength) {
      m_registers[registerIdx] = static_cast<uint8_t>(runLength);
      m_counts[oldRunLength] = m_counts[oldRunLength] - 1;
      m_counts[runLength] = m_counts[runLength] + 1;

      if (m_counts[oldRunLength] == 0 && oldRunLength == m_min_count) {
        while (m_counts[m_min_count] == 0) {
          m_min_count += 1;
        }
        m_register_value_filter = (~uint_fast64_t(0)) << m_min_count;
      }
    }
  }
}

std::vector<int64_t> llb::LogLogErtl::extract_counts() const {
  std::vector<int64_t> counts(m_max_precision_bits + 2);
  for (auto i = 0u; i < m_register_count; ++i) {
    counts[std::min(i, m_max_precision_bits + 1U)] += m_registers[i];
  }

  return counts;
}

uint64_t llb::LogLogErtl::cardinality() const {
  double z = m_tau_values[m_counts[m_max_precision_bits + 1]];
  for (auto k = m_max_precision_bits; k >= 1; --k) {
    z += m_counts[k];
    z *= 0.5;
  }
  z += m_sigma_values[m_counts[0]];

  return m_m2alpha / z;
}

void llb::LogLogErtl::merge(const LogLogErtl &merge_me) {
  for (auto register_ix = 0UL; register_ix < m_register_count;
       register_ix += 1) {
    m_registers[register_ix] =
        m_registers[register_ix] < merge_me.m_registers[register_ix]
            ? merge_me.m_registers[register_ix]
            : m_registers[register_ix];
  }
}
