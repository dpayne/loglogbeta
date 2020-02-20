/**
 * LogLogBeta.h
 */
#pragma once

#include <cstdint>
#include <string>

namespace llb {
namespace constants {
constexpr uint32_t k_minimum_precision = 8U;
constexpr uint32_t k_default_precision = 14U;
constexpr double k_default_error_rate = 0.01;
constexpr uint32_t k_default_alignment = 1024U;
} // namespace constants

class LogLogBeta {
public:
  LogLogBeta(double error_rate = constants::k_default_error_rate) noexcept;
  ~LogLogBeta() noexcept;

  void add(const std::string &value) {
    add(reinterpret_cast<const uint8_t *>(value.data()), value.size());
  }

  void add(const char *value, uint64_t length) {
    add(reinterpret_cast<const uint8_t *>(value), length);
  }

  void add(const uint8_t *value, uint64_t length);

  void add_hash(uint64_t hash);

#ifdef __AVX512F__
  uint64_t cardinality() const;
#else
  uint64_t cardinality() const { return cardinality_nonavx(); }
#endif

  uint64_t cardinality_nonavx() const;

#ifdef __AVX512F__
  void merge(const LogLogBeta &merge_me);
#else
  void merge(const LogLogBeta &merge_me) { merge_nonavx(merge_me); }
#endif

  void merge_nonavx(const LogLogBeta &merge_me);

protected:
#ifdef __AVX512F__
  void sum_registers(double *sum, uint64_t *zero_count) const;
#else
  void sum_registers(double *sum, uint64_t *zero_count) const {
    sum_registers_nonavx(sum, zero_count);
  }
#endif

  void sum_registers_nonavx(double *sum, uint64_t *zero_count) const;

  static double beta(uint64_t zero_count);

private:
  uint32_t m_precision_bits;
  uint32_t m_max_precision_bits;

  uint64_t m_register_count;
  double m_alpha;
  uint8_t *m_registers;
};
} // namespace llb
