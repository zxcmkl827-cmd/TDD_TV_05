#include "TVController.h"
#include "DevTuner.h"
#include <gtest/gtest.h>

class ControllerTest : public ::testing::Test {
protected:
  std::unique_ptr<DevTuner> ptrDevTuner;
  std::unique_ptr<TVController> ptrTVController;
  void SetUp() override {
    ptrDevTuner = std::make_unique<DevTuner>(std::vector<int>{1, 4, 12, 56});
    ptrTVController = std::make_unique<TVController>(*ptrDevTuner);
  }
};

// S1 - 1 : 한 자리 입력 + 확인
TEST_F(ControllerTest, PressNumber1Confirm) {
  ptrTVController->PressNumber(1);
  ptrTVController->PressConfirm();
  EXPECT_EQ("1", ptrDevTuner->getCurrentCH());
}

// S1 - 2 : 두 자리 자동 변경
TEST_F(ControllerTest, PressNumber2Confirm) {
  ptrTVController->PressNumber(1);
  ptrTVController->PressNumber(2);
  EXPECT_EQ("12", ptrDevTuner->getCurrentCH());
}

// S2 - 1 : 선호 채널 추가
TEST_F(ControllerTest, FavoriteChannelAdd) {
  ptrDevTuner->setCH("12");
  ptrTVController->PressFavorite();
  const auto &favs = ptrTVController->GetFavoriteChannels();
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2 - 2 : 선호 채널 제거
TEST_F(ControllerTest, FavoriteChannelRemove) {
  ptrDevTuner->setCH("12");
  ptrTVController->PressFavorite(); // 추가
  ptrTVController->PressFavorite(); // 삭제(토글)
  const auto &favs = ptrTVController->GetFavoriteChannels();
  EXPECT_EQ(favs.end(), std::find(favs.begin(), favs.end(), 12));
}

// S2 - 3 : 토클 전체 시나리오
TEST_F(ControllerTest, FavoriteToggleScenario) {
  for (int ch : {12, 8, 37, 8, 6}) {
    ptrDevTuner->setCH(std::to_string(ch));
    ptrTVController->PressFavorite();
  }
  const auto &favs = ptrTVController->GetFavoriteChannels();
  // {6, 12, 37} 만남아야함
  EXPECT_EQ(3u, favs.size());
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 6));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 12));
  EXPECT_NE(favs.end(), std::find(favs.begin(), favs.end(), 37));
}