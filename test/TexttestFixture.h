#pragma once

#include <string>

// Texttest-style fixture: runs a fixed remote-control scenario and returns
// the captured tuner/channel output as a single golden-master string.
std::string RunPrintTextFixture();
