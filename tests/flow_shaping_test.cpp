// Host-side check of components/optimizer/flow_shaping.h.
// Run: g++ -std=c++17 -I components/optimizer tests/flow_shaping_test.cpp -o /tmp/fs && /tmp/fs
#include <cassert>
#include <cmath>
#include <cstdio>
#include "flow_shaping.h"

using namespace esphome::optimizer;

static bool near(float a, float b) { return std::fabs(a - b) < 0.01f; }

int main()
{
  // Start with cold water: target 24, feed 20.5 -> only feed + 1.
  assert(near(live_heating_flow(24.0f, 20.5f), 21.5f));
  // Target within the band is kept.
  assert(near(live_heating_flow(25.3f, 25.0f), 25.3f));
  // Target far below the feed is raised to feed - 0.5 (no stop).
  assert(near(live_heating_flow(24.0f, 27.0f), 26.5f));
  // Above the weather ceiling the setpoint walks down at feed - 2, then stops at the ceiling.
  assert(near(live_heating_flow(29.0f, 37.0f, 29.0f), 35.0f));
  assert(near(live_heating_flow(29.0f, 30.0f, 29.0f), 29.0f));
  assert(near(live_heating_flow(29.0f, 29.4f, 29.0f), 29.0f));
  assert(near(live_heating_flow(24.0f, 27.0f, 29.0f), 26.5f));
  // Missing feed leaves the target alone.
  assert(near(live_heating_flow(24.0f, NAN), 24.0f));
  // Room cap starts at the max, then follows the room.
  assert(near(next_room_cap(NAN, NAN, 19.0f, 19.0f, 20.0f, 35.0f), 34.9f));
  assert(near(next_room_cap(28.0f, NAN, 19.4f, 19.0f, 20.0f, 35.0f), 27.8f));
  assert(near(next_room_cap(28.0f, NAN, 18.8f, 19.0f, 20.0f, 35.0f), 28.1f));
  // At target: slow search down. Just below target: hold.
  assert(near(next_room_cap(28.0f, NAN, 19.1f, 19.0f, 20.0f, 35.0f), 27.9f));
  assert(near(next_room_cap(28.0f, NAN, 19.2f, 19.0f, 20.0f, 35.0f), 27.9f));
  assert(near(next_room_cap(28.0f, NAN, 18.95f, 19.0f, 20.0f, 35.0f), 27.9f));
  assert(near(next_room_cap(28.0f, NAN, 18.9f, 19.0f, 20.0f, 35.0f), 28.0f));
  // A new cap starts from the setpoint in use, not from the max.
  assert(near(next_room_cap(NAN, 26.0f, 18.95f, 19.0f, 20.0f, 29.0f), 25.9f));
  assert(near(next_room_cap(NAN, 26.0f, 18.9f, 19.0f, 20.0f, 29.0f), 26.0f));
  assert(near(next_room_cap(NAN, 26.0f, 18.5f, 19.0f, 20.0f, 29.0f), 26.1f));
  // No wind-down below the running floor, never above the max.
  assert(near(next_room_cap(26.0f, NAN, 19.5f, 19.0f, 26.5f, 35.0f), 26.5f));
  assert(near(next_room_cap(35.0f, NAN, 18.0f, 19.0f, 20.0f, 35.0f), 35.0f));
  // Floor above the max: the max wins (the live guard handles the feed).
  assert(near(next_room_cap(30.0f, NAN, 19.0f, 19.0f, 31.0f, 29.0f), 29.0f));
  // Weather ceiling.
  assert(near(weather_flow_ceiling(12.0f), 29.0f));
  assert(near(weather_flow_ceiling(5.0f), 31.5f));
  assert(near(weather_flow_ceiling(0.0f), 34.0f));
  assert(near(weather_flow_ceiling(-7.0f), 38.0f));
  assert(near(weather_flow_ceiling(-15.0f), 38.0f));
  std::puts("flow_shaping_test: OK");
  return 0;
}
