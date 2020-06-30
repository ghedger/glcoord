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
#include "robotobj.h"
#include "gopmanager.h"

#define TEST_TEXTURE

// prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void updateCamera(GLFWwindow *window);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 1280;
unsigned int SCR_HEIGHT = 1024;
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

// models
Model * robotModel = nullptr;
Model * ballModel = nullptr;

// synchronization
boost::mutex gl_mutex;


extern unsigned int loadTexture( const char *path );

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
  shader.setVec3("dirLight.direction", -0.5f, -0.4f, -0.1f);
  shader.setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
  shader.setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
  shader.setVec3("dirLight.specular", 0.9f, 0.9f, 0.9f);
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

// render
void render( glm::mat4 projection, glm::mat4 view, Shader& shader, bool toBuffer )
{
  if( toBuffer )
  {
    boost::mutex::scoped_lock lock(gl_mutex);
    // Init FBO context
    if( !FBO ) {
      std::cout << "INITIALIZING FBO..." << std::endl;
      glGenFramebuffers( 1, &FBO );                     // Generate a framebuffer object (FBO)
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

  glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
  glClearColor(0.4f, 0.55f, 0.67f, 1.0f);

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

  // Our test hack: section of playfield will act as a platform, moving up and down
// #define HP_TEST_HACK
#ifdef HP_TEST_HACK
  for( int y = 40; y < 60; y++ ) {
    glBufferSubData( GL_ARRAY_BUFFER, g_pPlayfield->testOffset( y ) * sizeof(float), 20 * 6 * 8 * sizeof(float), g_pPlayfield->testGetRowPtr( y ) );
  }
#endif

  //shader.use();
  glDrawArrays(GL_TRIANGLES, 0, g_pPlayfield->getVerticeTot());

  // draw objects
  g_pObjManager->drawAll();

  if( toBuffer ) {
    glFlush();
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    //fbo.unbind();
  }
}

static bool _full_screen = false;
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
  GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "Roller Ball Concept",  _full_screen ? glfwGetPrimaryMonitor() : NULL, NULL);
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


Shader *g_lightingShader = nullptr;   // TODO: Better way? We're instantiating locally and setting this pointer to local var in main()
void initObj()
{
  g_pObjManager = new ObjManager();
  if( g_pObjManager ) {
    // Add ball
    ObjImpl *pO = new BallObj();
    if( pO ) {
      pO->setPos( 54.0, 0.0, 52.0 );
      pO->setModel( ballModel );
      pO->setShader( g_lightingShader );
      pO->setGopManager( g_pGOPManager);
      g_pObjManager->add( pO );
    }

    // Add robot
    pO = new RobotObj();
    if( pO ) {
      pO->setPos( 43.0, 0.0, 43.0 );
      pO->setModel( robotModel );
      pO->setShader( g_lightingShader );
      pO->setGopManager( g_pGOPManager);
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
  g_lightingShader = &lightingShader;
  Shader lampShader("lamp.vs", "lamp.fs");
  Shader reflectShader("nolight.vs", "nolight.fs");

  // Initialize the playfield manager
  g_pPlayfield = new Playfield();

  // Initialize the Graphics Operator manager
  initGOP();

  // first, configure vertice array and vertice buffer objects
  glGenVertexArrays(1, &VAO);
  glGenBuffers(1, &VBO);

  glBindVertexArray(VAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, g_pPlayfield->getVerticeBufSize(), g_pPlayfield->getVertices(), GL_DYNAMIC_DRAW);

  glBindVertexArray(VAO);
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);
  glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
  glEnableVertexAttribArray(1);
  glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(6 * sizeof(float)));
  glEnableVertexAttribArray(2);

  // second, configure the light's VAO (VBO stays the same;
  // the vertices are the same for the light object which is also a 3D cube)
  unsigned int lightVAO;
  glGenVertexArrays(1, &lightVAO);
  glBindVertexArray(lightVAO);

  glBindBuffer(GL_ARRAY_BUFFER, VBO);

  // note that we update the lamp's position attribute's stride to reflect the updated buffer data
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
  glEnableVertexAttribArray(0);

  // load textures
  diffuseMap = loadTexture( "tileset.png");
  specularMap = loadTexture( "pattern.png");
  testTexture = loadTexture( "test.png");

  // Set start position
  camera.setPosition( { 56.0, 0, 54.0 } );

  // Load the models
  ballModel = new Model("dome2.obj");
  robotModel = new Model("nanosuit.obj");

  // init object manager AFTER models are loaded...
  initObj();

  // camera.setPitchAdjust( true );

  //
  // MAIN GAME LOOP
  //
  while (!glfwWindowShouldClose(window))
  {
    // per-frame time logic
    float currentFrame = glfwGetTime();
    deltaTime = currentFrame - lastFrame;
    lastFrame = currentFrame;

//#define PROFILE_FPS
#ifdef PROFILE_FPS
    std::cout << (1.0f / deltaTime) << std::endl;
#endif

    //
    // UPDATE GAME MOTION AND PHYSICS
    //
    g_pObjManager->update(&camera);

    // TEST CODE; REMOVE
    g_pPlayfield->test();
    // END TEST

    // input
    processInput(window);
    updateCamera(window);

    // Lemme out! Lemme out!
    if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
      glfwSetWindowShouldClose(window, true);

    // Do after all the movement and collision detection but before render
    //camera.UpdateVectors();

    //
    // RENDER
    //
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, MAX_VIEWING_DISTANCE);

    glm::mat4 view = camera.GetViewMatrix();

    // Render the view to the visible back buffer
    render( projection, view, lightingShader, false);
    glFlush();

    g_pObjManager->drawSEMAll(renderedTexture, &camera, &projection, &view);    // Draw spherical-environment mapping objects

    // Render all the objects

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

// TEMP; REMOVE
bool _updatedFront = false;
void updateCamera(GLFWwindow *window)
{
  camera.saveOldPosition();

  glm::vec3 pos, posPrev, dir;
  g_pObjManager->getSubjectPos(pos, posPrev, dir);

  pos.y += 3.0;
  camera.setPosition(pos);
  //if (!_updatedFront) {
    camera.setFront(dir);
  //  _updatedFront = true;
  //}
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
    g_pObjManager->setControl(CTL_UP);
    //camera.ProcessKeyboard( ctrlPressed ? PITCH_UP : FORWARD, deltaTime);
  }

  if (g_pObjManager->getSubject()) {

    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
      g_pObjManager->setControl(CTL_DOWN);
      //camera.ProcessKeyboard( ctrlPressed ? PITCH_DOWN : BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
      g_pObjManager->setControl(CTL_LEFT);
      //camera.ProcessKeyboard( ctrlPressed ? LEFT : TURNLEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
      g_pObjManager->setControl(CTL_RIGHT);
      //camera.ProcessKeyboard( ctrlPressed ? RIGHT : TURNRIGHT, deltaTime);
    }

    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_RELEASE) {
      g_pObjManager->clearControl(CTL_UP);
      //camera.ProcessKeyboard( ctrlPressed ? PITCH_UP : FORWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_RELEASE) {
      g_pObjManager->clearControl(CTL_DOWN);
      //camera.ProcessKeyboard( ctrlPressed ? PITCH_DOWN : BACKWARD, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_RELEASE) {
      g_pObjManager->clearControl(CTL_LEFT);
      //camera.ProcessKeyboard( ctrlPressed ? LEFT : TURNLEFT, deltaTime);
    }
    if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_RELEASE) {
      g_pObjManager->clearControl(CTL_RIGHT);
      //camera.ProcessKeyboard( ctrlPressed ? RIGHT : TURNRIGHT, deltaTime);
    }
  }


  glm::vec3 pos = camera.getPosition();
  pos.y = g_pPlayfield->getHeightAt( pos.x, pos.z ) + EYE_HEIGHT;
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

