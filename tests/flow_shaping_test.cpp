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
  // Missing feed leaves the target alone.
  assert(near(live_heating_flow(24.0f, NAN), 24.0f));
  std::puts("flow_shaping_test: OK");
  return 0;
}
