#pragma once
#include <memory>
#include <string>
#include <vector>

enum class PortType { NONE, INLET, OUTLET };

struct Vertex {
  const int id;     // index for V in Graph
  const int index;  // index for U, width * y + x, in Graph
  const int x;
  const int y;
  std::vector<Vertex*> neighbor;
  int region = -1;
  char arrow = '\0';
  PortType port = PortType::NONE;

  Vertex(int _id, int _index, int _x, int _y);
};

class Orientation
{
public:
  enum Value {
    NONE = 0,
    X_MINUS = 1,
    X_PLUS = 2,
    Y_MINUS = 3,
    Y_PLUS = 4,
    NUM_ORIENTATIONS = 5
  };

  constexpr Orientation(Value v) : value(v) {}
  constexpr bool operator==(const Orientation::Value& v) const
  {
    return value == v;
  }
  constexpr bool operator==(const Orientation& o) const
  {
    return value == o.value;
  }
  constexpr bool operator!=(const Orientation::Value& v) const
  {
    return value != v;
  }
  constexpr bool operator!=(const Orientation& o) const
  {
    return value != o.value;
  }

  static Orientation from_string(const std::string& s)
  {
    if (s == "X_MINUS") return Orientation::X_MINUS;
    if (s == "X_PLUS") return Orientation::X_PLUS;
    if (s == "Y_MINUS") return Orientation::Y_MINUS;
    if (s == "Y_PLUS") return Orientation::Y_PLUS;
    return Orientation::NONE;
  }
  std::string to_str() const;
  float to_angle() const;

  constexpr operator Value() const { return value; }
  explicit operator bool() const = delete;

private:
  Value value;
};

struct Pose {
  Vertex* v;
  Orientation o;
  Pose(Vertex* _v, Orientation _o) : v(_v), o(_o) {}
  Pose(Vertex* _v) : v(_v), o(Orientation::NONE) {}
  bool operator==(const Pose& p) const { return v == p.v && o == p.o; }
};

using Vertices = std::vector<Vertex*>;
using Config =
    std::vector<Pose>;  // a set of locations and orientations for all agents
using Solution = std::vector<Config>;

struct Graph {
  std::vector<std::unique_ptr<Vertex>> V;
  Vertices U;  // with nullptr
  int width = 0;
  int height = 0;
  std::vector<int> regions;

  explicit Graph(const std::string& filename);
  Vertex* vertexAt(int x, int y) const;
};
