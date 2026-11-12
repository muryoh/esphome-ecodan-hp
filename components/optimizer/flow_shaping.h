#pragma once

// Pure flow-setpoint shaping helpers used by the auto-adaptive loop.
// Kept free of ESPHome types so tests/flow_shaping_test.cpp can build them on a PC.

#include <algorithm>
#include <cmath>

namespace esphome
{
  namespace optimizer
  {
    // A setpoint more than this below the feed risks an FTC compressor stop.
    constexpr float FLOW_FLOOR_BELOW_FEED = 0.5f;
    // A setpoint far above the feed makes the FTC ramp the compressor hard
    // (66 Hz on a start with cold water), so a running compressor is only
    // ever asked for this much above the current feed.
    constexpr float FLOW_CEILING_ABOVE_FEED = 1.0f;

    // Heating setpoint for a running compressor: the target, kept between
    // feed - FLOW_FLOOR_BELOW_FEED and feed + FLOW_CEILING_ABOVE_FEED.
    inline float live_heating_flow(float target, float feed)
    {
      if (std::isnan(feed) || std::isnan(target))
        return target;
      float flow = std::min(target, feed + FLOW_CEILING_ABOVE_FEED);
      flow = std::max(flow, feed - FLOW_FLOOR_BELOW_FEED);
      return std::round(flow * 10.0f) / 10.0f;
    }
  } // namespace optimizer
} // namespace esphome
