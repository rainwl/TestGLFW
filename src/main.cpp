#include "stb_image.h"
#include "../Shader.h"

#include <GLFW/glfw3.h>
#include <glad/glad.h>

#include <iostream>
#include <windows.h>
#include<filesystem>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

#include "chrono"
#ifdef _WIN32
#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>
#endif

std::chrono::time_point<std::chrono::steady_clock> GetTime();
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void processInput(GLFWwindow *window);
void check_success(const unsigned int shader);
void check_success_program(const unsigned int program);
float CalculateLoadingPercentage(const std::chrono::time_point<std::chrono::steady_clock> &start_time, int duration_seconds);
    // settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
GLFWmonitor *monitor;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance, LPSTR lpCmdLine, int nShowCmd) {
#pragma region glfw: initialize and configure
  glfwInit();
  monitor = glfwGetPrimaryMonitor();
  const GLFWvidmode *mode = glfwGetVideoMode(monitor);
  // Set the window to a full-screen window without borders
  glfwWindowHint(GLFW_RED_BITS, mode->redBits);
  glfwWindowHint(GLFW_GREEN_BITS, mode->greenBits);
  glfwWindowHint(GLFW_BLUE_BITS, mode->blueBits);
  glfwWindowHint(GLFW_REFRESH_RATE, mode->refreshRate);
  glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);// no borders


  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);


#pragma endregion

#pragma region glfw window creation

  GLFWwindow *window = glfwCreateWindow(mode->width, mode->height, "Loading", NULL, NULL);
  if (window == NULL) {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  glfwSwapInterval(0);
  glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_HIDDEN);// 隐藏鼠标光标

  #ifdef _WIN32
  // 仅当在Windows平台上时尝试设置窗口为最上层
  HWND hwnd = glfwGetWin32Window(window);
  SetWindowPos(hwnd, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
#endif

#pragma endregion

#pragma region glad: load all OpenGL function pointers

  if (!gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress))) {
    // NOLINT(clang-diagnostic-cast-function-type-strict)
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }
#pragma endregion

#pragma region build and compile our shader program

  Shader our_shader(".\\shader\\shader.vs", ".\\shader\\shader.fs");

#pragma endregion

#pragma region set up vertex data (and buffer(s)) and configure vertex attributes

  // set up vertex data (and buffer(s)) and configure vertex attributes
  // ------------------------------------------------------------------
  float vertices[] = {
      // positions          // colors           // texture coords
      1, 1, 0.0f, 1.0f, 0.0f, 0.0f, 1.0f, 1.0f,  // top right
      1, -1, 0.0f, 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, // bottom right
      -1, -1, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f,// bottom left
      -1, 1, 0.0f, 1.0f, 1.0f, 0.0f, 0.0f, 1.0f  // top left
  };
  unsigned int indices[] = {
      0, 1, 3,// first triangle
      1, 2, 3 // second triangle
  };

  unsigned int VBO, VAO, EBO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  // bind the Vertex Array Object first, then bind and set vertex buffer(s), and then configure vertex attributes(s).
  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);
  
  // position attribute
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  // color attribute
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  // texture coord attribute
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  //glUseProgram(shader_program);

#pragma endregion

#pragma region load and create a texture
  unsigned int texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture); // all upcoming GL_TEXTURE_2D operations now have effect on this texture object
  // set the texture wrapping parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	// set texture wrapping to GL_REPEAT (default wrapping method)
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  // set texture filtering parameters
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  // load image, create texture and generate mipmaps
  int width, height, nrChannels;
  // The FileSystem::getPath(...) is part of the GitHub repository so we can find files on any IDE/platform; replace it with your own image path.
  unsigned char *data = stbi_load(".\\resources\\x.jpg", &width, &height, &nrChannels, 0);
  if (data)
  {
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
  }
  else
  {
    std::cout << "Failed to load texture" << std::endl;
  }
  stbi_image_free(data);
  

#pragma endregion

#pragma region draw in wireframe polygons
  // uncomment this call to draw in wireframe polygons.
  // glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
  glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
#pragma endregion

#pragma region imgui

  IMGUI_CHECKVERSION();
  ImGui::CreateContext(nullptr);
  ImGuiIO &io = ImGui::GetIO();
  (void) io;
 // io.MouseDrawCursor = true;
  io.Fonts->AddFontFromFileTTF("simkai.ttf", 60, nullptr, io.Fonts->GetGlyphRangesChineseFull());
  io.ConfigFlags |= ImGuiConfigFlags_NoMouseCursorChange;

  ImGui::StyleColorsLight();
  ImGuiStyle &style = ImGui::GetStyle();

  ImVec4 *colors = style.Colors;
  colors[ImGuiCol_WindowBg] = ImVec4(0.94f, 0.94f, 0.94f, 0.00f);
  colors[ImGuiCol_Border] = ImVec4(0.00f, 0.00f, 0.00f, 0.00f);
  colors[ImGuiCol_TitleBg] = ImVec4(0.96f, 0.96f, 0.96f, 0.00f);
  colors[ImGuiCol_TitleBgActive] = ImVec4(0.82f, 0.82f, 0.82f, 0.00f);



  ImGui_ImplGlfw_InitForOpenGL(window, true);
  ImGui_ImplOpenGL3_Init("#version 330");
#pragma endregion

  double startTime = glfwGetTime();
  auto start_time = GetTime();

#pragma region render loop
  while (!glfwWindowShouldClose(window)) {
     double currentTime = glfwGetTime();

    // Check if 30 seconds have passed
    if (currentTime - startTime > 35.0) {
      glfwSetWindowShouldClose(window, true);
    }


    // input
    // -----
    processInput(window);

    // render
    // ------
    glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    // bind Texture
    glBindTexture(GL_TEXTURE_2D, texture);
    // draw our first triangle
    //glUseProgram(shader_program);

    our_shader.use();
    glBindVertexArray(VAO);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);

    //io.MouseDrawCursor = true;

    ImGui_ImplOpenGL3_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    ImGui::SetNextWindowPos(ImVec2(635, 600), ImGuiCond_Always);
    // 设定窗口大小
    ImGui::SetNextWindowSize(ImVec2(650, 300), ImGuiCond_Always);
    ImGui::Begin("", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoInputs);

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.211f, 0.18f, 0.18f, 0.5f));
    ImGui::Text("系统初始化中,请勿操作");

    ImGui::End();

    // 设定窗口位置
    ImGui::SetNextWindowPos(ImVec2(900, 700), ImGuiCond_Always);
    // 设定窗口大小
    ImGui::SetNextWindowSize(ImVec2(50, 300), ImGuiCond_Always);
    ImGui::Begin("", NULL, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoInputs);

    float loading_percentage = CalculateLoadingPercentage(start_time, 35);// 30秒钟

    ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.211f, 0.18f, 0.18f, 0.5f));
    ImGui::Text("%.0f%%", loading_percentage);
    
    ImGui::End();



   // ImGui::ShowDemoWindow();

    ImGui::Render();
    ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    

    glfwSwapBuffers(window);
    glfwPollEvents();
  }
#pragma endregion

#pragma region optional: de-allocate all resources once they have outlived their purpose

  glDeleteVertexArrays(1, &VAO);
  glDeleteBuffers(1, &VBO);
  glDeleteBuffers(1, &EBO);
  //glDeleteProgram(shader_program);
#pragma endregion

#pragma region glfw: terminate, clearing all previously allocated GLFW resources
  ImGui_ImplOpenGL3_Shutdown();
  ImGui_ImplGlfw_Shutdown();
  ImGui::DestroyContext();

  glfwTerminate();
  return 0;
#pragma endregion
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window) {
  //if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) glfwSetWindowShouldClose(window, true);
}
// glfw: whenever the window size changed (by OS or user resize) this callback function executes
void framebuffer_size_callback(GLFWwindow *window, int width, int height) {
  // make sure the viewport matches the new window dimensions; note that width and
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}

void check_success(const unsigned int shader) {
  int success;
  glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetShaderInfoLog(shader, 512, nullptr, info_log);
    std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n"
              << info_log << std::endl;
  }
}

void check_success_program(const unsigned int program) {
  int success;
  glGetProgramiv(program, GL_LINK_STATUS, &success);
  if (!success) {
    char info_log[512];
    glGetProgramInfoLog(program, 512, nullptr, info_log);
    std::cout << "ERROR::SHADER::PROGRAM::LINKING_FAILED\n"
              << info_log << std::endl;
  }
}
std::chrono::time_point<std::chrono::steady_clock> GetTime() {
  return std::chrono::steady_clock::now();
}

// 根据开始时间和当前时间计算加载进度的函数
float CalculateLoadingPercentage(const std::chrono::time_point<std::chrono::steady_clock> &start_time, int duration_seconds) {
  auto now = GetTime();
  auto elapsed = std::chrono::duration_cast<std::chrono::seconds>(now - start_time).count();
  float progress = (elapsed / static_cast<float>(duration_seconds)) * 100.0f;
  progress = (progress > 100.0f) ? 100.0f : progress;// Clamp to 100%
  return progress;
}