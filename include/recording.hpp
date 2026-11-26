#pragma once

#include "graph.hpp"

struct Task {
  Vertex* goal;
  int completed_at;
};

using TaskQueue = std::vector<Task>;
using Tasks = std::vector<TaskQueue>;

Solution loadSolution(const std::string& filename, const Graph& graph);
Tasks loadTasks(const std::string& filename, const Graph& graph,
                size_t agent_count);
Vertex* currentGoal(const TaskQueue& queue, int timestep);
