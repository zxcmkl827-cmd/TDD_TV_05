#include "TexttestFixture.h"

#include "DevTuner.h"
#include "TVController.h"

#include <sstream>
#include <string>
#include <vector>

std::string RunPrintTextFixture() {
  std::stringstream result;
  DevTuner dtuner(std::vector<int>{1, 4, 12, 56});
  TVController app(dtuner);

  app.PressNumber(1);
  app.PressConfirm();
  result << dtuner.getCurrentCH() << '\n';

  app.PressNumber(1);
  app.PressNumber(2);
  app.PressConfirm();
  result << dtuner.getCurrentCH() << '\n';

  for (int ch : {12, 8, 37, 8, 6}) {
    dtuner.setCH(std::to_string(ch));
    app.PressFavorite();
  }
  const auto favs = app.GetFavoriteChannels();
  for (auto it = favs.begin(); it != favs.end(); ++it) {
    result << std::to_string(*it) << ' ';
  }
  result << '\n';

  return result.str();
}
