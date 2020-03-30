/////////////////////////////////////////////////////////////////////////

// Hybrid A* Search implementation
// (C)2001 Justin Heyes-Jones
//
// Finding a path on a simple grid maze
// This shows how to do shortest path finding using A*

//////////////////////////////////////////////////////////////////////////

#include "common/plotting/include/gnuplot-iostream.h"

#include "../include/HybridAStar.h"

#include <math.h>
#include <stdio.h>
#include <iostream>

constexpr int DEBUG_LISTS = 0;
constexpr int DEBUG_LIST_LENGTHS_ONLY = 0;

using namespace ASV::planning;

// Main
int main() {
  CollisionData _collisiondata{
      4,     // MAX_SPEED
      4.0,   // MAX_ACCEL
      -3.0,  // MIN_ACCEL
      2.0,   // MAX_ANG_ACCEL
      -2.0,  // MIN_ANG_ACCEL
      0.2,   // MAX_CURVATURE
      4,     // HULL_LENGTH
      2,     // HULL_WIDTH
      2,     // HULL_BACK2COG
      3.3    // ROBOT_RADIUS
  };

  HybridAStar Hybrid_AStar(_collisiondata);

  Hybrid_AStar.performsearch();

  auto hr = Hybrid_AStar.results();

  // std::vector<std::pair<double, double>> x_y_ps;
  // std::vector<std::pair<double, double>> x_y_startend;

  // std::cout << "Displaying solution\n";

  // while (node) {
  //   node->PrintNodeInfo();
  //   x_y_ps.push_back(std::make_pair(node->x(), node->y()));
  //   node = astarsearch.GetSolutionNext();
  // }

  // x_y_startend.push_back(std::make_pair(nodeStart.x(), nodeStart.y()));
  // x_y_startend.push_back(std::make_pair(nodeEnd.x(), nodeEnd.y()));

  // // Once you're done with the solution you can free the nodes up
  // astarsearch.FreeSolutionNodes();

  // int MAP_WIDTH = 20;
  // int MAP_HEIGHT = 30;

  // Gnuplot gp;
  // gp << "set terminal x11 size 1100, 1200 0\n";
  // gp << "set title 'A star search'\n";
  // gp << "set tic scale 0\n";
  // gp << "set palette rgbformula -7,2,-7\n";  // rainbow
  // gp << "set cbrange [0:9]\n";
  // gp << "set cblabel 'obstacle'\n";
  // gp << "unset cbtics\n";
  // gp << "set xrange [0:" << MAP_WIDTH << "]\n";
  // gp << "set yrange [0:" << MAP_HEIGHT << "]\n";

  // std::vector<std::tuple<double, double, int>> x_y_z;
  // for (int i = 0; i != MAP_WIDTH; ++i)
  //   for (int j = 0; j != MAP_HEIGHT; ++j)
  //     x_y_z.push_back({i, j, nodeStart.GetMap(i, j)});
  // gp << "plot " << gp.file1d(x_y_z) << " with image notitle, "
  //    << gp.file1d(x_y_startend) << " with points pointtype 7 notitle,"
  //    << gp.file1d(x_y_ps) << " with points lc rgb 'black' title
  //    'path'\n";

  return 0;
}