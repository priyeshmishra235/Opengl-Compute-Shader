#include "glad/glad.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#define M_PI 3.14159265358979323846f
const unsigned int WIDTH = 16384, HEIGHT = 1;

const char *fragShaderSrc = R"glsl(
#version 430 core
out vec4 FragColor;
uniform float a;
uniform float b;
uniform float width;

void main(){
    float x_norm = (gl_FragCoord.x + 0.5) / width;
    float x = a + (b - a) * x_norm;
    float fx = cos(x);
    FragColor = vec4(fx, 0.0, 0.0, 1.0);
}
)glsl";

const char *vertShaderSrc = R"glsl(
#version 330 core
const vec2 verts[3] = vec2[3](
    vec2(-1.0, -1.0),
    vec2( 3.0, -1.0),
    vec2(-1.0,  3.0)
);
void main() {
    gl_Position = vec4(verts[gl_VertexID], 0.0, 1.0);
}
)glsl";

GLuint compileShader(GLenum type, const char *src) {
  GLuint shader = glCreateShader(type);
  glShaderSource(shader, 1, &src, nullptr);
  glCompileShader(shader);
  return shader;
}

GLuint createProgram(const char *vsSrc, const char *fsSrc) {
  GLuint vs = compileShader(GL_VERTEX_SHADER, vsSrc);
  GLuint fs = compileShader(GL_FRAGMENT_SHADER, fsSrc);
  GLuint program = glCreateProgram();
  glAttachShader(program, vs);
  glAttachShader(program, fs);
  glLinkProgram(program);
  glDeleteShader(vs);
  glDeleteShader(fs);
  return program;
}

int main() {
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_VISIBLE, GLFW_FALSE);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

  GLFWwindow *window =
      glfwCreateWindow(WIDTH, HEIGHT, "GL GPU Integration", nullptr, nullptr);
  glfwMakeContextCurrent(window);
  gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);

  GLuint tex;
  glGenTextures(1, &tex);
  glBindTexture(GL_TEXTURE_2D, tex);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_R32F, WIDTH, HEIGHT, 0, GL_RED, GL_FLOAT,
               nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

  GLuint vao;
  glGenVertexArrays(1, &vao);
  glBindVertexArray(vao);

  GLuint fbo;
  glGenFramebuffers(1, &fbo);
  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D,
                         tex, 0);

  GLuint program = createProgram(vertShaderSrc, fragShaderSrc);
  glUseProgram(program);

  float a = 0.0f, b = (float)M_PI;

  GLint loc_a = glGetUniformLocation(program, "a");
  GLint loc_b = glGetUniformLocation(program, "b");
  GLint loc_width = glGetUniformLocation(program, "width");

  glUniform1f(loc_a, a);
  glUniform1f(loc_b, b);
  glUniform1f(loc_width, (float)WIDTH);

  glBindFramebuffer(GL_FRAMEBUFFER, fbo);
  glViewport(0, 0, WIDTH, HEIGHT);
  glClearColor(0.0, 0.0, 0.0, 1.0);
  glClear(GL_COLOR_BUFFER_BIT);
  glDrawArrays(GL_TRIANGLES, 0, 3);
  glFinish();

  std::vector<float> pixels(WIDTH * HEIGHT);
  glReadPixels(0, 0, WIDTH, HEIGHT, GL_RED, GL_FLOAT, pixels.data());

  float dx = (b - a) / WIDTH;
  double sum = 0.0;
  for (unsigned int i = 0; i < WIDTH; ++i)
    sum += static_cast<double>(pixels[i]) * dx;
  std::cout
      << "\n\n\n<-Integration Result for Even Function (Riemann Sum)->\n\n";
  std::cout << "f(x)=cos(x)\n" << std::endl;
  std::cout << "Calculated integral [0, pi] = " << sum << std::endl;
  std::cout << "\nExpected integral value is 0.0000" << "\n\n\n" << std::endl;
  glDeleteVertexArrays(1, &vao);
  glDeleteFramebuffers(1, &fbo);
  glDeleteTextures(1, &tex);
  glDeleteProgram(program);
  glfwTerminate();
  return 0;
}
