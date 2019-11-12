#include <algorithm>
#include <functional>

#include "LogLogBeta.h"
#include "gtest/gtest.h"

int k_string_length = 256;
double k_error_limit = 0.02; // 2% estimation error
int k_merge_count = 64;

std::string random_string(size_t length) {
  auto randchar = []() -> char {
    const char charset[] = "0123456789"
                           "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                           "abcdefghijklmnopqrstuvwxyz";
    const size_t max_index = (sizeof(charset) - 1);
    return charset[rand() % max_index];
  };
  std::string str(length, 0);
  std::generate_n(str.begin(), length, randchar);
  return str;
}

TEST(LogLogBeta, AddOne) {
  llb::LogLogBeta llb{};
  llb.add("hello world");
  llb.cardinality();
}

void add_random_strings(uint64_t count, llb::LogLogBeta *llb) {
  std::map<std::string, bool> unique_hashes;
  while (unique_hashes.size() < count) {
    unique_hashes[random_string((random() % k_string_length) + 1)] = true;
  }

  for (const auto val : unique_hashes) {
    llb->add(val.first);
  }
}

double estimate_error(uint64_t expected, uint64_t actual) {
  const auto delta =
      std::abs(static_cast<int64_t>(expected) - static_cast<int64_t>(actual));
  return static_cast<double>(delta) / static_cast<double>(expected);
}

TEST(LogLogBeta, EstimateError) {
  int64_t count = 10;
  while (count <= 1000000) {
    llb::LogLogBeta llb{};
    add_random_strings(count, &llb);
    std::cout << count << "\t" << llb.cardinality() << "\t" << estimate_error(count, llb.cardinality()) << std::endl;
    ASSERT_TRUE(estimate_error(count, llb.cardinality()) < k_error_limit)
        << "cardinality: " << llb.cardinality();
    count *= 10;
  }
}

TEST(LogLogBeta, Add1000) {
  int64_t count = 1000;
  llb::LogLogBeta llb{};
  add_random_strings(count, &llb);
  ASSERT_TRUE(estimate_error(count, llb.cardinality()) < k_error_limit)
      << "cardinality: " << llb.cardinality();
}

TEST(LogLogBeta, Add100000) {
  int64_t count = 100000;
  llb::LogLogBeta llb{};
  add_random_strings(count, &llb);
  ASSERT_TRUE(estimate_error(count, llb.cardinality()) < k_error_limit)
      << "cardinality: " << llb.cardinality();
}

TEST(LogLogBetaMerge, Simple) {
  llb::LogLogBeta llb{};
  llb.add("hello world");

  llb::LogLogBeta llb2{};
  llb2.add("good bye world");
  llb.merge(llb2);
  llb.cardinality();
}

TEST(LogLogBetaMerge, Two) {
  int64_t count = 10000;
  llb::LogLogBeta llb{};
  add_random_strings(count, &llb);

  llb::LogLogBeta llb2{};
  add_random_strings(count, &llb2);
  llb.merge(llb2);

  ASSERT_TRUE(estimate_error(count * 2, llb.cardinality()) < k_error_limit)
      << "cardinality: " << llb.cardinality();
}

TEST(LogLogBetaMerge, Many) {
  std::vector<llb::LogLogBeta> llbs;
  llbs.resize(k_merge_count);
  int64_t count = 10000;
  for (auto &llb : llbs) {
    add_random_strings(count, &llb);
  }

  llb::LogLogBeta main;
  for (const auto &llb : llbs) {
    main.merge(llb);
  }

  ASSERT_TRUE(estimate_error(count * k_merge_count, main.cardinality()) <
              k_error_limit)
      << "cardinality: " << main.cardinality();
}
