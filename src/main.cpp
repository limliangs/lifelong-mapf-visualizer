#include <iostream>
#include <stdexcept>

#include "../include/ofApp.hpp"

static void printUsage(std::ostream& out)
{
  out << "Usage: mapd-visualizer <map.map|map.emap> <paths.txt> [tasks.txt]"
         " [--capture-only]\n";
}

int main(int argc, char* argv[])
try {
  if (argc == 2 && std::string(argv[1]) == "--help") {
    printUsage(std::cout);
    return 0;
  }
  if (argc < 3) {
    printUsage(std::cerr);
    return 1;
  }

  bool capture_only = false;
  std::string tasks_file;
  for (int i = 3; i < argc; ++i) {
    const std::string value = argv[i];
    if (value == "--capture-only" && !capture_only) {
      capture_only = true;
    } else if (!value.empty() && value.front() != '-' && tasks_file.empty()) {
      tasks_file = value;
    } else {
      throw std::runtime_error("Unexpected argument: " + value);
    }
  }

  const Graph graph(argv[1]);
  const Solution solution = loadSolution(argv[2], graph);
  const Tasks tasks = tasks_file.empty() ? Tasks()
                                         : loadTasks(tasks_file, graph,
                                                     solution.front().size());

  ofSetupOpenGL(100, 100, OF_WINDOW);
  ofRunApp(new ofApp(&graph, &solution, &tasks, capture_only));
  return 0;
} catch (const std::exception& error) {
  std::cerr << error.what() << '\n';
  return 1;
}
