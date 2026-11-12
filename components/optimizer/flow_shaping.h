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

    // Room-driven cap on the heating flow, moved once per auto-adaptive cycle
    // (5 min): down by ROOM_CAP_STEP while the room is above target +
    // ROOM_CAP_ABOVE_TARGET, down by ROOM_CAP_SEARCH_STEP while it is at or
    // just above target, up while it is below target - ROOM_CAP_BELOW_TARGET,
    // held in between. The slow search down finds the lowest flow that still
    // holds the target instead of sitting on whatever cap the last rise left.
    // Auto-adaptive itself never aims below return + base delta-T, so without
    // this cap the flow only ratchets up.
    constexpr float ROOM_CAP_STEP = 0.2f;
    constexpr float ROOM_CAP_SEARCH_STEP = 0.1f;
    constexpr float ROOM_CAP_ABOVE_TARGET = 0.2f;
    constexpr float ROOM_CAP_BELOW_TARGET = 0.1f;

    // lo should include feed - FLOW_FLOOR_BELOW_FEED while the compressor runs,
    // so the cap does not wind down below what can actually be applied.
    // A cap that is not set yet (boot, new run) starts from `start`, the flow
    // setpoint in use, so a run never begins with a free climb to hi.
    inline float next_room_cap(float cap, float start, float room, float target, float lo, float hi)
    {
      if (std::isnan(cap))
        cap = std::isnan(start) ? hi : start;
      if (!std::isnan(room) && !std::isnan(target))
      {
        if (room > target + ROOM_CAP_ABOVE_TARGET)
          cap -= ROOM_CAP_STEP;
        else if (room >= target - 0.05f) // at target, with sensor float noise
          cap -= ROOM_CAP_SEARCH_STEP;
        else if (room < target - ROOM_CAP_BELOW_TARGET)
          cap += ROOM_CAP_STEP;
      }
      return std::clamp(cap, std::min(lo, hi), hi);
    }

    // Weather ceiling for the room cap: 29 °C at +10 °C outside and above,
    // 34 °C at 0 °C, 38 °C at -7 °C and below, linear in between. A gentle
    // radiator curve that still rises by itself in cold weather.
    inline float weather_flow_ceiling(float outside)
    {
      if (std::isnan(outside))
        return 38.0f;
      if (outside >= 10.0f)
        return 29.0f;
      if (outside >= 0.0f)
        return 34.0f - 0.5f * outside;
      if (outside >= -7.0f)
        return 34.0f - 4.0f / 7.0f * outside;
      return 38.0f;
    }
  } // namespace optimizer
} // namespace esphome
