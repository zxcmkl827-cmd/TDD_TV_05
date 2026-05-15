#pragma once
#include "Tuner.h"
#include <algorithm>
#include <stdexcept>
#include <vector>

class DevTuner : public Tuner {
  int CurrentChannel = 0;
  std::vector<int> ChannelLists;

public:
  explicit DevTuner(std::vector<int> channels)
      : ChannelLists(std::move(channels)) {}
  // 현재보다큰채널중가장작은것(없으면wrap)
  std::string seekCH() override {
    auto it = std::upper_bound(ChannelLists.begin(), ChannelLists.end(),
                               CurrentChannel);
    CurrentChannel = it != ChannelLists.end() ? *it : *ChannelLists.begin();
    return std::to_string(CurrentChannel);
  }
  void setCH(const std::string &ch) override {
    int v = std::stoi(ch);
    if (v < 0 || v > 99)
      throw std::invalid_argument("채널범위초과: " + ch);
    CurrentChannel = v;
  }
  std::string getCurrentCH() override { return std::to_string(CurrentChannel); }
};