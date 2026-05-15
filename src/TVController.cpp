#include "TVController.h"
#include <algorithm>
#include <vector>

void TVController::ApplyChannel(int ch) {
  if (!IsValidChannel(ch))
    throw std::invalid_argument("Invalid Ch");
  tuner.setCH(std::to_string(ch));
}

void TVController::PressNumber(int ch) {
  if (ChannelBuffer == -1) {
    ChannelBuffer = ch;
  } else {
    int SelectedChannel = ChannelBuffer * 10 + ch;
    ChannelBuffer = -1;
    ApplyChannel(SelectedChannel);
  }
}

void TVController::PressConfirm() {
  if (ChannelBuffer != -1) {
    int SelectedChannel = ChannelBuffer;
    ChannelBuffer = -1;
    ApplyChannel(SelectedChannel);
  }
}

void TVController::PressOthers() { ChannelBuffer = -1; }

void TVController::PressFavorite() {
  int CurrentChannel = std::stoi(tuner.getCurrentCH());
  if (IsFavorite(CurrentChannel)) {
    FavoriteChannels.erase(std::remove(FavoriteChannels.begin(),
                                       FavoriteChannels.end(), CurrentChannel),
                           FavoriteChannels.end());
  } else {
    FavoriteChannels.push_back(CurrentChannel);
    std::sort(FavoriteChannels.begin(), FavoriteChannels.end());
  }
}

void TVController::PressNextFavorite() {
  if (FavoriteChannels.empty())
    return;
  int CurrentChannel = std::stoi(tuner.getCurrentCH());
  auto it = std::upper_bound(FavoriteChannels.begin(), FavoriteChannels.end(),
                             CurrentChannel);
  int NextChannel =
      it != FavoriteChannels.end() ? *it : *FavoriteChannels.end();
  ApplyChannel(NextChannel);
}

void TVController::AddFavoriteChannel(int ch) {
  if (!IsFavorite(ch)) {
    FavoriteChannels.push_back(ch);
    std::sort(FavoriteChannels.begin(), FavoriteChannels.end());
  }
}