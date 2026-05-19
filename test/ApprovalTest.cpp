#define APPROVALS_GOOGLETEST

#include "ApprovalTests.hpp"
#include "TexttestFixture.h"

#include <gtest/gtest.h>
#include <string>

class ApprovalTest : public ::testing::Test {
protected:
  // Approved files live under test/golden/ (see useApprovalsSubdirectory).
  ApprovalTests::SubdirectoryDisposer goldenDir_{"golden"};
};

TEST_F(ApprovalTest, PrintTextFixture) {
  std::string actual = RunPrintTextFixture();
  // StringWriter appends one '\n'; trim fixture trailing newlines first.
  while (!actual.empty() && (actual.back() == '\n' || actual.back() == '\r')) {
    actual.pop_back();
  }
  ApprovalTests::Approvals::verify(actual);
}
