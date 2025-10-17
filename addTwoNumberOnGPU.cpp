#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>

int main() {
  if (!glfwInit())
    return -1;

  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);

  GLFWwindow *window = glfwCreateWindow(100, 100, "Addition", nullptr, nullptr);
  if (!window)
    return -1;
  glfwMakeContextCurrent(window);

  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    return -1;

  // Shader ---
  const char *computeShaderSource = R"(
    #version 430
    layout (local_size_x = 1) in;

    layout(std430, binding = 0) buffer InputA { float a; };
    layout(std430, binding = 1) buffer InputB { float b; };
    layout(std430, binding = 2) buffer Output { float result; };

    void main() {
        result = a + b;
    }
  )";

  GLuint shader = glCreateShader(GL_COMPUTE_SHADER);
  glShaderSource(shader, 1, &computeShaderSource, NULL);
  glCompileShader(shader);

  GLuint program = glCreateProgram();
  glAttachShader(program, shader);
  glLinkProgram(program);
  glDeleteShader(shader);

  float aValue = 55.0f;
  float bValue = 45.9f;
  float resultValue = 0.0f;

  GLuint ssbo[3];
  glGenBuffers(3, ssbo);

  // Input A
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[0]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &aValue,
               GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 0, ssbo[0]);

  // Input B
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[1]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), &bValue,
               GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 1, ssbo[1]);

  // Result
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
  glBufferData(GL_SHADER_STORAGE_BUFFER, sizeof(float), nullptr,
               GL_DYNAMIC_COPY);
  glBindBufferBase(GL_SHADER_STORAGE_BUFFER, 2, ssbo[2]);

  glUseProgram(program);
  glDispatchCompute(1, 1, 1);
  glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

  // Result
  glBindBuffer(GL_SHADER_STORAGE_BUFFER, ssbo[2]);
  float *ptr = (float *)glMapBufferRange(GL_SHADER_STORAGE_BUFFER, 0,
                                         sizeof(float), GL_MAP_READ_BIT);
  resultValue = *ptr;
  glUnmapBuffer(GL_SHADER_STORAGE_BUFFER);

  std::cout << "\n\n\n1st Number = " << aValue << "\n2nd Number = " << bValue
            << "\nAddition Result: " << resultValue << "\n\n\n"
            << std::endl;

  glDeleteBuffers(3, ssbo);
  glDeleteProgram(program);
  glfwDestroyWindow(window);
  glfwTerminate();

  return 0;
}
