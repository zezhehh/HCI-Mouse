#include "common.h"

#include <string>
#include <cstdlib>
#include <chrono>
#include <cstdio>

#include "layerobject.h"
#include "shader.h"
#include "renderer.h"
#include "imageUtils.h"
#include "dragrectlayer.h"
#include "multimouse.h"
#include "curvelayer.h"

using namespace npnx;

const int num_position_texture = 248;
int image_shift = 0;

class Test_Slider {
public:
  GLFWwindow * window;
  DragRectLayer * targetRect;
  int bgIndex = 0;
  int testCount = 0;
  bool running = false;
  int nbFrames = 0;
  std::chrono::high_resolution_clock::time_point lastClickTime;
  FILE * mousePathFile;
}; 

Test_Slider test_;

void mouse_button_callback(int hDevice, int button, int action, double screenX, double screenY) 
{
  if (hDevice != 0) return;
  if (button == 0x00000001 && action == GLFW_PRESS) {
    test_.lastClickTime = std::chrono::high_resolution_clock::now();
    if (test_.running == true) {
      UCHAR buf[16];
      memset(buf, 0xff, sizeof(buf));
      fwrite(buf, 1, 16, test_.mousePathFile);
      fflush(test_.mousePathFile);
    }
    test_.running = true;
  } 
}

void glfwmouse_button(GLFWwindow *window, int button, int action, int _) 
{
  if (action != GLFW_PRESS) return;
  double x, y;
  glfwGetCursorPos(test_.window, &x, &y); 
  mouse_button_callback(0, 1, GLFW_PRESS, (double)(x - WINDOW_WIDTH / 2) / (WINDOW_WIDTH / 2),
      - (double) (y - WINDOW_HEIGHT / 2) / (WINDOW_HEIGHT / 2));
}
void save_respondtime_to_file(double respondTimer)
{
  freopen("slider.txt","a",stdout);
  printf("%.4lf\n",respondTimer);
  freopen("CON","a",stdout);
  return ;
}
void before_every_frame() 
{
  double x,y;
  multiMouseSystem.GetCursorPos(0, &x, &y);
  if (multiMouseSystem.GetNumMouse() == 0) {
    glfwGetCursorPos(test_.window, &x, &y);
    x = (double)(x - WINDOW_WIDTH / 2) / (WINDOW_WIDTH / 2);
    y = - (double) (y - WINDOW_HEIGHT / 2) / (WINDOW_HEIGHT / 2);
  }
  if (test_.running == true) {
    fwrite(&x, sizeof(double), 1, test_.mousePathFile);
    fwrite(&y, sizeof(double), 1, test_.mousePathFile);
  }

  if (test_.running == true && test_.targetRect->isInside(x, y, test_.nbFrames)) {
    double respondTimer = (double) std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - test_.lastClickTime).count();
    respondTimer /= 1e6;
    // NPNX_LOG(respondTimer);
    save_respondtime_to_file(respondTimer);
    test_.testCount ++;
    NPNX_LOG(test_.testCount);
    NPNX_LOG(test_.bgIndex);
    test_.running = false;
    UCHAR buf[16];
    memset(buf, 0xff, sizeof(buf));
    fwrite(buf, 1, 16, test_.mousePathFile);
    fflush(test_.mousePathFile);
    int k = test_.nbFrames;
    test_.targetRect->textureNoCallback = [&,k] (int) {
      return test_.nbFrames - k > 120 ? 0 : 1;
    };
  }
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
  if (action != GLFW_PRESS) return;
	switch (key) {
	case GLFW_KEY_W:
		test_.bgIndex ++;
    test_.bgIndex %= 3;
    test_.testCount = 0;
		break;
	default:
		break;
	}
}

int main() 
{
  test_.mousePathFile = fopen(NPNX_FETCH_DATA("mouse_path.dat"), "w");
  srand(1); // this guarantee that the random target rectangle will be same every time we run.
  NPNX_LOG(NPNX_DATA_PATH);
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  int monitorCount;
  GLFWmonitor** pMonitor = glfwGetMonitors(&monitorCount);

  int holographic_screen = -1;
  for (int i = 0; i < monitorCount; i++) {
    int screen_x, screen_y;
    const GLFWvidmode * mode = glfwGetVideoMode(pMonitor[i]);
    screen_x = mode->width;
    screen_y = mode->height;
    std::cout << "Screen size is X = " << screen_x << ", Y = " << screen_y << std::endl;
    if (screen_x == WINDOW_WIDTH && screen_y == WINDOW_HEIGHT) {
      holographic_screen = i;
    }
  }
  NPNX_LOG(holographic_screen);

  GLFWwindow* window;
#if (defined __linux__ || defined NPNX_BENCHMARK)
  window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My Title", NULL, NULL);

#else
  if (holographic_screen == -1)
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "My Title", NULL, NULL);
  else
    window = glfwCreateWindow(WINDOW_WIDTH, WINDOW_HEIGHT, "Holographic projection", pMonitor[holographic_screen], NULL);
#endif  

  NPNX_ASSERT(window);
  glfwMakeContextCurrent(window);

  test_.window = window;

  glewExperimental = GL_TRUE;
  GLenum err = glewInit();
  NPNX_ASSERT(!err);

#ifndef NPNX_BENCHMARK
#ifdef __linux__
  if (glxewIsSupported("GLX_MESA_swap_control"))
  {
    printf("OK, we can use GLX_MESA_swap_control\n");
  }
  else
  {
    printf("[WARNING] GLX_MESA_swap_control is NOT supported.\n");
  }
  glXSwapIntervalMESA(1);
  printf("Swap interval: %d\n", glXGetSwapIntervalMESA());
#endif

#ifdef _WIN32
  if (wglewIsSupported("WGL_EXT_swap_control"))
  {
    printf("OK, we can use WGL_EXT_swap_control\n");
  }
  else
  {
    printf("[WARNING] WGL_EXT_swap_control is NOT supported.\n");
  }
  wglSwapIntervalEXT(1);
#endif
#endif

  glViewport(0, 0, WINDOW_WIDTH, WINDOW_HEIGHT);
  glEnable(GL_BLEND);
  glBlendFuncSeparate(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA, GL_ONE, GL_ZERO);

  multiMouseSystem.Init(mouse_button_callback, true);
  glfwSetKeyCallback(window, key_callback);
  glfwSetMouseButtonCallback(window, glfwmouse_button);

  Shader defaultShader;
  defaultShader.LoadShader(NPNX_FETCH_DATA("defaultVertex1.glsl"), NPNX_FETCH_DATA("defaultFragment.glsl"));
  defaultShader.Use();
  glUniform1i(glGetUniformLocation(defaultShader.mShader, "texture0"), 0);
  glUniform1f(glGetUniformLocation(defaultShader.mShader, "xTrans"), 0.0f);
  glUniform1f(glGetUniformLocation(defaultShader.mShader, "yTrans"), 0.0f);

  Shader adjustShader;
  adjustShader.LoadShader(NPNX_FETCH_DATA("defaultVertex.glsl"), NPNX_FETCH_DATA("adjustFragment.glsl"));
  adjustShader.Use();
  glUniform1i(glGetUniformLocation(adjustShader.mShader, "texture0"), 0);
  glUniform1i(glGetUniformLocation(adjustShader.mShader, "rawScreen"), 1);
  glUniform1i(glGetUniformLocation(adjustShader.mShader, "letThrough"), 0);

  unsigned int fbo0, fboColorTex0;
  generateFBO(fbo0, fboColorTex0);

  Renderer renderer(&defaultShader, fbo0);
  Renderer mouseRenderer(&defaultShader, fbo0);
  Renderer postRenderer(&adjustShader, 0);
  Renderer postMouseRenderer(&defaultShader, 0);

  postRenderer.mDefaultTexture.assign({ 0, fboColorTex0 });

  RectLayer bg(-1.0f, -1.0f, 1.0f, 1.0f, -999.0f);
  bg.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA("win.jpg")));
  bg.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA("lion.png")));
  bg.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA("grey_1920_1080.png")));
  bg.textureNoCallback = [&](int _) {return test_.bgIndex; };
  renderer.AddLayer(&bg);

  // RectLayer bgb(-(double)WINDOW_HEIGHT / WINDOW_WIDTH, -1.0f, (double)WINDOW_HEIGHT / WINDOW_WIDTH, 1.0f, -9.0f);
  // bgb.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA("goboard.jpg")));
  // renderer.AddLayer(&bgb);

  
  // std::vector<float> curveControlPoints = {
  //   -0.33125000f, 0.58333333f,
  //   -0.42500000f, 0.43333333f,
  //   -0.16354167f, 0.72962963f,
  //   0.00625000f, 0.54629630f,
  //   -0.05104167f, 0.86296296f,
  //   0.06666667f, 0.23888889f,
  //   0.22187500f, 0.17222222f,
  //   0.11145833f, 0.24074074f,
  //   0.32708333f, 0.01481481f,
  //   -0.09791667f, -0.26481481f,
  //   -0.01354167f, -0.46296296f,
  //   -0.21145833f, -0.06481481f
  // };

std::vector<float> curveControlPoints = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints2 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints3 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints4 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints5 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints6 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints7 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints8 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints9 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};

std::vector<float> curveControlPoints10 = {
  -0.34576667, 0.60090370,
  -0.40710000, 0.50028889,
  -0.28980000, 0.74468889,
  -0.02271250, 0.75321852,
  -0.17911250, 0.74677778,
  0.15410000, 0.76905926,
  0.28606250, 0.50168148,
  0.27571250, 0.64999259,
  0.28817083, 0.32812963,
  -0.00460000, 0.28965926,
  0.16914583, 0.27642963,
  -0.13100417, 0.32151481,
  -0.25606667, 0.10340000,
  -0.27465833, 0.39601852,
  -0.26344583, -0.06910741,
  -0.02961250, -0.04821852,
  -0.17000833, -0.04386667,
  0.09832500, -0.03168148,
  0.29315417, 0.08268518,
  0.23067083, -0.01392593,
  0.34691667, 0.26302593
};


// std::vector<float> curveControlPointsArr[10] = {curveControlPoints, };

  // std::vector<float> curveControlPoints = {
  //   -0.33125000f, 0.6f,
  //   -0.42500000f, 0.6f,
  //   -0.16354167f, 0.6f,
  //   0.325000f, 0.6f,
  //   0.15104167f, 0.6f,
  //   0.56666667f, 0.6f,
  // };
  CurveLayer curve(curveControlPoints, 150.0f, 50.0f);
  curve.mTexture = makeTextureFromImage(NPNX_FETCH_DATA("bar.png"));
  renderer.AddLayer(&curve);
  
  const float targetVSize = 0.2f;
  const float targetHSize = targetVSize * WINDOW_HEIGHT / WINDOW_WIDTH;
  GLuint targetTex = makeTextureFromImage(NPNX_FETCH_DATA("sliderb.png"));
  DragRectLayer sourceRect(-targetHSize / 2, -targetVSize / 2, targetHSize / 2, targetVSize / 2, 100.0f);
  sourceRect.mTexture.push_back(targetTex);
  sourceRect.mATransX = curveControlPoints[0];
  sourceRect.mATransY = curveControlPoints[1];
  renderer.AddLayer(&sourceRect);

  DragRectLayer targetRect(-targetHSize / 2, -targetVSize / 2, targetHSize / 2, targetVSize / 2, 200.0f);
  targetRect.mTexture.push_back(targetTex);
  targetRect.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA("passed.png")));
  int s = curveControlPoints.size();
  targetRect.mATransX = curveControlPoints[s - 6];
  targetRect.mATransY = curveControlPoints[s - 5];
  targetRect.textureNoCallback = [] (int) {return 0;};
  renderer.AddLayer(&targetRect);
  test_.targetRect = &targetRect;
  
  GLuint mouseTex = makeTextureFromImage(NPNX_FETCH_DATA("cursor.png"));
  GLuint mouseWhiteBlockTex = makeTextureFromImage(NPNX_FETCH_DATA("whiteblock.png"));
  GLuint mouseRedBlockTex = makeTextureFromImage(NPNX_FETCH_DATA("redblock.png"));
  for (int i = 0; i < multiMouseSystem.cNumLimit; i++) {
    const float cursorSize = 0.1f;
    RectLayer *cursorLayer = new RectLayer(0.0f, -cursorSize, cursorSize * WINDOW_HEIGHT / WINDOW_WIDTH, 0.0f, *(float *)&i);
    cursorLayer->mTexture.push_back(mouseTex);
    cursorLayer->visibleCallback = [](int) {return false; };
    mouseRenderer.AddLayer(cursorLayer);

    const float blockVSize = 0.15f;
    const float blockHSize = blockVSize * WINDOW_HEIGHT / WINDOW_WIDTH;
    RectLayer *postColor = new RectLayer(-blockHSize / 2, -blockVSize / 2, blockHSize / 2, blockVSize / 2, *(float *)&i);
    postColor->mTexture.push_back(mouseRedBlockTex);
    postColor->mTexture.push_back(mouseWhiteBlockTex);
    postColor->visibleCallback = [](int) {return false; };
    postMouseRenderer.AddLayer(postColor);
  }

  mouseRenderer.Initialize();
  postMouseRenderer.Initialize();


  RectLayer postBaseRect(-1.0, -1.0, 1.0, 1.0, -999.9);
  postBaseRect.mTexture.push_back(0);
  postBaseRect.beforeDraw = [&](const int nbFrames) {
    glUniform1i(glGetUniformLocation(postRenderer.mDefaultShader->mShader, "letThrough"), 1);
    return 0;
  };
  postBaseRect.afterDraw = [&](const int nbFrames) {
    glUniform1i(glGetUniformLocation(postRenderer.mDefaultShader->mShader, "letThrough"), 0);
    return 0;
  };
  postRenderer.AddLayer(&postBaseRect);

  RectLayer postRect(-9.0f / 16.0f, -1.0f, 9.0f / 16.0f, 1.0f, 999.9f);
  for (int i = 0; i < num_position_texture; i++) {
    std::string pos_texture_path = "fremw3_9_";//"fremw2_";
    pos_texture_path += std::to_string(i);
    pos_texture_path += ".png";
    postRect.mTexture.push_back(makeTextureFromImage(NPNX_FETCH_DATA(pos_texture_path)));
  }
  postRect.visibleCallback = [](int) {return true; };
  postRect.textureNoCallback = [=](int nbFrames) {return nbFrames % 62 + image_shift; };
  postRenderer.AddLayer(&postRect);
  
  renderer.Initialize();
  postRenderer.Initialize();

  multiMouseSystem.RegisterPoseMouseRenderer(&postMouseRenderer);
  multiMouseSystem.RegisterMouseRenderer(&mouseRenderer, [&](int) { return true; });
  multiMouseSystem.mEnableAngle = false;

  test_.nbFrames = 0;
  int lastNbFrames = 0;
  double lastTime = glfwGetTime();
  while (!glfwWindowShouldClose(window))
  {
    before_every_frame();

    renderer.Draw(test_.bgIndex);
    mouseRenderer.Draw(test_.nbFrames);
    postRenderer.Draw(test_.nbFrames);
    postMouseRenderer.Draw(test_.nbFrames);
    test_.nbFrames++;
    double thisTime = glfwGetTime();
    double deltaTime = thisTime - lastTime;
    if (deltaTime > 1.0)
    {
      glfwSetWindowTitle(window, std::to_string((test_.nbFrames - lastNbFrames) / deltaTime).c_str());
      lastNbFrames = test_.nbFrames;
      lastTime = thisTime;
    }

    glfwSwapBuffers(window);
    glfwPollEvents();
    multiMouseSystem.PollMouseEvents();
  }
  mouseRenderer.FreeLayers();
  postMouseRenderer.FreeLayers();

  fclose(test_.mousePathFile);
  return 0;

}