/**
 * LogLogBeta.h
 */
#pragma once

#include <cstdint>
#include <string>
#include <vector>
#include <limits>
#include <cmath>

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

  const int p = 12;
  const int q = 52;
  const int m = 1 << p;
  const double m2alpha = (m / (2. * std::log(2))) * m;
  const std::vector<double> tauValues = initTau(m);
  const std::vector<double> sigmaValues = initSigma(m);

  static double sigma(double x, int &numIterations) {
    numIterations = 0;
    if (x == 1.)
      return std::numeric_limits<double>::infinity();
    double zPrime;
    double y = 1;
    double z = x;
    do {
      numIterations += 1;
      x *= x;
      zPrime = z;
      z += x * y;
      y += y;
    } while (zPrime != z);
    return z;
  }

  static double tau(double x, int &numIterations) {
    numIterations = 0;
    if (x == 0. || x == 1.)
      return 0.;
    double zPrime;
    double y = 1.0;
    double z = 1 - x;
    do {
      numIterations += 1;
      x = std::sqrt(x);
      zPrime = z;
      y *= 0.5;
      z -= std::pow(1 - x, 2) * y;
    } while (zPrime != z);
    return z / 3;
  }

  static std::vector<double> initSigma(int m) {
    int numIterations;
    std::vector<double> result(m + 1);
    for (int c = 0; c <= m; ++c) {
      result[c] = m * sigma(static_cast<double>(c) / static_cast<double>(m),
                            numIterations);
    }
    return result;
  }

  static std::vector<double> initTau(int m) {
    int numIterations;
    std::vector<double> result(m + 1);
    for (int c = 0; c <= m; ++c) {
      result[c] = m * tau(static_cast<double>(m - c) / static_cast<double>(m),
                          numIterations);
    }
    return result;
  }

  double estimate_on_demand(const std::vector<int> &c,
                            int &numSmallCorrectionIterations,
                            int &numLargeCorrectionIterations) const {

    numSmallCorrectionIterations = 0;
    numLargeCorrectionIterations = 0;

    double z =
        m * tau(static_cast<double>(m - c[q + 1]) / static_cast<double>(m),
                numLargeCorrectionIterations);
    for (int k = q; k >= 1; --k) {
      z += c[k];
      z *= 0.5;
    }
    z += m * sigma(static_cast<double>(c[0]) / static_cast<double>(m),
                   numSmallCorrectionIterations);
    return m2alpha / z;
  }

  double estimate_precalculated(const std::vector<int> &c) const {

    double z = tauValues[c[q + 1]];
    for (int k = q; k >= 1; --k) {
      z += c[k];
      z *= 0.5;
    }
    z += sigmaValues[c[0]];
    return m2alpha / z;
  }

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
