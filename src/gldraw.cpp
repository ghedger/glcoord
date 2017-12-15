#include <boost/thread/thread.hpp>
#include <boost/thread/mutex.hpp>
#include <boost/bind.hpp>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <shader.h>
#include <camera.h>
#include <model.h>
#include <iostream>
#include <math.h>

// GPH stuff
#include "common.h"
#include "playfield.h"
#include "camera.h"
#include "objm.h"
#include "ballobj.h"
#include "gopmanager.h"


#define TEST_TEXTURE

// prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 800;
unsigned int SCR_HEIGHT = 600;
float g_xpos = 30.0, g_ypos = 30.0, g_zpos = 0.0;
float g_xyDir;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = SCR_WIDTH / 2.0f;
float lastY = SCR_HEIGHT / 2.0f;
bool firstMouse = true;

// playfield manager
Playfield *g_pPlayfield;

// object manager
ObjManager *g_pObjManager;

// graphics operation manager
GOPManager *g_pGOPManager;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

// lighting
glm::vec3 lightPos(1.2f, 1.0f, 2.0f);

// synchronization
boost::mutex gl_mutex;


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

// TODO: These should not be global.  This whole thing should
// eventually live in a class.
unsigned int VBO, VAO;
unsigned int diffuseMap, specularMap;
unsigned int testTexture;
unsigned int renderedTexture;
// positions of the point lights
glm::vec3 pointLightPositions[] = {
  glm::vec3( 30.7f,  20.2f,  89.0f),
  glm::vec3( 290.3f, 18.3f, 89.0f),
  glm::vec3( 290.0f, 290.0f, 12.0f),
  glm::vec3( 290.0f,  35.0f, -3.0f)
};

unsigned int FBO = 0;


void setMaterialsAndLighting( Shader shader )
{
  shader.setVec3("viewPos", camera.Position);
  shader.setFloat("material.shininess", 32.0f);

  // directional light
  shader.setVec3("dirLight.direction", -0.1f, -0.8f, -0.1f);
  shader.setVec3("dirLight.ambient", 0.3f, 0.3f, 0.3f);
  shader.setVec3("dirLight.diffuse", 0.5f, 0.5f, 0.5f);
  shader.setVec3("dirLight.specular", 0.7f, 0.7f, 0.7f);
  // point light 1
  shader.setVec3("pointLights[0].position", pointLightPositions[0]);
  shader.setVec3("pointLights[0].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[0].diffuse", 0.2f, 0.2f, 0.8f);
  shader.setVec3("pointLights[0].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[0].constant", 1.0f);
  shader.setFloat("pointLights[0].linear", 0.09);
  shader.setFloat("pointLights[0].quadratic", 0.032);
  // point light 2
  shader.setVec3("pointLights[1].position", pointLightPositions[1]);
  shader.setVec3("pointLights[1].ambient", 0.05f, 0.05f, 0.05f);
  shader.setVec3("pointLights[1].diffuse", 0.8f, 0.2f, 0.82);
  shader.setVec3("pointLights[1].specular", 1.0f, 1.0f, 1.0f);
  shader.setFloat("pointLights[1].constant", 1.0f);
  shader.setFloat("pointLights[1].linear", 0.09);
  shader.setFloat("pointLights[1].quadratic", 0.032);
}

Model * ourRobot = NULL;
void render( glm::mat4 projection, glm::mat4 view, Shader& shader, bool toBuffer )
{
  if( toBuffer )
  {
    boost::mutex::scoped_lock lock(gl_mutex);
    // Init FBO context
    if( !FBO ) {
      std::cout << "INITIALIZING FBO..." << std::endl;
      glGenFramebuffers( 1, &FBO );                     //Generate a framebuffer object(FBO)
      glBindFramebuffer( GL_FRAMEBUFFER, FBO );         // and bind it to the pipeline

      glGenTextures( 1, &renderedTexture );

      glBindTexture( GL_TEXTURE_2D, renderedTexture );

      unsigned width = SCR_WIDTH, height = SCR_HEIGHT;

      glTexImage2D( GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, 0 );

      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
      glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);

      // The depth buffer
      GLuint depthrenderbuffer;
      glGenRenderbuffers(1, &depthrenderbuffer);
      glBindRenderbuffer(GL_RENDERBUFFER, depthrenderbuffer);
      glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH_COMPONENT, width, height);
      glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_RENDERBUFFER, depthrenderbuffer);

      // Set "renderedTexture" as our colour attachement #0
      glFramebufferTexture(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, renderedTexture, 0);
      // (the below is optional)
      //glFramebufferTexture(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, texture_depth, 0);//optional

      // Set the list of draw buffers.
      GLenum DrawBuffers[1] = {GL_COLOR_ATTACHMENT0};
      glDrawBuffers(1, DrawBuffers); // "1" is the size of DrawBuffers

      // Always check that our framebuffer is ok
      if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cout << "A BAD THING HAPPENED, glError==" << glGetError() << std::endl;
        return;
      }
      //fbo.bind();
    }
    glBindFramebuffer(GL_FRAMEBUFFER, FBO);

  } else {
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    if(glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
      std::cout << "A BAD THING HAPPENED, glError==" << glGetError() << std::endl;
      return;
    }
  }

  // render
  glClearColor(0.4f, 0.55f, 0.67f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

  // shader setup
  shader.use();
  setMaterialsAndLighting(shader);

  // view/projection transformations
  shader.setMat4("projection", projection);
  shader.setMat4("view", view);
  glm::mat4 model(1.0f);
  shader.setMat4("model", model);

  // bind diffuse map
  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, diffuseMap);
  // bind specular map
  glActiveTexture(GL_TEXTURE1);
  glBindTexture(GL_TEXTURE_2D, specularMap);
  glBindVertexArray(VAO);
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  // Our test hack
#ifdef HP_TEST_HACK
  for( int y = 40; y < 60; y++ ) {
    glBufferSubData( GL_ARRAY_BUFFER, g_pPlayfield->testOffset( y ) * sizeof(float), 20 * 6 * 8 * sizeof(float), g_pPlayfield->testGetRowPtr( y ) );
  }
#endif

  //shader.use();
  glDrawArrays(GL_TRIANGLES, 0, g_pPlayfield->getVerticeTot());

  if( ourRobot ) {
    // TEST ROBOT
    shader.use();
    glm::mat4 model3(1.0f);
    //model2 = glm::scale(model2, glm::vec3(3.4f, 3.4f, 3.4f));
    //shader.setMat4("projection", projection);
    //shader.setMat4("view", view);
    // world transformation
    model3 = glm::translate(
        model3,
        glm::vec3(43.0f,
          g_pPlayfield->getHeightAt(43.0, 43.0) + 1.0,
          43.0)
        );
    shader.setMat4("model", model3);

    ourRobot->Draw(shader);
  }

  if( toBuffer ) {
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //fbo.unbind();
  }
}

GLFWwindow * initWindow()
{
  // glfw: initialize and configure
  glfwInit();
  glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
  glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
  glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
  glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

  // glfw window creation
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Roller Ball Concept", NULL, NULL);
  if (window == NULL)
  {
    std::cout << "Failed to create GLFW window" << std::endl;
    glfwTerminate();
    return NULL;
  }
  glfwMakeContextCurrent(window);
  glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
  // glfwSetCursorPosCallback(window, mouse_callback);
  glfwSetScrollCallback(window, scroll_callback);

  return window;
}


void initObj()
{
  g_pObjManager = new ObjManager();
  if( g_pObjManager ) {
    BallObj *pO = new BallObj();
    if( pO ) {
      pO->setPos( 55.0, 0.0, 108.0 );
      g_pObjManager->add( pO );
    }
  } else {
    // TODO: LOG ERROR
  }
}

void initGOP()
{
  g_pGOPManager = new GOPManager();
  if( !g_pGOPManager ) {
    // TODO: LOG ERROR
  }

}


static float ballX = 55.0;
static float ballZ = 108.0;
static float ballY = 0.0;
static float ballXVel = 0.0;
static float ballZVel = 0.0;
int main()
{
  //glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
  GLFWwindow* window = initWindow();

  // glad: load all OpenGL function pointers
  if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
  {
    std::cout << "Failed to initialize GLAD" << std::endl;
    return -1;
  }

  // configure global opengl state
  glEnable(GL_DEPTH_TEST);

  // build and compile our shader zprogram
  Shader lightingShader("multilight.vs", "multilight.fs");
  Shader lampShader("lamp.vs", "lamp.fs");
  Shader reflectShader("reflect.vs", "reflect.fs");
  //Shader reflectShader("multilight.vs", "multilight.fs");

  // GPH Stuff
  g_pPlayfield = new Playfield();
  initObj();
  initGOP();
  // End GPH

#if 1
  // first, configure vertice array and vertice buffer objects
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
  diffuseMap = loadTexture( "tileset.png");
  specularMap = loadTexture( "pattern2.png");
  testTexture = loadTexture( "test.png");
#endif

  // Set start position
  camera.setPosition( { 56.0, 0, 54.0 } );

  // Model ourModel("nanosuit.obj");
  Model ourModel("dome2.obj");
  ourRobot = new Model("nanosuit.obj");

  ballY = g_pPlayfield->getHeightAt( ballX, ballZ ) + 0.5;

  Camera cameraReflect(
      glm::vec3(ballX, ballY, ballZ),
      glm::normalize(glm::vec3(0.0f, 1.0f, 0.0))
      );

  //cameraReflect.setZoom(90.0);

  camera.setPitchAdjust( true );

  // Main game loop
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

    g_pObjManager->update();

    //
    // UPDATE GAME MOTION AND PHYSICS
    //

    // TEST CODE; REMOVE
    g_pPlayfield->test();
    // END TEST

    // input
    processInput(window);

#if 1
    // TEMP CODE: Ball Move
    {
#define MAX_VEL 0.01f
      glm::vec3 norm = g_pPlayfield->getNormalAt( ballX, ballZ );
      ballXVel += norm.x / 2000.0f;
      ballZVel += norm.y / 2000.0f;
      if( ballXVel > MAX_VEL )
        ballXVel = MAX_VEL;
      if( ballXVel < -MAX_VEL )
        ballXVel = -MAX_VEL;
      if( ballZVel > MAX_VEL )
        ballZVel = MAX_VEL;
      if( ballZVel < -MAX_VEL )
        ballZVel = -MAX_VEL;

      ballX += ballXVel;
      ballZ += ballZVel;
      ballY = g_pPlayfield->getHeightAt( ballX, ballZ ) + 0.5;
    }
#endif

    // Do after all the movement and collision detection but before render
    camera.UpdateVectors();

    //
    // RENDER
    //
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, MAX_VIEWING_DISTANCE);

    glm::mat4 view = camera.GetViewMatrix();

    // Render the view to the visible back buffer
    render( projection, view, lightingShader, false);

    // Render all the objects
    GOPItem *pI = g_pGOPManager->getCurrentItem();
    if( pI ) {
      // Set up our ball
      pI->m_pos = glm::vec3( ballX, ballY, ballZ );
      pI->m_eyePos = camera.getPosition();
      pI->m_view = view;
      pI->m_projection = projection;
      pI->m_opType = GOP_REFLECTANDFACE;
      pI->m_customTexture = renderedTexture;
      pI->m_pMeshModel = &ourModel;
      pI->m_pShader = &lightingShader;

      g_pGOPManager->push();
    }
    g_pGOPManager->update();

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  glDeleteVertexArrays(1, &VAO);
  glDeleteVertexArrays(1, &lightVAO);
  glDeleteBuffers(1, &VBO);

  // glfw: terminate, clearing all previously allocated GLFW resources.
  glfwTerminate();
  return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
void processInput(GLFWwindow *window)
{
  camera.saveOldPosition();

  bool ctrlPressed = glfwGetKey( window, GLFW_KEY_LEFT_CONTROL ) == GLFW_PRESS;
  ctrlPressed |= glfwGetKey( window, GLFW_KEY_RIGHT_CONTROL ) == GLFW_PRESS;

  if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
    glfwSetWindowShouldClose(window, true);

  if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? PITCH_UP : FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? PITCH_DOWN : BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? LEFT : TURNLEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? RIGHT : TURNRIGHT, deltaTime);
  }
  glm::vec3 pos = camera.getPosition();
  pos.y = g_pPlayfield->getHeightAt( pos.x, pos.z ) + EYE_HEIGHT;
#define DEBUG_POSITION
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
  boost::mutex::scoped_lock lock(gl_mutex);
  glDeleteBuffers( 1, &FBO );
  FBO = 0;
  glDeleteTextures( 1, &renderedTexture );

  glViewport(0, 0, width, height);
  SCR_WIDTH = width;
  SCR_HEIGHT = height;
}


// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
  camera.ProcessMouseScroll(yoffset);
}

