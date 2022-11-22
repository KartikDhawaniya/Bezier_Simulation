
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <fstream>
#include <vector>
#include <cmath>
#include <algorithm>
#include "glm/vec3.hpp"
#include "glm/vec4.hpp"
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "glm/gtx/string_cast.hpp"

using namespace std;

void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void cursor_position_callback(GLFWwindow *window, double xpos, double ypos);
void mouse_button_callback(GLFWwindow *window, int button, int action, int mods);
string readShaderSource(fstream &shaderFile);
void shader_error(GLuint shader, GLuint eShaderType);
GLuint sdrtopm(const char *vs, const char *fs);
glm::vec3 bezeir(vector<glm::vec4> c_pts, GLuint n, GLfloat i);
long long nCr(int n, int r);

vector<glm::vec4> control_pts;
GLuint degree = 2;
GLfloat x_cursor = 0.0f, y_cursor = 0.0f;
int screen_width = 1024, screen_height = 768;
int selected_point = -1;
bool is_dragging = false;
bool show_control_polygon = true;

int main()
{
    GLFWwindow *window;

    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW");
        return -1;
    }
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    // This is for MacOSX - can be omitted otherwise
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
    // We don't want the old OpenGL
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    window = glfwCreateWindow(1024, 768, "Assignment 2", NULL, NULL);
    if (window == NULL)
    {
        fprintf(stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.");
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glfwMakeContextCurrent(window);

    glewExperimental = true; // Needed for core profile
    GLenum err = glewInit();
    if (GLEW_OK != err)
    {
        // Problem: glewInit failed, something is seriously wrong.
        std::cerr << "GLEW Init Failed : %s" << std::endl;
    }

    glfwSetKeyCallback(window, key_callback);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetCursorPosCallback(window, cursor_position_callback);
    glfwSetMouseButtonCallback(window, mouse_button_callback);

    glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
    // Set depth buffer furthest depth
    glClearDepth(1.0);
    // Set depth test to less-than
    glDepthFunc(GL_LESS);
    // Enable depth testing
    glEnable(GL_DEPTH_TEST);

    GLuint tesel = 100;

    control_pts.push_back(glm::vec4(-0.5f, 0.0f, 0.0f, 1.0f));
    control_pts.push_back(glm::vec4(0.5f, 0.0f, 0.0f, 1.0f));
    control_pts.push_back(glm::vec4(0.0f, 0.5f, 0.0f, 1.0f));

    vector<glm::vec4> colors;
    vector<glm::vec4> vertices;

    fstream vertex_stream, fragment_stream;
    vertex_stream.open("vertex.shader", ios::in);
    fragment_stream.open("fragment.shader", ios::in);

    string vertex_shader = readShaderSource(vertex_stream);
    string fragment_shader = readShaderSource(fragment_stream);

    GLuint program = sdrtopm(vertex_shader.c_str(), fragment_shader.c_str());

    GLuint uMVP = glGetUniformLocation(program, "uMVPMatrix");

    while (!glfwWindowShouldClose(window))
    {

        vertices.clear();
        colors.clear();
        for (int i = 0; i <= tesel; i++)
        {
            GLfloat t = i * 1.0f / tesel;
            vertices.push_back(glm::vec4(bezeir(control_pts, degree, t), 1.0f));
            // cout << glm::to_string(bezeir(control_pts, degree, t)) << endl;
        }

        for (int i = 0; i < degree + 1; i++)
        {
            vertices.push_back(control_pts[i] + glm::vec4(0.0, 0.01, 0.0, 0.0));
            vertices.push_back(control_pts[i] + glm::vec4(0.005 * sqrt(3), -0.005, 0.0, 0.0));
            vertices.push_back(control_pts[i] + glm::vec4(-0.005 * sqrt(3), -0.005, 0.0, 0.0));
            vertices.push_back(control_pts[i] + glm::vec4(0.0, 0.01, 0.0, 0.0));
        }

        for (int i = 0; i < degree + 1; i++)
        {
            vertices.push_back(control_pts[i]);
        }

        colors = vector<glm::vec4>(tesel + 1 + 4 * (degree + 1), glm::vec4(1.0f, 0.0f, 0.0f, 1.0f));

        for (int i = 0; i < degree + 1; i++)
        {
            colors.push_back(glm::vec4(0.5f, 0.5f, 0.5f, 1.0f));
        }

        GLuint vao, vbo;

        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);

        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4) + colors.size() * sizeof(glm::vec4), NULL, GL_STATIC_DRAW);
        glBufferSubData(GL_ARRAY_BUFFER, 0, vertices.size() * sizeof(glm::vec4), &vertices[0]);
        glBufferSubData(GL_ARRAY_BUFFER, vertices.size() * sizeof(glm::vec4), colors.size() * sizeof(glm::vec4), &colors[0]);
        GLuint vPosition = glGetAttribLocation(program, "vPosition");
        glEnableVertexAttribArray(vPosition);
        glVertexAttribPointer(vPosition, 4, GL_FLOAT, GL_FALSE, 0, NULL);

        GLuint vColor = glGetAttribLocation(program, "vColor");
        glEnableVertexAttribArray(vColor);
        glVertexAttribPointer(vColor, 4, GL_FLOAT, GL_FALSE, 0, (GLvoid *)(vertices.size() * sizeof(glm::vec4)));

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glUseProgram(program);

        glm::mat4 MVP = glm::mat4(1.0f);

        glUniformMatrix4fv(uMVP, 1, GL_FALSE, glm::value_ptr(MVP));

        glBindVertexArray(vao);
        glDrawArrays(GL_LINE_STRIP, 0, tesel + 1);
        for (int i = 0; i <= degree; i++)
            glDrawArrays(GL_LINE_STRIP, tesel + 1 + 4 * i, 4);

        if(show_control_polygon)
            glDrawArrays(GL_LINE_STRIP, tesel + 1 + 4 * (degree + 1), degree + 1);

        glfwSwapBuffers(window);
        glfwPollEvents();
    }
}

void framebuffer_size_callback(GLFWwindow *window, int width, int height)
{
    screen_height = height;
    screen_width = width;
    glViewport(0, 0, width, height);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods)
{
    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        glfwSetWindowShouldClose(window, GL_TRUE);
    else if (key == GLFW_KEY_S && action == GLFW_PRESS)
        show_control_polygon = !show_control_polygon;
    else if (key == GLFW_KEY_D && action == GLFW_PRESS)
    {
        selected_point = -1;
        GLfloat dist = 3.0f;
        for (int i = 0; i < degree + 1; i++)
        {
            GLfloat len = glm::length(glm::vec2(control_pts[i]) - glm::vec2(x_cursor, y_cursor));
            if (len < dist)
            {
                dist = len;
                selected_point = i;
            }
        }

        if(dist > 0.05 || degree < 3)
            selected_point = -1;

        if(selected_point != -1)
        {
            // delete control_pts[selected_point];
            control_pts.erase(control_pts.begin() + selected_point);
            degree--;
            selected_point = -1;
        }
    }
    
}

string readShaderSource(fstream &shaderFile)
{
    string line;
    string source;
    while (getline(shaderFile, line))
        source += line + "\n";
    return source;
}

void shader_error(GLuint shader, GLuint eShaderType)
{
    GLint status;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &status);
    if (status == GL_FALSE)
    {
        GLint infoLogLength;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLogLength);

        GLchar *strInfoLog = new GLchar[infoLogLength + 1];
        glGetShaderInfoLog(shader, infoLogLength, NULL, strInfoLog);

        const char *strShaderType = NULL;
        switch (eShaderType)
        {
        case GL_VERTEX_SHADER:
            strShaderType = "vertex";
            break;
        case GL_GEOMETRY_SHADER:
            strShaderType = "geometry";
            break;
        case GL_FRAGMENT_SHADER:
            strShaderType = "fragment";
            break;
        }

        std::cerr << "Compile failure in " << strShaderType << " shader:" << std::endl
                  << strInfoLog << std::endl;
        delete[] strInfoLog;
    }
    else
    {
        // std::cout << "Shader compiled successfully" << std::endl;
        return;
    }
}

GLuint sdrtopm(const char *vs, const char *fs)
{
    GLuint vshader, fshader, program;
    GLint vCompiled, fCompiled, linked;

    vshader = glCreateShader(GL_VERTEX_SHADER);
    fshader = glCreateShader(GL_FRAGMENT_SHADER);

    glShaderSource(vshader, 1, &vs, NULL);
    glShaderSource(fshader, 1, &fs, NULL);

    glCompileShader(vshader);
    shader_error(vshader, GL_VERTEX_SHADER);

    glCompileShader(fshader);
    shader_error(fshader, GL_FRAGMENT_SHADER);

    program = glCreateProgram();
    glAttachShader(program, vshader);
    glAttachShader(program, fshader);

    glLinkProgram(program);
    glGetProgramiv(program, GL_LINK_STATUS, &linked);
    if (linked != 1)
    {
        cout << "Program not linked." << endl;
        exit(1);
    }

    return program;
}

glm::vec3 bezeir(vector<glm::vec4> c_pts, GLuint n, GLfloat i)
{
    glm::vec3 final(0.0, 0.0, 0.0);

    for (int j = 0; j <= n; j++)
    {
        final = final + glm::vec3(c_pts[j]) * (GLfloat)nCr(n, j) * (GLfloat)pow(i, j) * (GLfloat)pow(1 - i, n - j);
    }

    return final;
}

long long nCr(int n, int r)
{

    // p holds the value of n*(n-1)*(n-2)...,
    // k holds the value of r*(r-1)...
    long long p = 1, k = 1;

    // C(n, r) == C(n, n-r),
    // choosing the smaller value
    if (n - r < r)
        r = n - r;

    if (r != 0)
    {
        while (r)
        {
            p *= n;
            k *= r;

            // gcd of p, k
            long long m = __gcd(p, k);

            // dividing by gcd, to simplify
            // product division by their gcd
            // saves from the overflow
            p /= m;
            k /= m;

            n--;
            r--;
        }

        // k should be simplified to 1
        // as C(n, r) is a natural number
        // (denominator should be 1 ) .
    }

    else
        p = 1;

    // if our approach is correct p = ans and k =1
    return p;
}

void cursor_position_callback(GLFWwindow *window, double xpos, double ypos)
{
    xpos = 2 * xpos / screen_width;
    ypos = 2 * (1 - ypos / screen_height);
    x_cursor = xpos - 1;
    y_cursor = ypos - 1;

    // cout << x_cursor << " " << y_cursor << endl;

    // if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_PRESS)
    // {
    //     GLfloat dist = 2;
    //     is_dragging = true;
    //     for(int i = 0; i<= degree; i++)
    //     {
    //         GLfloat len = glm::length(glm::vec2(x_cursor, y_cursor) - glm::vec2(control_pts[i].x, control_pts[i].y));
    //         if(len < dist)
    //         {
    //             dist = len;
    //             selected_point = i;
    //         }
    //     }
    //     if(dist>0.01)
    //     {
    //         selected_point = -1;
    //     }

    //     cout << "Selected point: " << selected_point << endl;

    // }

    // if(glfwGetMouseButton(window, GLFW_MOUSE_BUTTON_LEFT) == GLFW_RELEASE)
    // {
    //     is_dragging = false;
    //     selected_point = -1;
    // }

    if (is_dragging && selected_point != -1)
    {
        control_pts[selected_point].x = x_cursor;
        control_pts[selected_point].y = y_cursor;
    }
}

void mouse_button_callback(GLFWwindow *window, int button, int action, int mods)
{
    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_PRESS)
    {
        GLfloat dist = 2;
        is_dragging = true;
        for (int i = 0; i <= degree; i++)
        {
            GLfloat len = glm::length(glm::vec2(x_cursor, y_cursor) - glm::vec2(control_pts[i].x, control_pts[i].y));
            if (len < dist)
            {
                dist = len;
                selected_point = i;
            }
        }
        if (dist > 0.05)
        {
            selected_point = -1;
        }

        // cout << "Selected point " << selected_point << endl;
    }

    if (button == GLFW_MOUSE_BUTTON_LEFT && action == GLFW_RELEASE)
    {
        is_dragging = false;
        selected_point = -1;
    }

    if (button == GLFW_MOUSE_BUTTON_RIGHT && action == GLFW_PRESS)
    {
        if (degree < 20)
        {
            degree++;
            control_pts.push_back(glm::vec4(x_cursor, y_cursor, 0.0, 1.0));
        }
    }
}