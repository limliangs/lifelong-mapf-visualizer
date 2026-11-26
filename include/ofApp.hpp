#pragma once

#include "ofMain.h"
#include "ofxGui.h"
#include "recording.hpp"

class ofApp : public ofBaseApp
{
public:
  const Graph* G;
  const Solution* P;  // plan
  const int N;        // number of agents
  const int T;        // makespan
  const Tasks* tasks;
  Config goals;
  const bool has_sidebar;
  int goal_page = 0;

  // size
  const int map_top;
  const int scale;
  const float agent_rad;
  const float goal_rad;
  const int font_size;

  // flg
  bool flg_capture_only;
  bool flg_autoplay;
  bool flg_loop;
  bool flg_goal;
  bool flg_font;
  bool flg_snapshot;
  bool flg_zoomout;
  bool flg_zoomin;
  bool flg_grid;

  enum struct LINE_MODE { STRAIGHT, PATH, NONE, NUM };
  LINE_MODE line_mode;

  // font
  ofTrueTypeFont font;

  // gui
  ofxFloatSlider timestep_slider;
  ofxFloatSlider speed_slider;
  ofxPanel gui;

  // camera
  ofEasyCam cam;

  void setup();
  void update();
  void draw();
  void drawMap();
  void drawSidebar(float x, int timestep);
  int goalsTop() const;
  int goalRowsPerPage() const;

  void keyPressed(int key);
  void keyReleased(int key);
  void mouseMoved(int x, int y);
  void mouseDragged(int x, int y, int button);
  void mousePressed(int x, int y, int button);
  void mouseReleased(int x, int y, int button);
  void mouseEntered(int x, int y);
  void mouseExited(int x, int y);
  void windowResized(int w, int h);
  void dragEvent(ofDragInfo dragInfo);
  void gotMessage(ofMessage msg);

  ofApp(const Graph* _G, const Solution* _P, const Tasks* _tasks,
        bool flg_capture_only = false);
};
