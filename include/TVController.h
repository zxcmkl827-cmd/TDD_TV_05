/**
 * Copyright 2020 by Samsung Electronics, Inc.,
 *
 * This software is the confidential and proprietary information
 * of Samsung Electronics, Inc. ("Confidential Information").  You
 * shall not disclose such Confidential Information and shall use
 * it only in accordance with the terms of the license agreement
 * you entered into with Samsung.
 */

#ifndef TV_CONTROLLER_H
#define TV_CONTROLLER_H

#include "Tuner.h"
#include "remoteKey.h"
#include <algorithm>
#include <iostream>
#include <string>
#include <vector>

class TVController {
protected:
  Tuner &tuner;
  int ChannelBuffer = -1;
  std::vector<int> FavoriteChannels;

public:
  explicit TVController(Tuner &t) : tuner(t) {}
  void PressNumber(int ch);
  void PressConfirm();
  void PressFavorite();
  void PressNextFavorite();
  void PressOthers();
  void AddFavoriteChannel(int ch);
  const std::vector<int> &GetFavoriteChannels() const {
    return FavoriteChannels;
  };

private:
  bool IsValidChannel(int ch) const { return ch >= 0 && ch <= 99; }
  bool IsFavorite(int ch) const {
    return std::find(FavoriteChannels.begin(), FavoriteChannels.end(), ch) !=
           FavoriteChannels.end();
  }
  void ApplyChannel(int ch);
};

#endif // TV_CONTROLLER_H
