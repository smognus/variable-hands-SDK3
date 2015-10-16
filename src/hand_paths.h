#include <pebble.h>

GPathInfo hour_hand_path_points = {
  .num_points = 6,
  .points = (GPoint []) {{6,0},{0,8},{-6,0},{-4,-60},{0,-65},{4,-60}}
};
GPathInfo minute_hand_path_points = {
  .num_points = 6,
  .points = (GPoint []) {{4,0},{0,8},{-4,0},{-3,-60},{0,-65},{3,-60}}
};
GPathInfo second_hand_path_points = {
  .num_points = 6,
  .points = (GPoint []) {{4,0},{0,8},{-4,0},{-3,-60},{0,-65},{3,-60}}
};
GPathInfo hand_highlight_path_points = {
  .num_points = 2,
  .points = (GPoint []) {{0,0},{0,-60}}
};
