#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <sstream>
#include <string>

#include <gtest/gtest.h>

namespace golden {

inline std::string NormalizeNewlines(std::string text) {
  std::string normalized;
  normalized.reserve(text.size());
  for (std::size_t i = 0; i < text.size(); ++i) {
    if (text[i] == '\r') {
      if (i + 1 < text.size() && text[i + 1] == '\n') {
        ++i;
      }
      normalized.push_back('\n');
      continue;
    }
    normalized.push_back(text[i]);
  }
  return normalized;
}

inline std::filesystem::path GoldenRoot() {
#ifdef GOLDEN_DIR
  return std::filesystem::path(GOLDEN_DIR);
#else
  return std::filesystem::path("golden");
#endif
}

inline std::string ReadFile(const std::filesystem::path &path) {
  std::ifstream in(path, std::ios::binary);
  EXPECT_TRUE(in.is_open()) << "Cannot read golden file: " << path.string();
  if (!in) {
    return {};
  }
  std::ostringstream buffer;
  buffer << in.rdbuf();
  return buffer.str();
}

inline void WriteFile(const std::filesystem::path &path,
                      const std::string &content) {
  std::filesystem::create_directories(path.parent_path());
  std::ofstream out(path, std::ios::binary | std::ios::trunc);
  ASSERT_TRUE(out.is_open()) << "Cannot write golden file: " << path.string();
  out << content;
}

inline bool UpdateGoldenEnabled() {
#if defined(_MSC_VER)
#pragma warning(push)
#pragma warning(disable : 4996)
#endif
  const char *flag = std::getenv("UPDATE_GOLDEN");
#if defined(_MSC_VER)
#pragma warning(pop)
#endif
  return flag != nullptr && flag[0] != '\0' && flag[0] != '0';
}

inline void AssertMatchesGolden(const std::string &actual,
                                const std::string &goldenFileName) {
  const auto approved = GoldenRoot() / goldenFileName;

  if (UpdateGoldenEnabled()) {
    WriteFile(approved, actual);
    GTEST_LOG_(INFO) << "Updated golden file: " << approved.string();
    return;
  }

  const std::string expected = NormalizeNewlines(ReadFile(approved));
  const std::string normalizedActual = NormalizeNewlines(actual);
  EXPECT_EQ(expected, normalizedActual)
      << "Golden mismatch. To refresh: set UPDATE_GOLDEN=1 and re-run.";
}

} // namespace golden
