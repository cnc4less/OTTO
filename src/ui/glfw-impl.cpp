#define GLFW_INCLUDE_ES3
#define GLFW_INCLUDE_GLEXT
#include <GLFW/glfw3.h>
#include <nanocanvas/NanoCanvas.h>
#define NANOVG_GLES3_IMPLEMENTATION
#include <nanovg/nanovg_gl.h>
#include <nanovg/nanovg_gl_utils.h>

#include "base.h"
#include "mainui.h"
#include "utils.h"
#include "../globals.h"

#include <thread>
#include <mutex>
#include <condition_variable>
#include <chrono>
#include <string>
#include <algorithm>


static ui::Key keyboardKey(int xKey) {
  using namespace ui;
  switch (xKey) {

    // Rotaries
  case GLFW_KEY_Q:     return K_RED_UP;
  case GLFW_KEY_A:     return K_RED_DOWN;
  case GLFW_KEY_W:     return K_BLUE_UP;
  case GLFW_KEY_S:     return K_BLUE_DOWN;
  case GLFW_KEY_E:     return K_WHITE_UP;
  case GLFW_KEY_D:     return K_WHITE_DOWN;
  case GLFW_KEY_R:     return K_GREEN_UP;
  case GLFW_KEY_F:     return K_GREEN_DOWN;

  case GLFW_KEY_LEFT:  return K_LEFT;
  case GLFW_KEY_RIGHT:  return K_RIGHT;

    // Tapedeck
  case GLFW_KEY_SPACE: return K_PLAY;
  case GLFW_KEY_Z:     return K_REC;
  case GLFW_KEY_F1:    return K_TRACK_1;
  case GLFW_KEY_F2:    return K_TRACK_2;
  case GLFW_KEY_F3:    return K_TRACK_3;
  case GLFW_KEY_F4:    return K_TRACK_4;

    // Numbers
  case GLFW_KEY_1:     return K_1;
  case GLFW_KEY_2:     return K_2;
  case GLFW_KEY_3:     return K_3;
  case GLFW_KEY_4:     return K_4;
  case GLFW_KEY_5:     return K_5;
  case GLFW_KEY_6:     return K_6;
  case GLFW_KEY_7:     return K_7;
  case GLFW_KEY_8:     return K_8;
  case GLFW_KEY_9:     return K_9;
  case GLFW_KEY_0:     return K_0;

  default:       return K_NONE;
  }
}


static void key(GLFWwindow* window, int key, int scancode, int action, int mods) {
  MainUI& self = MainUI::getInstance();
  if (action == GLFW_PRESS) {
    self.keypress(keyboardKey(key));
  } else if (action == GLFW_RELEASE) {
    self.keyrelease(keyboardKey(key));
  }
}

static void render_routine() {
  MainUI& self = MainUI::getInstance();

  GLFWwindow* window;
	NVGcontext* vg = NULL;
	double prevt = 0;

	if (!glfwInit()) {
		printf("Failed to init GLFW.");
	}

	//glfwSetErrorCallback(errorcb);

	glfwWindowHint(GLFW_CLIENT_API, GLFW_OPENGL_ES_API);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 0);

	window = glfwCreateWindow(drawing::WIDTH, drawing::HEIGHT, "TOP-1", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return;
	}

	glfwSetKeyCallback(window, key);

	glfwMakeContextCurrent(window);

	vg = nvgCreateGLES3(NVG_ANTIALIAS | NVG_STENCIL_STROKES | NVG_DEBUG);
	if (vg == NULL) {
		printf("Could not init nanovg.\n");
		return;
	}

	glfwSwapInterval(0);

	glfwSetTime(0);
	prevt = glfwGetTime();

	while (!glfwWindowShouldClose(window))
	{
		double mx, my, t, dt, spent;
		int winWidth, winHeight;
		int fbWidth, fbHeight;
		float pxRatio;
    float scale;

		t = glfwGetTime();
		dt = t - prevt;
		prevt = t;

		glfwGetCursorPos(window, &mx, &my);
		glfwGetWindowSize(window, &winWidth, &winHeight);
		glfwGetFramebufferSize(window, &fbWidth, &fbHeight);
		// Calculate pixel ration for hi-dpi devices.
		pxRatio = (float)fbWidth / (float)winWidth;
    scale = std::min((float)winWidth/(float)drawing::WIDTH, (float)winHeight/(float)drawing::HEIGHT);
    NanoCanvas::Canvas canvas(vg, winWidth, winHeight);
    drawing::initUtils(canvas);

		// Update and render
		glViewport(0, 0, fbWidth, fbHeight);

    glClearColor(0,0,0,0);
		glClear(GL_COLOR_BUFFER_BIT|GL_DEPTH_BUFFER_BIT|GL_STENCIL_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_CULL_FACE);
		glDisable(GL_DEPTH_TEST);

    canvas.clearColor(drawing::COLOR_BLACK);
    canvas.begineFrame(winWidth, winHeight);

    canvas.scale(scale, scale);
    self.draw(canvas);

    canvas.endFrame();

		glEnable(GL_DEPTH_TEST);

		glfwSwapBuffers(window);

    spent = glfwGetTime() - t;

		glfwPollEvents();

    std::this_thread::sleep_for(
      std::chrono::milliseconds(int(100/6 - spent*1000)));

	}

	nvgDeleteGLES3(vg);

	glfwTerminate();

  GLOB.running = false;
}

void MainUI::mainRoutine() {
  std::thread renderThread = std::thread(render_routine);

  while (GLOB.running);
  renderThread.join();
}