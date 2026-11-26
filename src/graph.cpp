#include "../include/graph.hpp"

#include <cmath>
#include <fstream>
#include <limits>
#include <regex>
#include <set>
#include <stdexcept>

Vertex::Vertex(int _id, int _index, int _x, int _y)
    : id(_id), index(_index), x(_x), y(_y), neighbor(Vertices())
{
}

Vertex* Graph::vertexAt(int x, int y) const
{
  if (x < 0 || y < 0 || x >= width || y >= height) return nullptr;
  return U[width * y + x];
}

Graph::Graph(const std::string& filename)
{
  std::ifstream file(filename);
  const auto error = [&filename](const std::string& message) {
    return std::runtime_error(filename + ": " + message);
  };
  if (!file) throw error("could not open map");

  std::string line;
  const auto readLine = [&]() {
    line.clear();
    if (!std::getline(file, line)) return false;
    if (!line.empty() && line.back() == '\r') line.pop_back();
    return true;
  };
  const auto readRow = [&](const std::string& block, int y) {
    if (!readLine() || line.size() != static_cast<size_t>(width)) {
      throw error(block + " row " + std::to_string(y + 1) +
                  " does not match the declared dimensions");
    }
  };

  const std::regex r_height(R"(height\s+(\d+)\s*)");
  const std::regex r_width(R"(width\s+(\d+)\s*)");
  std::smatch match;
  while (readLine() && line != "map") {
    if (std::regex_match(line, match, r_height)) {
      height = std::stoi(match[1].str());
    }
    if (std::regex_match(line, match, r_width)) {
      width = std::stoi(match[1].str());
    }
  }
  if (line != "map" || width <= 0 || height <= 0 ||
      height > std::numeric_limits<int>::max() / width) {
    throw error("expected valid height, width, and map headers");
  }

  U.resize(width * height, nullptr);
  for (int y = 0; y < height; ++y) {
    readRow("map", y);
    for (int x = 0; x < width; ++x) {
      const char terrain = line[x];
      if (std::string("@OTW").find(terrain) != std::string::npos) continue;
      if (std::string(".GS<>^v").find(terrain) == std::string::npos) {
        throw error("unknown terrain at (" + std::to_string(x) + "," +
                    std::to_string(y) + ")");
      }
      const int index = width * y + x;
      V.push_back(std::make_unique<Vertex>(V.size(), index, x, y));
      U[index] = V.back().get();
      if (std::string("<>^v").find(terrain) != std::string::npos) {
        U[index]->arrow = terrain;
      }
    }
  }

  while (readLine() && line.empty()) {
  }
  if (!line.empty()) {
    if (line != "regions")
      throw error("expected a regions block after the map");
    std::set<int> ids;
    for (int y = 0; y < height; ++y) {
      readRow("regions", y);
      for (int x = 0; x < width; ++x) {
        auto* vertex = vertexAt(x, y);
        if (!vertex) continue;
        if (line[x] < '0' || line[x] > '9') {
          throw error("expected a region digit at (" + std::to_string(x) + "," +
                      std::to_string(y) + ")");
        }
        vertex->region = line[x] - '0';
        ids.insert(vertex->region);
      }
    }
    regions.assign(ids.begin(), ids.end());
    while (readLine()) {
      if (!line.empty()) throw error("unexpected data after the regions block");
    }
  }

  for (const auto& vertex : V) {
    const int x = vertex->x;
    const int y = vertex->y;
    for (auto* neighbor : {vertexAt(x - 1, y), vertexAt(x + 1, y),
                           vertexAt(x, y + 1), vertexAt(x, y - 1)}) {
      if (neighbor) vertex->neighbor.push_back(neighbor);
    }
    if (!vertex->arrow) continue;

    const int dx = (vertex->arrow == '>') - (vertex->arrow == '<');
    const int dy = (vertex->arrow == 'v') - (vertex->arrow == '^');
    const auto isPeer = [&](const Vertex* other) {
      return other && vertex->region >= 0 && other->region >= 0 &&
             other->region != vertex->region && other->arrow == vertex->arrow;
    };
    const bool forward = isPeer(vertexAt(x + dx, y + dy));
    const bool backward = isPeer(vertexAt(x - dx, y - dy));
    if (forward == backward) {
      throw error("port at (" + std::to_string(x) + "," + std::to_string(y) +
                  ") needs exactly one same-arrow peer in another region");
    }
    vertex->port = forward ? PortType::OUTLET : PortType::INLET;
  }
}

std::string Orientation::to_str() const
{
  switch (value) {
    case X_MINUS:
      return "X_MINUS";
    case X_PLUS:
      return "X_PLUS";
    case Y_MINUS:
      return "Y_MINUS";
    case Y_PLUS:
      return "Y_PLUS";
    default:
      return "NONE";
  }
}

float Orientation::to_angle() const
{
  switch (value) {
    case X_MINUS:
      return 180;
    case X_PLUS:
      return 0;
    case Y_MINUS:
      return 270;
    case Y_PLUS:
      return 90;
    default:
      return NAN;
  }
}
