/*
    g++ sphereDemo.cpp Sphere.cpp glad.c -lglfw3 -ldl -lm -lGL -lGLU -lX11 -pthread -o sphereDemo

    This is a spinning sphere without a texture. It's just a solid orange color.
*/

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

float cameraDistance = 4.0f;
unsigned int shaderProgram;
int screenWidth, screenHeight;
// unsigned int uniformMatrixModelView, uniformMatrixModelViewProjection;

// This is required in only *one* file, in this case, the one we're using it.
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include "Sphere.h"

void toPerspective();
void initShader();
void initVBO();
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow *window);

unsigned int attribVertexPosition;
glm::mat4 matrixModelView;
glm::mat4 matrixProjection;

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;

unsigned int VAO, vboId1, iboId1;
Sphere sphere1(1.0f, 8, 8, false);

float cameraAngleX;
float cameraAngleY;

// Vertex shader
/*
    We aren't utilizing textures, and are just going to draw an orange sphere.
    So, we only need access to the model, view, and projection matrices.
*/
const char *vertexShaderSource = "#version 330 core\n"
    "layout (location = 0) in vec3 aPos;\n"
    "uniform mat4 model;\n"
    "uniform mat4 view;\n"
    "uniform mat4 projection;\n\n"
    "void main()\n"
    "{\n"
    " gl_Position = projection * view * model * vec4(aPos, 1.0);\n"
    "}\0";

// Fragment shader
const char *fragmentShaderSource = "#version 330 core\n"
    "out vec4 FragColor;\n"
    "void main()\n"
    "{\n"
    "FragColor = vec4(1.0f, 0.5f, 0.2f, 1.0f);\n"
    "}\0";

int main()
{
	printf("Start\n");
    // glfw: initialize and configure
    // ------------------------------
    glfwInit();
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
    glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

    // glfw window creation
    // --------------------
    GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LearnOpenGL", NULL, NULL);
    if (window == NULL)
    {
        printf("Failed to create GLFW window");
        glfwTerminate();
        return -1;
    }
    glfwMakeContextCurrent(window);
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

    // glad: load all OpenGL function pointers
    // ---------------------------------------
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        printf("Failed to initialize GLAD");
        return -1;
    }   

    // glShadeModel(GL_SMOOTH);                    // shading mathod: GL_SMOOTH or GL_FLAT
    //glPixelStorei(GL_UNPACK_ALIGNMENT, 4);      // 4-byte pixel alignment

    // enable /disable features
    // glHint(GL_PERSPECTIVE_CORRECTION_HINT, GL_NICEST);
    //glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
    //glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);
    glEnable(GL_DEPTH_TEST);
    // glEnable(GL_LIGHTING);
    //glEnable(GL_TEXTURE_2D);
    //glEnable(GL_CULL_FACE);

    // track material ambient and diffuse from surface color, call it before glEnable(GL_COLOR_MATERIAL)
    //glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
    //glEnable(GL_COLOR_MATERIAL);

    glClearColor(0.2, 0.3, 0.4, 0);             // background color
    //glClearStencil(0);                          // clear stencil buffer
    //glClearDepth(1.0f);                         // 0 is near, 1 is far
    //glDepthFunc(GL_LEQUAL);

    initShader();
    initVBO();
    //sphere1.reverseNormals();
        
    // set attrib arrays using glVertexAttribPointer()
    

    //attribVertexPosition = glGetAttribLocation(shaderProgram, "aPos");

    printf("Finished with initial setup. \n"); 
    sphere1.printSelf();
    // render loop
    while (!glfwWindowShouldClose(window))
    {
        processInput(window);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);
        toPerspective();
        glUseProgram(shaderProgram);
        glEnableVertexAttribArray(0);

        glBindVertexArray(VAO);
        // Bind these buffers so we use the data in them.
        glBindBuffer(GL_ARRAY_BUFFER, vboId1);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId1);

        // For the vertex attribute, we need to set up how to read the data.
        // The sphere class provides this function to get the stride.
        int stride = sphere1.getInterleavedStride();
        glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);

        // transform camera (view)
        glm::mat4 view = glm::mat4(1.0f);
        view = glm::translate(view, glm::vec3(0, 0, -cameraDistance));

        glm::mat4 model = glm::mat4(1.0f);
        model = glm::rotate(model, (float)glfwGetTime() * glm::radians(50.0f), glm::vec3(0.5f, 1.0f, 0.0f));
        /*model = glm::rotate(model, glm::radians(cameraAngleX), glm::vec3(1.0f, 0.0f, 0.0f));
        model = glm::rotate(model, glm::radians(cameraAngleY), glm::vec3(0.0f, 1.0f, 0.0f));
        model = glm::rotate(model, glm::radians(-90.0f), glm::vec3(1.0f, 0.0f, 0.0f));*/

        /*glm::mat4 projection = glm::mat4(1.0f); // Identity matrix.
        projection = glm::perspective(glm::radians(45.0f), 800.0f / 600.0f, 0.1f, 100.0f);*/

        // projection = projection * view;
        // glm::mat4 matrixNormal = matrixModelView;

        int modelLoc = glGetUniformLocation(shaderProgram, "model");
        glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));

        int viewLoc = glGetUniformLocation(shaderProgram, "view");
        glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));

        toPerspective();

        //glBindVertexArray(VAO);
        //int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
        //glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));

        // draw left sphere
        glDrawElements(GL_TRIANGLES,            // primitive type
                    sphere1.getIndexCount(), // # of indices
                    GL_UNSIGNED_INT,         // data type
                    (void*)0);               // ptr to indices

        glBindBuffer(GL_ARRAY_BUFFER, 0);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
        glUseProgram(0);
        glfwSwapBuffers(window);
        glfwPollEvents();
    }
        
    // clean up and exit.
    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &vboId1);
    glDeleteBuffers(1, &iboId1);
    glDeleteProgram(shaderProgram);
    glfwTerminate();
    printf("Goodbye\n");
    return 0;
}

// to use orthogonal view, call this function.
// this was copied from the main.cpp of the sphereShader example, and converted
// to utilize GLM instead of custom matrix classes. Not sure why things are hard coded
// or how to not make them hard coded.
void toOrtho()
{
    const float N = -1.0f;
    const float F = 1.0f;
    // set viewport to be the entire window
    glViewport(0, 0, (GLsizei)SCR_WIDTH, (GLsizei)SCR_HEIGHT);
    /*
    The first two parameters specify the left and right coordinate of the frustum and the third and
fourth parameter specify the bottom and top part of the frustum. With those 4 points we’ve defined
the size of the near and far planes and the 5th and 6th parameter then define the distances between
the near and far plane. This specific projection matrix transforms all coordinates between these x, y
and z range values to normalized device coordinates.
    */
    glm::mat4 projection = glm::ortho(0.0f, (float)screenWidth, 0.0f, (float)screenHeight, N, F);
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

///////////////////////////////////////////////////////////////////////////////
// set the projection matrix as perspective
///////////////////////////////////////////////////////////////////////////////
void toPerspective()
{
    const float N = 0.1f;
    const float F = 100.0f;
    const float DEG2RAD = 3.141592f / 180;
    const float FOV_Y = 40.0f * DEG2RAD;
    glViewport(0, 0, (GLsizei)SCR_WIDTH, (GLsizei)SCR_HEIGHT);
    float aspectRatio = (float)SCR_WIDTH / (float)SCR_HEIGHT;
    // perspective takes in (fovy, aspect, near, far)
    glm::mat4 projection = glm::perspective(FOV_Y, aspectRatio, N, F);
    int projectionLoc = glGetUniformLocation(shaderProgram, "projection");
    glUniformMatrix4fv(projectionLoc, 1, GL_FALSE, glm::value_ptr(projection));
}

/*
    Reads the sphere data into a VBO and EBO
*/
void initVBO() 
{
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &vboId1);
    glGenBuffers(1, &iboId1);
    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, vboId1);
    glBufferData(GL_ARRAY_BUFFER, sphere1.getInterleavedVertexSize(), sphere1.getInterleavedVertices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

   
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, iboId1);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sphere1.getIndexSize(), sphere1.getIndices(), GL_STATIC_DRAW);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);

    int stride = sphere1.getInterleavedStride();
    glVertexAttribPointer(0, 3, GL_FLOAT, false, stride, 0);
    glEnableVertexAttribArray(0);
    glBindVertexArray(0);
}

/*
    Reads the shader source code and compiles it into a shader program.
*/
void initShader() 
{
    unsigned int vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    // check for shader compile errors
    int success;
    char infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::VERTEX::COMPILATION_FAILED\n");
        printf("Check the vertex fragment GLSL code.\n\nGoodbye!\n\n");
    }
    // fragment shader
    unsigned int fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    // check for shader compile errors
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success)
    {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n");
        printf("Check the fragment shader GLSL code.\n\nGoodbye!\n\n");
    }
    // link shaders
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    // check for linking errors
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        printf("ERROR::SHADER::PROGRAM::LINKING_FAILED\n");
        printf("The fragment and vertex shaders individually compiled, but the shader program did not compile.\n\nGoodbye!\n\n");
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
        glfwSetWindowShouldClose(window, 1);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
    // make sure the viewport matches the new window dimensions; note that width and 
    // height will be significantly larger than specified on retina displays.
    glViewport(0, 0, width, height);
}
