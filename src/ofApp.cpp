#include "../include/ofApp.hpp"

#include "../include/param.hpp"

static int get_scale(const Graph* G, bool sidebar, int map_top)
{
  auto window_max_w = default_screen_width - screen_x_buffer * 2 -
                      window_x_buffer * 2 - (sidebar ? sidebar_width : 0);
  auto window_max_h = default_screen_height - map_top - window_y_bottom_buffer;
  return std::min(window_max_w / G->width, window_max_h / G->height) + 1;
}

static void printKeys(bool show_tasks)
{
  std::cout << "keys for visualizer" << std::endl;
  std::cout << "- p : play or pause" << std::endl;
  std::cout << "- l : loop or not" << std::endl;
  std::cout << "- r : reset" << std::endl;
  std::cout << "- v : show virtual line to goals" << std::endl;
  std::cout << "- f : show agent & node id" << std::endl;
  std::cout << "- g : show goals" << std::endl;
  if (show_tasks) {
    std::cout << "- left/right : pause and step one timestep\n"
              << "- [ / ] : page through current goals\n";
  } else {
    std::cout << "- right : progress\n"
              << "- left  : back\n";
  }
  std::cout << "- up    : speed up" << std::endl;
  std::cout << "- down  : speed down" << std::endl;
  std::cout << "- i : toggle zoom in" << std::endl;
  std::cout << "- o : toggle zoom out" << std::endl;
  std::cout << "- G : toggle gridlines" << std::endl;
  std::cout << "- space : screenshot (saved in Desktop)" << std::endl;
  std::cout << "- esc : terminate" << std::endl;
}

ofApp::ofApp(const Graph* _G, const Solution* _P, const Tasks* _tasks,
             bool _flg_capture_only)
    : G(_G),
      P(_P),
      N(P->front().size()),
      T(P->size() - 1),
      tasks(_tasks),
      goals(P->back()),
      has_sidebar(!G->regions.empty() || !tasks->empty()),
      map_top(has_sidebar ? sidebar_map_top : window_y_top_buffer),
      scale(get_scale(G, has_sidebar, map_top)),
      agent_rad(scale / std::sqrt(2) / 2),
      goal_rad(scale / 4.0),
      font_size(std::max(scale / 8, 6)),
      flg_capture_only(_flg_capture_only),
      flg_autoplay(true),
      flg_loop(true),
      flg_goal(true),
      flg_font(false),
      flg_snapshot(flg_capture_only),
      flg_zoomout(false),
      flg_zoomin(false),
      flg_grid(true),
      line_mode(flg_capture_only ? LINE_MODE::PATH : LINE_MODE::STRAIGHT)
{
}

void ofApp::setup()
{
  auto w = G->width * scale + 2 * window_x_buffer;
  auto h = G->height * scale + map_top + window_y_bottom_buffer;
  if (has_sidebar) {
    w = std::max(w, sidebar_width);
    h = std::max(h, 340);
  }
  ofSetWindowShape(w + (has_sidebar ? sidebar_width : 0), h);
  ofSetWindowTitle(has_sidebar ? "MAPD Visualizer" : "MAPF Visualizer");
  ofBackground(Color::bg);
  ofSetCircleResolution(32);
  ofSetFrameRate(30);
  font.load("MuseoModerno-VariableFont_wght.ttf", font_size, true, false, true);

  // setup gui
  gui.setup();
  gui.add(timestep_slider.setup("time step", 0, 0, T));
  gui.add(speed_slider.setup("speed", 0.1, 0, 1));

  cam.setVFlip(true);
  if (has_sidebar) {
    cam.setAutoDistance(false);
    const auto distance = cam.getImagePlaneDistance(ofRectangle(0, 0, w, h));
    cam.setGlobalPosition(ofVec3f(w / 2.f, h / 2.f, distance));
  } else {
    cam.setGlobalPosition(ofVec3f(w / 2, h / 2 - map_top / 2, 580));
  }
  cam.removeAllInteractions();
  cam.addInteraction(ofEasyCam::TRANSFORM_TRANSLATE_XY, OF_MOUSE_BUTTON_LEFT);

  if (!flg_capture_only) printKeys(!tasks->empty());
}

void ofApp::update()
{
  if (flg_autoplay && speed_slider > 0) {
    const float t = timestep_slider;
    timestep_slider =
        t == T && flg_loop ? 0.f : std::min<float>(T, t + speed_slider);
  }

  if (flg_zoomout) {
    auto campos = cam.getGlobalPosition();
    campos.z = campos.z * 1.01;
    cam.setGlobalPosition(campos);
  }
  if (flg_zoomin) {
    auto campos = cam.getGlobalPosition();
    campos.z = std::max(campos.z * 0.99, 50.0);
    cam.setGlobalPosition(campos);
  }
}

static void drawPort(const Vertex& vertex, float x, float y, int scale)
{
  const auto color =
      vertex.port == PortType::INLET ? Color::inlet : Color::outlet;
  const float center_x = x + scale / 2.f;
  const float center_y = y + scale * (scale >= 32 ? 0.38f : 0.5f);
  const float radius = scale * 0.25f;
  ofSetColor(color);
  ofDrawRectangle(x + scale * 0.08f, y + scale * 0.08f, scale * 0.84f,
                  scale * 0.84f);

  ofSetColor(255);
  ofPushMatrix();
  ofTranslate(center_x, center_y);
  if (vertex.arrow == '<') ofRotateZDeg(180);
  if (vertex.arrow == '^') ofRotateZDeg(-90);
  if (vertex.arrow == 'v') ofRotateZDeg(90);
  ofDrawRectangle(-radius, -radius * 0.2f, radius * 1.3f, radius * 0.4f);
  ofDrawTriangle(0, -radius * 0.65f, 0, radius * 0.65f, radius, 0);
  ofPopMatrix();

  if (scale >= 32) {
    const bool inlet = vertex.port == PortType::INLET;
    ofDrawBitmapString(inlet ? "IN" : "OUT", center_x - (inlet ? 8 : 12),
                       y + scale * 0.82f);
  }
}

void ofApp::drawMap()
{
  ofSetLineWidth(1);
  ofFill();
  const float grid_gap = flg_grid ? 0.3f : 0.f;
  for (const auto& vertex : G->V) {
    const float x = vertex->x * scale + window_x_buffer;
    const float y = vertex->y * scale + map_top;
    ofSetColor(vertex->region < 0 ? Color::node
                                  : Color::regions[vertex->region]);
    ofDrawRectangle(x - grid_gap / 2, y - grid_gap / 2, scale - grid_gap,
                    scale - grid_gap);
    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(vertex->index), x + 1, y + font_size + 1);
    }
  }

  if (G->regions.empty()) return;
  ofSetLineWidth(2);
  ofSetColor(Color::region_edge);
  for (const auto& vertex : G->V) {
    const float x = vertex->x * scale + window_x_buffer;
    const float y = vertex->y * scale + map_top;
    const auto* right = G->vertexAt(vertex->x + 1, vertex->y);
    const auto* below = G->vertexAt(vertex->x, vertex->y + 1);
    if (right && right->region != vertex->region) {
      ofDrawLine(x + scale, y, x + scale, y + scale);
    }
    if (below && below->region != vertex->region) {
      ofDrawLine(x, y + scale, x + scale, y + scale);
    }
  }
  ofSetLineWidth(1);
  for (const auto& vertex : G->V) {
    if (vertex->port != PortType::NONE) {
      drawPort(*vertex, vertex->x * scale + window_x_buffer,
               vertex->y * scale + map_top, scale);
    }
  }
}

int ofApp::goalsTop() const
{
  const int region_rows = (G->regions.size() + 1) / 2;
  return region_rows == 0 ? 24 : 98 + 20 * region_rows;
}

int ofApp::goalRowsPerPage() const
{
  return std::max(1, (ofGetHeight() - goalsTop() - 45) / 20);
}

void ofApp::drawSidebar(float x, int timestep)
{
  ofFill();
  ofSetColor(15, 23, 42);
  ofDrawRectangle(x, 0, ofGetWidth() - x, ofGetHeight());
  x += 14;
  ofSetColor(Color::font_info);
  if (!G->regions.empty()) {
    ofDrawBitmapString("Regions", x, 24);
    for (size_t i = 0; i < G->regions.size(); ++i) {
      const int region = G->regions[i];
      const float column = x + (i % 2) * 94;
      const int y = 34 + (i / 2) * 20;
      ofSetColor(Color::regions[region]);
      ofDrawRectangle(column, y, 12, 12);
      ofSetColor(Color::font_info);
      ofDrawBitmapString("Region " + std::to_string(region), column + 19,
                         y + 10);
    }

    const int port_y = goalsTop() - 38;
    ofSetColor(Color::inlet);
    ofDrawRectangle(x, port_y - 10, 12, 12);
    ofSetColor(Color::font_info);
    ofDrawBitmapString("IN  Inlet", x + 19, port_y);
    ofSetColor(Color::outlet);
    ofDrawRectangle(x + 100, port_y - 10, 12, 12);
    ofSetColor(Color::font_info);
    ofDrawBitmapString("OUT Outlet", x + 119, port_y);
  }

  if (tasks->empty()) return;
  const int top = goalsTop();
  ofSetColor(Color::font_info);
  ofDrawBitmapString("Current goals", x, top);
  ofSetColor(148, 163, 184);
  ofDrawBitmapString("t=" + std::to_string(timestep), x + 132, top);
  const int rows = goalRowsPerPage();
  const int last_page = (N - 1) / rows;
  goal_page = std::min(goal_page, last_page);
  for (int agent = goal_page * rows;
       agent < N && agent < (goal_page + 1) * rows; ++agent) {
    const int y = top + 22 + (agent % rows) * 20;
    ofSetColor(Color::agents[agent % Color::agents.size()]);
    ofDrawCircle(x + 5, y - 4, 5);
    ofSetColor(Color::font_info);
    ofDrawBitmapString("Agent " + std::to_string(agent), x + 19, y);
    const auto* goal = goals[agent].v;
    ofDrawBitmapString(goal ? "(" + std::to_string(goal->x) + ", " +
                                  std::to_string(goal->y) + ")"
                            : "None",
                       x + 112, y);
  }
  if (last_page > 0) {
    ofSetColor(148, 163, 184);
    ofDrawBitmapString("[ / ]  Page " + std::to_string(goal_page + 1) + "/" +
                           std::to_string(last_page + 1),
                       x, ofGetHeight() - 14);
  }
}

void ofApp::draw()
{
  const int t1 = static_cast<int>(timestep_slider);
  const int t2 = t1 + 1;
  if (!tasks->empty()) {
    for (int i = 0; i < N; ++i) goals[i].v = currentGoal((*tasks)[i], t1);
  }

  const int map_width =
      std::max(1, ofGetWidth() - (has_sidebar ? sidebar_width : 0));
  cam.begin(ofRectangle(0, 0, map_width, ofGetHeight()));
  if (flg_snapshot) {
    ofBeginSaveScreenAsPDF(ofFilePath::getUserHomeDir() +
                               "/Desktop/screenshot-" + ofGetTimestampString() +
                               ".pdf",
                           false);
  }

  drawMap();

  // draw goals
  if (flg_goal && tasks->empty()) {
    for (int i = 0; i < N; ++i) {
      ofSetColor(Color::agents[i % Color::agents.size()]);
      auto g = goals[i].v;
      auto o = goals[i].o;
      int x = g->x * scale + window_x_buffer + scale / 2;
      int y = g->y * scale + map_top + scale / 2;
      ofDrawRectangle(x - goal_rad / 2, y - goal_rad / 2, goal_rad, goal_rad);

      if (o != Orientation::NONE) {
        ofSetColor(255, 255, 255);
        ofPushMatrix();
        ofTranslate(x, y);
        ofRotateZDeg(o.to_angle());
        ofDrawTriangle(0, goal_rad / 2, 0, -goal_rad / 2, goal_rad / 2, 0);
        ofPopMatrix();
      }
    }
  }

  // draw agents
  for (int i = 0; i < N; ++i) {
    ofSetColor(Color::agents[i % Color::agents.size()]);

    // agent position
    auto p_t1 = P->at(t1)[i];
    auto v = p_t1.v;
    auto o = p_t1.o;
    float x = v->x;
    float y = v->y;
    float angle = o.to_angle();

    if (t2 <= T) {
      auto p_t2 = P->at(t2)[i];
      auto u = p_t2.v;
      x += (u->x - x) * (timestep_slider - t1);
      y += (u->y - y) * (timestep_slider - t1);

      if (o != Orientation::NONE && p_t2.o != Orientation::NONE) {
        float angle_next = p_t2.o.to_angle();
        float diff = angle_next - angle;
        if (diff > 180.0f) diff -= 360.0f;
        if (diff < -180.0f) diff += 360.0f;
        angle += diff * (timestep_slider - t1);
      }
    }
    x *= scale;
    y *= scale;
    x += window_x_buffer + scale / 2;
    y += map_top + scale / 2;

    ofDrawCircle(x, y, agent_rad);

    // goal
    const auto* goal = goals[i].v;
    if (line_mode == LINE_MODE::STRAIGHT && goal) {
      ofDrawLine(goal->x * scale + window_x_buffer + scale / 2,
                 goal->y * scale + map_top + scale / 2, x, y);
    } else if (line_mode == LINE_MODE::PATH) {
      // next loc
      ofSetLineWidth(2);
      if (t2 <= T) {
        auto u = P->at(t2)[i].v;
        ofDrawLine(x, y, u->x * scale + window_x_buffer + scale / 2,
                   u->y * scale + map_top + scale / 2);
      }
      for (int t = t1 + 1; t < T; ++t) {
        auto v_from = P->at(t)[i].v;
        auto v_to = P->at(t + 1)[i].v;
        if (v_from == v_to) continue;
        ofDrawLine(v_from->x * scale + window_x_buffer + scale / 2,
                   v_from->y * scale + map_top + scale / 2,
                   v_to->x * scale + window_x_buffer + scale / 2,
                   v_to->y * scale + map_top + scale / 2);
      }
      ofSetLineWidth(1);
    }

    // agent at goal
    if (tasks->empty() ? p_t1 == goals[i] : p_t1.v == goal) {
      ofSetColor(255, 255, 255);
      ofDrawCircle(x, y, agent_rad * 0.7);
    }

    // agent orientation
    if (o != Orientation::NONE) {
      ofSetColor(255, 255, 255);
      ofPushMatrix();
      ofTranslate(x, y);
      ofRotateZDeg(angle);
      ofDrawTriangle(0, agent_rad, 0, -agent_rad, agent_rad, 0);
      ofPopMatrix();
    }

    // id
    if (flg_font) {
      ofSetColor(Color::font);
      font.drawString(std::to_string(i), x - font_size / 2, y + font_size / 2);
    }
  }

  if (flg_goal && !tasks->empty()) {
    for (int i = 0; i < N; ++i) {
      const auto* goal = goals[i].v;
      if (!goal) continue;
      const float x = goal->x * scale + window_x_buffer;
      const float y = goal->y * scale + map_top;
      const auto label = "G" + std::to_string(i);
      const float label_width = label.size() * 8 + 6;
      ofSetColor(Color::agents[i % Color::agents.size()]);
      if (scale >= 24 && label_width <= scale) {
        ofDrawRectangle(x + scale - label_width, y + 1, label_width, 14);
        ofSetColor(0);
        ofDrawBitmapString(label, x + scale - label_width + 3, y + 12);
      } else {
        ofDrawRectangle(x + scale - goal_rad, y, goal_rad, goal_rad);
      }
    }
  }

  if (flg_snapshot) {
    ofEndSaveScreenAsPDF();
    flg_snapshot = false;
    if (flg_capture_only) std::exit(0);
  }

  cam.end();
  if (has_sidebar) drawSidebar(map_width, t1);
  gui.draw();
}

void ofApp::keyPressed(int key)
{
  if (key == 'r') timestep_slider = 0;  // reset
  if (key == 'p') flg_autoplay = !flg_autoplay;
  if (key == 'l') flg_loop = !flg_loop;
  if (key == 'g') flg_goal = !flg_goal;
  if (key == 'f') {
    flg_font = !flg_font;
    flg_font &= (scale - font_size > 6);
  }
  if (key == 32) flg_snapshot = true;  // space
  if (key == 'v') {
    line_mode =
        static_cast<LINE_MODE>(((int)line_mode + 1) % (int)LINE_MODE::NUM);
  }
  if (!tasks->empty()) {
    if (key == '[') goal_page = std::max(goal_page - 1, 0);
    if (key == ']')
      goal_page = std::min(goal_page + 1, (N - 1) / goalRowsPerPage());
    if (key == OF_KEY_LEFT || key == OF_KEY_RIGHT) {
      flg_autoplay = false;
      const int step = static_cast<int>(timestep_slider);
      timestep_slider =
          key == OF_KEY_LEFT ? std::max(0, step - 1) : std::min(T, step + 1);
      return;
    }
  }
  float t;
  if (key == OF_KEY_RIGHT) {
    t = timestep_slider + speed_slider;
    timestep_slider = std::min((float)T, t);
  }
  if (key == OF_KEY_LEFT) {
    t = timestep_slider - speed_slider;
    timestep_slider = std::max((float)0, t);
  }
  if (key == OF_KEY_UP) {
    t = speed_slider + 0.001;
    speed_slider = std::min(t, (float)speed_slider.getMax());
  }
  if (key == OF_KEY_DOWN) {
    t = speed_slider - 0.001;
    speed_slider = std::max(t, (float)speed_slider.getMin());
  }
  if (key == 'i') {
    flg_zoomin = !flg_zoomin;
  }
  if (key == 'o') {
    flg_zoomout = !flg_zoomout;
  }
  if (key == 'G') {
    flg_grid = !flg_grid;
  }
}

void ofApp::keyReleased(int key) {}

void ofApp::mouseMoved(int x, int y) {}

void ofApp::mouseDragged(int x, int y, int button) {}

void ofApp::mousePressed(int x, int y, int button) {}

void ofApp::mouseReleased(int x, int y, int button) {}

void ofApp::mouseEntered(int x, int y) {}

void ofApp::mouseExited(int x, int y) {}

void ofApp::windowResized(int w, int h) {}

void ofApp::gotMessage(ofMessage msg) {}

void ofApp::dragEvent(ofDragInfo dragInfo) {}
