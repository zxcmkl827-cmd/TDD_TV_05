#include "GoldenFile.h"
#include "TexttestFixture.h"

#include <gtest/gtest.h>

TEST(GoldenMasterTest, PrintTextFixture) {
  golden::AssertMatchesGolden(
      RunPrintTextFixture(), "ApprovalTest.PrintTextFixture.approved.txt");
}
