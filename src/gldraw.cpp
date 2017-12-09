#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <camera.h>
#include <model.h>
#include <iostream>

// GPH stuff
#include "common.h"
#include "playfield.h"
#include "camera.h"
#include "sphere.h"

// prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;
float g_xpos = 30.0, g_ypos = 30.0, g_zpos = 0.0;
float g_xyDir;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// playfield
Playfield *g_pPlayfield;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

unsigned int loadTexture( const char *path)
{
  unsigned int textureID;
  glGenTextures(1, &textureID);

  int width, height, nrComponents;
  unsigned char *data = stbi_load(path, &width, &height, &nrComponents, 0);
  if (data)
  {
    GLenum format = GL_RGBA;
    if (nrComponents == 1)
      format = GL_RED;
    else if (nrComponents == 3)
      format = GL_RGB;
    else if (nrComponents == 4)
      format = GL_RGBA;

    glBindTexture(GL_TEXTURE_2D, textureID);
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    stbi_image_free(data);
  }
  else
  {
    std::cout << "Texture failed to load at path: " << path << std::endl;
    stbi_image_free(data);
  }

  return textureID;
}



int main()
{
  // glfw: initialize and configure
  // ------------------------------
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  // --------------------
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Roller Ball Concept", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return -1;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

  // glad: load all OpenGL function pointers
  // ---------------------------------------
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global opengl state
  // -----------------------------
  glEnable(GL_DEPTH_TEST);

  // build and compile our shader zprogram
  // ------------------------------------
  Shader lightingShader("multilight.vs", "multilight.fs");
  Shader lampShader("lamp.vs", "lamp.fs");
  Shader robotShader("model_loading.vs", "model_loading.fs");

  // GPH Stuff
  g_pPlayfield = new Playfield();
  // End GPH

  // positions of the point lights
  glm::vec3 pointLightPositions[] = {
    glm::vec3( 30.7f,  20.2f,  89.0f),
    glm::vec3( 290.3f, 18.3f, 89.0f),
    glm::vec3( 290.0f, 290.0f, 12.0f),
    glm::vec3( 290.0f,  35.0f, -3.0f)
  };



#if 1
  // first, configure the cube's VAO (and VBO)
  unsigned int VBO, VAO;
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  // GPH Stuff
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, g_pPlayfield->getVerticeBufSize(), g_pPlayfield->getVertices(), GL_DYNAMIC_DRAW);

  glBindVertexArray(VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // second, configure the light's VAO (VBO stays the same; the vertices are the same for the light object which is also a 3D cube)
  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // note that we update the lamp's position attribute's stride to reflect the updated buffer data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // load textures (we now use a utility function to keep the code more organized)
  // -----------------------------------------------------------------------------
  unsigned int diffuseMap, specularMap;
  diffuseMap = loadTexture( "tileset.png");
  specularMap = loadTexture( "pattern2.png");
#endif


  // shader configuration
  // --------------------
  lightingShader.use();
  lightingShader.setInt("material.diffuse", 0);
  lightingShader.setInt("material.specular", 1);


  // Set start position
  camera.setPosition( { 40.0, 0, 30.0 } );

  //Model ourModel("nanosuit.obj");
  Model ourModel("mirrorsphere.obj");
  // render loop
  // -----------
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    // TEST CODE; REMOVE
    g_pPlayfield->test();
    // END TEST

    // input
    processInput(window);

    // render
    // ------
    glClearColor(0.4f, 0.55f, 0.67f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // be sure to activate shader when setting uniforms/drawing objects
    lightingShader.use();
    lightingShader.setVec3("viewPos", camera.Position);
    lightingShader.setFloat("material.shininess", 32.0f);

    // directional light
    lightingShader.setVec3("dirLight.direction", -0.1f, -0.8f, -0.1f);
    lightingShader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
    lightingShader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
    lightingShader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);
    // point light 1
    lightingShader.setVec3("pointLights[0].position", pointLightPositions[0]);
    lightingShader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[0].diffuse", 0.2f, 0.2f, 0.8f);
    lightingShader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[0].constant", 1.0f);
    lightingShader.setFloat("pointLights[0].linear", 0.09);
    lightingShader.setFloat("pointLights[0].quadratic", 0.032);
    // point light 2
    lightingShader.setVec3("pointLights[1].position", pointLightPositions[1]);
    lightingShader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
    lightingShader.setVec3("pointLights[1].diffuse", 0.8f, 0.2f, 0.82);
    lightingShader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
    lightingShader.setFloat("pointLights[1].constant", 1.0f);
    lightingShader.setFloat("pointLights[1].linear", 0.09);
    lightingShader.setFloat("pointLights[1].quadratic", 0.032);

    // view/projection transformations
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 500.0f);
    glm::mat4 view = camera.GetViewMatrix();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);

    // world transformation
    glm::mat4 model;
    lightingShader.setMat4("model", model);
#if 1
    // bind diffuse map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, diffuseMap);
    // bind specular map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, specularMap);

    glBindVertexArray(VAO);
#endif
    // TEST CODE; REMOVE
#if 1
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    for( int y = 40; y < 60; y++ ) {
      glBufferSubData( GL_ARRAY_BUFFER, g_pPlayfield->testOffset( y ) * sizeof(float), 20 * 6 * 8 * sizeof(float), g_pPlayfield->testGetRowPtr( y ) );
    }
    //glBufferData(GL_ARRAY_BUFFER, g_pPlayfield->getVerticeBufSize(), g_pPlayfield->getVertices(), GL_DYNAMIC_DRAW);
#endif
    // END TEST

    glDrawArrays(GL_TRIANGLES, 0, g_pPlayfield->getVerticeTot());


    // TEST MESH
#if 0
    robotShader.use();
    robotShader.setMat4("projection", projection);
    robotShader.setMat4("view", view);
#endif
    // render the loaded model
    //lightingShader.setInt("material.diffuse", 3);
static float mtheta = 0.0;
    mtheta += 0.01;
    model = glm::translate(model, glm::vec3(43.0f, g_pPlayfield->getHeightAt(43.0, 43.0), 43.0)); // translate it down so it's at the center of the scene
    model = glm::rotate(model, mtheta, glm::vec3(1.0f, 0.0f, 0.0f));
    model = glm::scale(model, glm::vec3(1.4f, 1.4f, 1.4f));	// it's a bit too big for our scene, so scale it down
    lightingShader.setMat4("model", model);
    ourModel.Draw(lightingShader);

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    // -------------------------------------------------------------------------------
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  // ------------------------------------------------------------------------
  ////glDeleteVertexArrays(1, &VAO);
  ////glDeleteVertexArrays(1, &lightVAO);
  ////glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  // ------------------------------------------------------------------
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow *window)
{
  camera.saveOldPosition();
  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.ProcessKeyboard(LEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.ProcessKeyboard(RIGHT, deltaTime);
  }
  glm::vec3 pos = camera.getPosition();
  pos.y = g_pPlayfield->getHeightAt( pos.x, pos.z ) + 0.5;
  //#define DEBUG_POSITION
#ifdef DEBUG_POSITION
  printf( "%2.2f %2.2f %2.2f \n", pos.x, pos.y, pos.z );
#endif
  camera.setPosition( pos );
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
  if (firstMouse)
  {
    lastX = xpos;
    lastY = ypos;
    firstMouse = false;
  }

  float xoffset = xpos - lastX;
  float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

  lastX = xpos;
  lastY = ypos;

  camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
  // make sure the viewport matches the new window dimensions; note that width and 
  // height will be significantly larger than specified on retina displays.
  glViewport(0, 0, width, height);
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
}

