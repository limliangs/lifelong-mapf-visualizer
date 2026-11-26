#include "../include/recording.hpp"

#include <algorithm>
#include <fstream>
#include <regex>
#include <stdexcept>

static Vertex* readPosition(const Graph& graph, const std::smatch& match)
{
  const int x = std::stoi(match[1].str());
  const int y = std::stoi(match[2].str());
  auto* vertex = graph.vertexAt(x, y);
  if (!vertex) {
    const bool outside =
        x < 0 || y < 0 || x >= graph.width || y >= graph.height;
    throw std::runtime_error("position (" + match[1].str() + "," +
                             match[2].str() + ") is " +
                             (outside ? "outside the map" : "on an obstacle"));
  }
  return vertex;
}

Solution loadSolution(const std::string& filename, const Graph& graph)
{
  std::ifstream file(filename);
  if (!file) throw std::runtime_error(filename + ": could not open paths");

  const std::regex row(R"(^\s*(\d+)\s*:\s*)");
  const std::regex pose(
      R"(\(\s*(-?\d+)\s*,\s*(-?\d+)\s*(?:,\s*(X_MINUS|X_PLUS|Y_MINUS|Y_PLUS)\s*)?\)\s*(?:,\s*)?)");
  Solution solution;
  std::string line;
  std::smatch match;
  while (std::getline(file, line)) {
    // Planner exports can include metadata before the solution rows.
    if (!std::regex_search(line, match, row)) continue;
    const auto start = match[0].second;
    try {
      if (std::stoull(match[1].str()) != solution.size()) {
        throw std::runtime_error("timesteps must be consecutive from zero");
      }
      Config config;
      for (auto next = start; next != line.cend(); next = match[0].second) {
        if (!std::regex_search(next, line.cend(), match, pose,
                               std::regex_constants::match_continuous)) {
          throw std::runtime_error("expected a path pose (x,y[,orientation])");
        }
        config.emplace_back(readPosition(graph, match),
                            Orientation::from_string(match[3].str()));
      }
      if (config.empty() ||
          (!solution.empty() && config.size() != solution.front().size())) {
        throw std::runtime_error(
            "every timestep must have the same nonzero agent count");
      }
      solution.push_back(std::move(config));
    } catch (const std::exception& error) {
      throw std::runtime_error(filename + ": paths at timestep " +
                               std::to_string(solution.size()) + ": " +
                               error.what());
    }
  }
  if (solution.empty()) {
    throw std::runtime_error(filename + ": no path timesteps found");
  }
  return solution;
}

Tasks loadTasks(const std::string& filename, const Graph& graph,
                size_t agent_count)
{
  std::ifstream file(filename);
  const auto error = [&filename](const std::string& message) {
    return std::runtime_error(filename + ": " + message);
  };
  if (!file) throw error("could not open tasks");

  const std::regex count(R"(\s*(\d+)\s*)");
  const std::regex entry(
      R"(\s*\(\s*(-?\d+)\s*,\s*(-?\d+)\s*\)\s*,\s*(-?\d+)\s*,\s*;\s*)");
  const std::regex summary(R"(\s*Total completed tasks:\s*(\d+)\s*)");
  const auto blank = [](const std::string& line) {
    return line.find_first_not_of(" \t\r") == std::string::npos;
  };

  std::smatch match;
  std::string line;
  if (!std::getline(file, line) || !std::regex_match(line, match, count) ||
      std::stoull(match[1].str()) != agent_count) {
    throw error("agent count must match paths (" + std::to_string(agent_count) +
                ")");
  }
  Tasks tasks(agent_count);
  size_t completed = 0;
  for (size_t agent = 0; agent < agent_count; ++agent) {
    if (!std::getline(file, line))
      throw error("expected one task row per agent");
    if (blank(line)) continue;
    auto& queue = tasks[agent];
    try {
      for (auto next = line.cbegin(); next != line.cend();
           next = match[0].second) {
        if (!std::regex_search(next, line.cend(), match, entry,
                               std::regex_constants::match_continuous)) {
          throw std::runtime_error(
              "expected a task entry (x,y),completion_time,;");
        }
        const int time = std::stoi(match[3].str());
        if (time < -1 || (time >= 0 && !queue.empty() &&
                          (queue.back().completed_at == -1 ||
                           time < queue.back().completed_at))) {
          throw std::runtime_error(
              "completion times must be ordered, followed by pending (-1) "
              "goals");
        }
        queue.push_back({readPosition(graph, match), time});
        if (time >= 0) ++completed;
      }
    } catch (const std::exception& cause) {
      throw error("agent " + std::to_string(agent) + ": " + cause.what());
    }
  }

  bool has_summary = false;
  while (std::getline(file, line)) {
    if (blank(line)) continue;
    if (has_summary || !std::regex_match(line, match, summary)) {
      throw error("unexpected data after the task rows");
    }
    if (std::stoull(match[1].str()) != completed) {
      throw error("Total completed tasks does not match task records");
    }
    has_summary = true;
  }
  return tasks;
}

Vertex* currentGoal(const TaskQueue& queue, int timestep)
{
  // The next queued goal is active on the completion timestep itself. A
  // lookup from the displayed time also handles seeking backwards and loops.
  const auto active = std::partition_point(
      queue.begin(), queue.end(), [timestep](const Task& task) {
        return task.completed_at >= 0 && task.completed_at <= timestep;
      });
  return active == queue.end() ? nullptr : active->goal;
}
