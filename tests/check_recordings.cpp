#include <cassert>
#include <iostream>
#include <stdexcept>

#include "recording.hpp"

int main()
{
  const Graph graph("assets/mapd.emap");
  assert(graph.width == 16 && graph.height == 8);
  assert((graph.regions == std::vector<int>{0, 1}));
  assert(graph.vertexAt(7, 0)->port == PortType::INLET);
  assert(graph.vertexAt(8, 0)->port == PortType::OUTLET);
  assert(graph.vertexAt(7, 7)->port == PortType::OUTLET);
  assert(graph.vertexAt(8, 7)->port == PortType::INLET);

  const auto solution = loadSolution("assets/demo_mapd.txt", graph);
  assert(solution.size() == 11 && solution.front().size() == 3);
  const auto tasks =
      loadTasks("assets/tasks_mapd.txt", graph, solution.front().size());
  assert(currentGoal(tasks[0], 6) == graph.vertexAt(3, 5));
  assert(currentGoal(tasks[0], 7) == graph.vertexAt(9, 2));
  assert(currentGoal(tasks[2], 7) == graph.vertexAt(8, 4));
  assert(currentGoal(tasks[1], 9) == graph.vertexAt(6, 0));
  assert(currentGoal(tasks[1], 8) == graph.vertexAt(1, 0));
  assert(currentGoal(tasks[0], 0) == graph.vertexAt(3, 5));
  assert(currentGoal(tasks[0], 100) == graph.vertexAt(9, 2));
  assert(currentGoal({{graph.vertexAt(0, 0), 0}}, 0) == nullptr);
  assert(currentGoal({}, 0) == nullptr);

  bool rejected = false;
  try {
    loadTasks("assets/tasks_mapd.txt", graph, 2);
  } catch (const std::runtime_error&) {
    rejected = true;
  }
  assert(rejected);

  for (const std::string map :
       {"2x2", "random-32-32-20", "tunnel", "ost003d"}) {
    const Graph legacy("assets/" + map + ".map");
    const auto paths = loadSolution("assets/demo_" + map + ".txt", legacy);
    assert(legacy.regions.empty());
    if (map == "2x2") assert(paths.front().front().o == Orientation::Y_MINUS);
    if (map == "ost003d") assert(paths.front().size() == 1000);
  }

  std::cout << "MAPD recording checks passed\n";
}
