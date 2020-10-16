/**
 * LogLogErtl.h
 */
#pragma once

#include <cstdint>
#include <vector>
#include <string>
#include "LogLogBeta.h"

namespace llb {

class LogLogErtl {
public:
  LogLogErtl() noexcept;
  ~LogLogErtl() noexcept;

  void add(const std::string &value) {
    add(reinterpret_cast<const uint8_t *>(value.data()), value.size());
  }

  void add(const char *value, uint64_t length) {
    add(reinterpret_cast<const uint8_t *>(value), length);
  }

  void add(const uint8_t *value, uint64_t length);

  void add_hash(uint64_t hash);

  uint64_t cardinality() const;

  void merge(const LogLogErtl &merge_me);

  std::vector<int64_t> extract_counts() const;

  static std::vector<double> initSigma(int32_t m);

  static std::vector<double> initTau(int32_t m);

protected:
  void sum_registers(double *sum, uint64_t *zero_count) const;

  static double sigma(double x);

  static double tau(double x);

private:

  uint32_t m_precision_bits;
  uint32_t m_max_precision_bits;

  uint8_t m_min_register_val{};

  uint64_t m_register_count{};
  uint8_t *m_registers{};

  std::vector<uint64_t> m_counts;
  uint64_t m_min_count{};
  uint64_t m_register_value_filter{};

  const std::vector<double> & m_tau_values;
  const std::vector<double> & m_sigma_values;
  const double m_m2alpha{};
};
} // namespace llb

