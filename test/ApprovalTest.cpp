#define APPROVALS_GOOGLETEST

#include "ApprovalTests.hpp"
#include <gtest/gtest.h>

#include <iostream>
#include <sstream>
#include <string>
#include <vector>

#include "DevTuner.h"
#include "TVController.h"

class ApprovalTest : public ::testing::Test {
protected:
  std::string printTextFixture() {
    std::stringstream result;
    DevTuner dtuner(std::vector<int>{1, 4, 12, 56});
    TVController app(dtuner);

    app.PressNumber(1);
    app.PressConfirm();
    result << dtuner.getCurrentCH();
    result << '\n';

    app.PressNumber(1);
    app.PressNumber(2);
    app.PressConfirm();
    result << dtuner.getCurrentCH();
    result << '\n';

    for (int ch : {12, 8, 37, 8, 6}) {
      dtuner.setCH(std::to_string(ch));
      app.PressFavorite();
    }
    const auto favs = app.GetFavoriteChannels();
    for (auto it = favs.begin(); it != favs.end(); it++)
      result << std::to_string(*it) << ' ';
    result << '\n';

    return result.str();
  }
};

// ─────────────────────────────────────────────
// Approval Test
// ─────────────────────────────────────────────

TEST_F(ApprovalTest, PrintTextFixture) {

  std::string actual = printTextFixture();
  ApprovalTests::Approvals::verify(actual);
}