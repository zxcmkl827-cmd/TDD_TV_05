#include "DevTuner.h"
#include "TVController.h"
#include <algorithm>
#include <gtest/gtest.h>
#include <memory>
#include <string>
#include <vector>

class ControllerTest : public ::testing::Test {
protected:
  std::unique_ptr<DevTuner> ptrDevTuner;
  std::unique_ptr<TVController> ptrTVController;

  void SetUp() override {
    ptrDevTuner = std::make_unique<DevTuner>(std::vector<int>{1, 4, 12, 56});
    ptrTVController = std::make_unique<TVController>(*ptrDevTuner);
  }

  void TuneTo(int ch) { ptrDevTuner->setCH(std::to_string(ch)); }

  bool HasFavorite(int ch) const {
    const auto &favs = ptrTVController->GetFavoriteChannels();
    return std::find(favs.begin(), favs.end(), ch) != favs.end();
  }

  void AddFavorites(std::initializer_list<int> channels) {
    for (int ch : channels) {
      ptrTVController->AddFavoriteChannel(ch);
    }
  }
};

// ---------------------------------------------------------------------------
// PressNumber
// ---------------------------------------------------------------------------

// should_set_channel_1_when_one_digit_then_confirm
TEST_F(ControllerTest, PressNumber_OneDigitThenConfirm) {
  ptrTVController->PressNumber(1);
  ptrTVController->PressConfirm();
  EXPECT_EQ("1", ptrDevTuner->getCurrentCH());
}

// should_set_channel_12_when_two_digits_entered_without_confirm
TEST_F(ControllerTest, PressNumber_TwoDigitsAutoApply) {
  ptrTVController->PressNumber(1);
  ptrTVController->PressNumber(2);
  EXPECT_EQ("12", ptrDevTuner->getCurrentCH());
}

// should_apply_12_then_34_when_four_digits_entered
TEST_F(ControllerTest, PressNumber_FourDigitsPairwiseApply) {
  ptrTVController->PressNumber(1);
  ptrTVController->PressNumber(2);
  ptrTVController->PressNumber(3);
  ptrTVController->PressNumber(4);
  EXPECT_EQ("34", ptrDevTuner->getCurrentCH());
}

// should_apply_45_then_confirm_6_when_three_digits_entered
TEST_F(ControllerTest, PressNumber_ThreeDigitsConfirmLast) {
  ptrTVController->PressNumber(4);
  ptrTVController->PressNumber(5);
  ptrTVController->PressNumber(6);
  EXPECT_EQ("45", ptrDevTuner->getCurrentCH());
  ptrTVController->PressConfirm();
  EXPECT_EQ("6", ptrDevTuner->getCurrentCH());
}

// should_set_channel_7_when_leading_zero_then_7
TEST_F(ControllerTest, PressNumber_LeadingZeroDiscarded) {
  ptrTVController->PressNumber(0);
  ptrTVController->PressNumber(7);
  EXPECT_EQ("7", ptrDevTuner->getCurrentCH());
}

// should_set_channel_99_when_boundary_digits_9_and_9
TEST_F(ControllerTest, PressNumber_BoundaryChannel99) {
  ptrTVController->PressNumber(9);
  ptrTVController->PressNumber(9);
  EXPECT_EQ("99", ptrDevTuner->getCurrentCH());
}

// should_set_channel_0_when_zero_digit_confirmed
TEST_F(ControllerTest, PressNumber_BoundaryChannel0) {
  ptrTVController->PressNumber(0);
  ptrTVController->PressConfirm();
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
}

// should_buffer_first_digit_when_only_one_number_pressed
TEST_F(ControllerTest, PressNumber_BufferSingleDigitUntilConfirm) {
  ptrTVController->PressNumber(5);
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
  ptrTVController->PressConfirm();
  EXPECT_EQ("5", ptrDevTuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// PressConfirm
// ---------------------------------------------------------------------------

// should_not_change_channel_when_buffer_is_empty
TEST_F(ControllerTest, PressConfirm_NoBufferDoesNothing) {
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
  ptrTVController->PressConfirm();
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
}

// should_apply_buffered_digit_when_confirm_after_single_input
TEST_F(ControllerTest, PressConfirm_AppliesBufferedDigit) {
  ptrTVController->PressNumber(3);
  ptrTVController->PressConfirm();
  EXPECT_EQ("3", ptrDevTuner->getCurrentCH());
}

// should_apply_channel_0_when_zero_buffered_and_confirmed
TEST_F(ControllerTest, PressConfirm_BoundaryChannel0) {
  ptrTVController->PressNumber(0);
  ptrTVController->PressConfirm();
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
}

// should_apply_channel_9_when_nine_buffered_and_confirmed
TEST_F(ControllerTest, PressConfirm_BoundaryChannel9) {
  ptrTVController->PressNumber(9);
  ptrTVController->PressConfirm();
  EXPECT_EQ("9", ptrDevTuner->getCurrentCH());
}

// should_clear_buffer_after_confirm_so_second_confirm_is_noop
TEST_F(ControllerTest, PressConfirm_ClearsBufferAfterApply) {
  ptrTVController->PressNumber(4);
  ptrTVController->PressConfirm();
  ASSERT_EQ("4", ptrDevTuner->getCurrentCH());
  ptrTVController->PressConfirm();
  EXPECT_EQ("4", ptrDevTuner->getCurrentCH());
}

// should_apply_pending_digit_after_odd_count_sequence
TEST_F(ControllerTest, PressConfirm_AfterThreeDigitSequence) {
  ptrTVController->PressNumber(4);
  ptrTVController->PressNumber(5);
  ptrTVController->PressNumber(6);
  ASSERT_EQ("45", ptrDevTuner->getCurrentCH());
  ptrTVController->PressConfirm();
  EXPECT_EQ("6", ptrDevTuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// PressOthers
// ---------------------------------------------------------------------------

// should_discard_buffered_digit_when_other_pressed_before_confirm
TEST_F(ControllerTest, PressOthers_ClearsBufferBeforeConfirm) {
  ptrTVController->PressNumber(6);
  ptrTVController->PressOthers();
  ptrTVController->PressConfirm();
  EXPECT_EQ("0", ptrDevTuner->getCurrentCH());
}

// should_not_change_channel_when_other_pressed_with_empty_buffer
TEST_F(ControllerTest, PressOthers_NoBufferDoesNotChangeChannel) {
  TuneTo(12);
  ptrTVController->PressOthers();
  EXPECT_EQ("12", ptrDevTuner->getCurrentCH());
}

// should_allow_new_input_after_buffer_cleared_by_other
TEST_F(ControllerTest, PressOthers_AllowsFreshInputAfterClear) {
  ptrTVController->PressNumber(6);
  ptrTVController->PressOthers();
  ptrTVController->PressNumber(3);
  ptrTVController->PressNumber(4);
  EXPECT_EQ("34", ptrDevTuner->getCurrentCH());
}

// should_discard_pending_digit_after_three_digit_sequence
TEST_F(ControllerTest, PressOthers_ClearsPendingAfterThreeDigits) {
  ptrTVController->PressNumber(4);
  ptrTVController->PressNumber(5);
  ptrTVController->PressNumber(6);
  ASSERT_EQ("45", ptrDevTuner->getCurrentCH());
  ptrTVController->PressOthers();
  ptrTVController->PressConfirm();
  EXPECT_EQ("45", ptrDevTuner->getCurrentCH());
}

// should_keep_current_channel_when_other_pressed_mid_single_digit_entry
TEST_F(ControllerTest, PressOthers_PreservesTunedChannel) {
  TuneTo(56);
  ptrTVController->PressNumber(1);
  ptrTVController->PressOthers();
  EXPECT_EQ("56", ptrDevTuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// PressFavorite
// ---------------------------------------------------------------------------

// should_add_current_channel_when_not_in_favorites
TEST_F(ControllerTest, PressFavorite_AddsCurrentChannel) {
  TuneTo(12);
  ptrTVController->PressFavorite();
  EXPECT_TRUE(HasFavorite(12));
}

// should_remove_current_channel_when_already_favorite
TEST_F(ControllerTest, PressFavorite_RemovesExistingChannel) {
  TuneTo(12);
  ptrTVController->PressFavorite();
  ASSERT_TRUE(HasFavorite(12));
  ptrTVController->PressFavorite();
  EXPECT_FALSE(HasFavorite(12));
}

// should_keep_favorites_sorted_when_multiple_channels_added
TEST_F(ControllerTest, PressFavorite_KeepsSortedOrder) {
  for (int ch : {37, 6, 12}) {
    TuneTo(ch);
    ptrTVController->PressFavorite();
  }
  const auto &favs = ptrTVController->GetFavoriteChannels();
  ASSERT_EQ(3u, favs.size());
  EXPECT_EQ((std::vector<int>{6, 12, 37}), favs);
}

// should_toggle_channels_correctly_in_mixed_scenario
TEST_F(ControllerTest, PressFavorite_ToggleScenario) {
  for (int ch : {12, 8, 37, 8, 6}) {
    TuneTo(ch);
    ptrTVController->PressFavorite();
  }
  const auto &favs = ptrTVController->GetFavoriteChannels();
  EXPECT_EQ(3u, favs.size());
  EXPECT_TRUE(HasFavorite(6));
  EXPECT_TRUE(HasFavorite(12));
  EXPECT_TRUE(HasFavorite(37));
  EXPECT_FALSE(HasFavorite(8));
}

// should_add_boundary_channel_0_when_current_is_zero
TEST_F(ControllerTest, PressFavorite_BoundaryChannel0) {
  TuneTo(0);
  ptrTVController->PressFavorite();
  EXPECT_TRUE(HasFavorite(0));
}

// should_add_boundary_channel_99_when_current_is_99
TEST_F(ControllerTest, PressFavorite_BoundaryChannel99) {
  TuneTo(99);
  ptrTVController->PressFavorite();
  EXPECT_TRUE(HasFavorite(99));
}

// ---------------------------------------------------------------------------
// PressNextFavorite
// ---------------------------------------------------------------------------

// should_not_change_channel_when_favorite_list_is_empty
TEST_F(ControllerTest, PressNextFavorite_EmptyListNoOp) {
  TuneTo(6);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("6", ptrDevTuner->getCurrentCH());
}

// should_move_to_12_when_current_is_6_and_favorites_are_preset
TEST_F(ControllerTest, PressNextFavorite_SelectsNearestGreater) {
  AddFavorites({1, 4, 12, 56});
  TuneTo(6);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("12", ptrDevTuner->getCurrentCH());
}

// should_wrap_to_1_when_current_is_56_and_no_greater_favorite
TEST_F(ControllerTest, PressNextFavorite_WrapsToSmallest) {
  AddFavorites({1, 4, 12, 56});
  TuneTo(56);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("1", ptrDevTuner->getCurrentCH());
}

// should_move_to_4_when_current_is_1
TEST_F(ControllerTest, PressNextFavorite_FromSmallestToNext) {
  AddFavorites({1, 4, 12, 56});
  TuneTo(1);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("4", ptrDevTuner->getCurrentCH());
}

// should_wrap_to_same_channel_when_only_one_favorite_exists
TEST_F(ControllerTest, PressNextFavorite_SingleFavoriteWrapsToSelf) {
  AddFavorites({12});
  TuneTo(12);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("12", ptrDevTuner->getCurrentCH());
}

// should_move_to_56_when_current_is_12
TEST_F(ControllerTest, PressNextFavorite_FromMidListToNext) {
  AddFavorites({1, 4, 12, 56});
  TuneTo(12);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("56", ptrDevTuner->getCurrentCH());
}

// ---------------------------------------------------------------------------
// AddFavoriteChannel
// ---------------------------------------------------------------------------

// should_add_channel_when_not_already_favorite
TEST_F(ControllerTest, AddFavoriteChannel_AddsNewChannel) {
  ptrTVController->AddFavoriteChannel(8);
  EXPECT_TRUE(HasFavorite(8));
}

// should_not_duplicate_when_channel_already_favorite
TEST_F(ControllerTest, AddFavoriteChannel_SkipsDuplicate) {
  ptrTVController->AddFavoriteChannel(8);
  ptrTVController->AddFavoriteChannel(8);
  const auto &favs = ptrTVController->GetFavoriteChannels();
  EXPECT_EQ(1u, favs.size());
  EXPECT_EQ(8, favs.front());
}

// should_keep_list_sorted_when_multiple_channels_added
TEST_F(ControllerTest, AddFavoriteChannel_KeepsSortedOrder) {
  ptrTVController->AddFavoriteChannel(56);
  ptrTVController->AddFavoriteChannel(1);
  ptrTVController->AddFavoriteChannel(12);
  EXPECT_EQ((std::vector<int>{1, 12, 56}),
            ptrTVController->GetFavoriteChannels());
}

// should_add_boundary_channels_0_and_99
TEST_F(ControllerTest, AddFavoriteChannel_BoundaryChannels) {
  ptrTVController->AddFavoriteChannel(99);
  ptrTVController->AddFavoriteChannel(0);
  const auto &favs = ptrTVController->GetFavoriteChannels();
  ASSERT_EQ(2u, favs.size());
  EXPECT_EQ(0, favs[0]);
  EXPECT_EQ(99, favs[1]);
}

// should_preserve_existing_favorites_when_adding_new_one
TEST_F(ControllerTest, AddFavoriteChannel_AppendsToExistingList) {
  ptrTVController->AddFavoriteChannel(4);
  ptrTVController->AddFavoriteChannel(12);
  const auto &favs = ptrTVController->GetFavoriteChannels();
  ASSERT_EQ(2u, favs.size());
  EXPECT_EQ(4, favs[0]);
  EXPECT_EQ(12, favs[1]);
}

// should_allow_press_next_favorite_after_programmatic_add
TEST_F(ControllerTest, AddFavoriteChannel_WorksWithPressNextFavorite) {
  ptrTVController->AddFavoriteChannel(4);
  ptrTVController->AddFavoriteChannel(12);
  TuneTo(1);
  ptrTVController->PressNextFavorite();
  EXPECT_EQ("4", ptrDevTuner->getCurrentCH());
}
