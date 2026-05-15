#include "TVController.h"
#include "Tuner.h"
#include <gmock/gmock.h>
#include <gtest/gtest.h>

class MockTunerForController : public Tuner {
public:
  MOCK_METHOD(std::string, seekCH, (), (override));
  MOCK_METHOD(void, setCH, (const std::string &ch), (override));
  MOCK_METHOD(std::string, getCurrentCH, (), (override));
};

class TVControllerMockTest : public ::testing::Test {
protected:
  MockTunerForController dtuner;
  std::unique_ptr<TVController> ptrTVController;
  void SetUp() override {
    ptrTVController = std::make_unique<TVController>(dtuner);
  }
};

// S1 - 1 : 한 자리 입력 + 확인
TEST_F(TVControllerMockTest, PressNumber1Confirm) {
  EXPECT_CALL(dtuner, setCH("1")).Times(1);
  ptrTVController->PressNumber(1);
  ptrTVController->PressConfirm();
}

// S1 - 2 : 두 자리 자동 변경
TEST_F(TVControllerMockTest, PressNumber2Confirm) {
  EXPECT_CALL(dtuner, setCH("12")).Times(1);
  ptrTVController->PressNumber(1);
  ptrTVController->PressNumber(2);
}

// pressFavorite → getCurrentCH() 호출검증
TEST_F(TVControllerMockTest, PressFavorite_GetsCurrentCH) {
  EXPECT_CALL(dtuner, getCurrentCH()).WillOnce(::testing::Return("12"));
  ptrTVController->PressFavorite();
}

// pressNextFavorite: 선호목록있을때setCH 호출
TEST_F(TVControllerMockTest, NextFav_CallsSetCH) {
  ptrTVController->AddFavoriteChannel(12);
  ptrTVController->AddFavoriteChannel(56);
  EXPECT_CALL(dtuner, getCurrentCH()).WillOnce(::testing::Return("6"));
  EXPECT_CALL(dtuner, setCH("12")).Times(1);
  ptrTVController->PressNextFavorite();
}