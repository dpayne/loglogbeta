/**
 * LogLogBeta.cpp
 */

#include <cmath>
#include <cstdlib>
#include <cstring>
#include <vector>

#include "LogLogBeta.h"
#include "xxhash.h"

#include <emmintrin.h>
#include <immintrin.h>
#include <pmmintrin.h>
#include <xmmintrin.h>

namespace {
constexpr int64_t k_one = 1;
constexpr __m512i k_ones = {k_one, k_one, k_one, k_one,
                            k_one, k_one, k_one, k_one};

constexpr auto k_alpha1 = 0.7213;
constexpr auto k_alpha2 = 1.079;
} // namespace

llb::LogLogBeta::LogLogBeta(double error_rate) noexcept
    : m_precision_bits{0UL}, m_max_precision_bits{0UL},
      m_register_count{0UL}, m_alpha{0.0}, m_registers{nullptr} {

  const auto est_error_rate =
      std::ceil(std::log2(std::pow((1.04 / error_rate), 2.0)));
  m_precision_bits = std::max(static_cast<uint32_t>(est_error_rate),
                              llb::constants::k_minimum_precision);
  m_max_precision_bits = (sizeof(uint64_t) * 8) - m_precision_bits;
  m_register_count = 1UL << m_precision_bits;
  m_alpha = k_alpha1 / (1.0 + k_alpha2 / static_cast<double>(m_register_count));
  m_registers = reinterpret_cast<uint8_t *>(std::aligned_alloc(
      llb::constants::k_default_alignment, m_register_count));

  std::memset(m_registers, 0, m_register_count);
}

llb::LogLogBeta::~LogLogBeta() noexcept { free(m_registers); }

void llb::LogLogBeta::add(const uint8_t *value, uint64_t length) {
  add_hash(XXH3_64bits(value, length));
}

void llb::LogLogBeta::add_hash(uint64_t hash) {
  // Count leading zeros
  const auto val = __builtin_clzll(hash << m_precision_bits) + 1;

  const auto k = hash >> m_max_precision_bits;
  m_registers[k] =
      m_registers[k] < val ? static_cast<uint8_t>(val) : m_registers[k];
}

void llb::LogLogBeta::sum_registers_nonavx(double *sum_arg,
                                           uint64_t *zero_count_arg) const {

  double sum = 0.0;
  uint64_t zero_count = 0;
  for (auto register_ix = 0UL; register_ix < m_register_count;
       register_ix += 1) {
    const auto value = m_registers[register_ix];
    zero_count += value == 0 ? 1 : 0;
    sum += 1.0 / (1UL << value);
  }

  *sum_arg = sum;
  *zero_count_arg = zero_count;
}

#ifdef __AVX512F__
void llb::LogLogBeta::sum_registers(double *sum_arg,
                                    uint64_t *zero_count_arg) const {
  __m512 accumulate_0 = _mm512_setzero_ps();
  __m512 accumulate_1 = _mm512_setzero_ps();
  __m512 accumulate_2 = _mm512_setzero_ps();
  __m512 accumulate_3 = _mm512_setzero_ps();
  uint64_t zero_count = 0;

  __m512i reg;

  __mmask64 mask;

  __m128i reg0;
  __m128i reg1;
  __m128i reg2;
  __m128i reg3;
  __m128i reg4;
  __m128i reg5;
  __m128i reg6;
  __m128i reg7;

  __m256 reg0_reciprocal;
  __m256 reg1_reciprocal;
  __m256 reg2_reciprocal;
  __m256 reg3_reciprocal;
  __m256 reg4_reciprocal;
  __m256 reg5_reciprocal;
  __m256 reg6_reciprocal;
  __m256 reg7_reciprocal;

  __m512 sum_one_round_0;
  __m512 sum_one_round_1;
  __m512 sum_one_round_2;
  __m512 sum_one_round_3;

  for (auto register_ix = 0UL; register_ix < m_register_count;
       register_ix += 64) {

    reg = *(reinterpret_cast<const __m512i *>(&m_registers[register_ix]));

    mask = _mm512_cmp_epi8_mask(reg, _mm512_setzero_si512(), _MM_CMPINT_EQ);
    zero_count += static_cast<uint64_t>(_mm_popcnt_u64(mask));

    reg0 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 0));
    reg1 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 8));
    reg2 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 16));
    reg3 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 24));
    reg4 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 32));
    reg5 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 40));
    reg6 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 48));
    reg7 = _mm_loadu_si128((__m128i const *)(m_registers + register_ix + 56));

    reg0_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg0))));
    reg1_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg1))));
    reg2_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg2))));
    reg3_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg3))));
    reg4_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg4))));
    reg5_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg5))));
    reg6_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg6))));
    reg7_reciprocal = _mm256_rcp14_ps(_mm512_cvtepi64_ps(
        _mm512_sllv_epi64(k_ones, _mm512_cvtepi8_epi64(reg7))));

    sum_one_round_0 = _mm512_insertf32x8(sum_one_round_0, reg0_reciprocal, 0);
    sum_one_round_0 = _mm512_insertf32x8(sum_one_round_0, reg1_reciprocal, 1);

    sum_one_round_1 = _mm512_insertf32x8(sum_one_round_1, reg2_reciprocal, 0);
    sum_one_round_1 = _mm512_insertf32x8(sum_one_round_1, reg3_reciprocal, 1);

    sum_one_round_2 = _mm512_insertf32x8(sum_one_round_2, reg4_reciprocal, 0);
    sum_one_round_2 = _mm512_insertf32x8(sum_one_round_2, reg5_reciprocal, 1);

    sum_one_round_3 = _mm512_insertf32x8(sum_one_round_3, reg6_reciprocal, 0);
    sum_one_round_3 = _mm512_insertf32x8(sum_one_round_3, reg7_reciprocal, 1);

    accumulate_0 = _mm512_add_ps(sum_one_round_0, accumulate_0);
    accumulate_1 = _mm512_add_ps(sum_one_round_1, accumulate_1);
    accumulate_2 = _mm512_add_ps(sum_one_round_2, accumulate_2);
    accumulate_3 = _mm512_add_ps(sum_one_round_3, accumulate_3);
  }

  accumulate_0 = _mm512_add_ps(accumulate_0, accumulate_1);
  accumulate_0 = _mm512_add_ps(accumulate_0, accumulate_2);
  accumulate_0 = _mm512_add_ps(accumulate_0, accumulate_3);

  *sum_arg =
      accumulate_0[0] + accumulate_0[1] + accumulate_0[2] + accumulate_0[3] +
      accumulate_0[4] + accumulate_0[5] + accumulate_0[6] + accumulate_0[7] +
      accumulate_0[8] + accumulate_0[9] + accumulate_0[10] + accumulate_0[11] +
      accumulate_0[12] + accumulate_0[13] + accumulate_0[14] + accumulate_0[15];
  *zero_count_arg = zero_count;
}
#endif

double llb::LogLogBeta::beta(uint64_t zero_count) {
  const auto leading_zero_count = std::log(zero_count + 1);
  return -0.370393911 * zero_count + 0.070471823 * leading_zero_count +
         0.17393686 * std::pow(leading_zero_count, 2) +
         0.16339839 * std::pow(leading_zero_count, 3) +
         -0.09237745 * std::pow(leading_zero_count, 4) +
         0.03738027 * std::pow(leading_zero_count, 5) +
         -0.005384159 * std::pow(leading_zero_count, 6) +
         0.00042419 * std::pow(leading_zero_count, 7);
}

uint64_t llb::LogLogBeta::cardinality_nonavx() const {
  double sum = 0.0;
  uint64_t zero_count = 0;
  sum_registers_nonavx(&sum, &zero_count);
  return static_cast<uint64_t>(
      (m_alpha * m_register_count * (m_register_count - zero_count)) /
      (beta(zero_count) + sum));
}

#ifdef __AVX512F__
uint64_t llb::LogLogBeta::cardinality() const {
  double sum = 0.0;
  uint64_t zero_count = 0;
  sum_registers(&sum, &zero_count);
  return static_cast<uint64_t>(
      (m_alpha * m_register_count * (m_register_count - zero_count)) /
      (beta(zero_count) + sum));
}
#endif

void llb::LogLogBeta::merge_nonavx(const LogLogBeta &merge_me) {
  for (auto register_ix = 0UL; register_ix < m_register_count;
       register_ix += 1) {
    m_registers[register_ix] =
        m_registers[register_ix] < merge_me.m_registers[register_ix]
            ? merge_me.m_registers[register_ix]
            : m_registers[register_ix];
  }
}

#ifdef __AVX512F__
void llb::LogLogBeta::merge(const LogLogBeta &merge_me) {
  for (auto register_ix = 0UL; register_ix < m_register_count;
       register_ix += 64) {
    const auto this_registers =
        *(reinterpret_cast<const __m512i *>(&m_registers[register_ix]));
    const auto merge_me_registers = *(
        reinterpret_cast<const __m512i *>(&merge_me.m_registers[register_ix]));

    const auto mask =
        _mm512_cmp_epi8_mask(this_registers, merge_me_registers, _MM_CMPINT_LT);
    *(reinterpret_cast<__m512i *>(&m_registers[register_ix])) =
        _mm512_mask_blend_epi8(mask, this_registers, merge_me_registers);
  }
}
#endif
