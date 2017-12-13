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
#include "sphere.h"

// prototypes
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
unsigned int loadTexture(const char *path);

// settings
unsigned int SCR_WIDTH = 512;
unsigned int SCR_HEIGHT = 512;
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

// TODO: These should not be global.  This whole thing should
// eventually live in a class.
unsigned int VBO, VAO;
unsigned int diffuseMap, specularMap;
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

void render( glm::mat4 projection, glm::mat4 view, Shader shader, bool toBuffer )
{
  if( toBuffer )
  {
    // Init FBO context
    if( !FBO ) {
      std::cout << "INITIALIZING FBO..." << std::endl;
      glGenFramebuffers( 1, &FBO );                     //Generate a framebuffer object(FBO)
      glBindFramebuffer( GL_FRAMEBUFFER, FBO );         // and bind it to the pipeline

      glGenTextures( 1, &renderedTexture );
      glBindTexture( GL_TEXTURE_2D, renderedTexture );
    }
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
  glm::mat4 model;
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
  for( int y = 40; y < 60; y++ ) {
    glBufferSubData( GL_ARRAY_BUFFER, g_pPlayfield->testOffset( y ) * sizeof(float), 20 * 6 * 8 * sizeof(float), g_pPlayfield->testGetRowPtr( y ) );
  }

  //shader.use();
  glDrawArrays(GL_TRIANGLES, 0, g_pPlayfield->getVerticeTot());

  if( ourRobot ) {
    // TEST ROBOT
    shader.use();
    glm::mat4 model3;
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
  // End GPH

#if 1
  // first, configure the cube's VAO (and VBO)
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
#endif

  // Set start position
  camera.setPosition( { 30.0, 0, 30.0 } );

  //Model ourModel("nanosuit.obj");
  Model ourModel("dome.obj");
  ourRobot = new Model("nanosuit.obj");

  float ballX = 21.0;
  float ballZ = 21.0;
  float ballY = g_pPlayfield->getHeightAt( ballX, ballZ ) + 0.5;

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

    //
    // UPDATE GAME MOTION AND PHYSICS
    //

    // TEST CODE; REMOVE
    g_pPlayfield->test();
    // END TEST

    // input
    processInput(window);

    // Do after all the movement and collision detection but before render
    camera.UpdateVectors();

    //
    // RENDER
    //

    #if 1
    glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
    #else
    glm::mat4 projection = glm::ortho(
      ( float ) 0.0f, // (SCR_WIDTH / 2 - SCR_WIDTH / 4),
      ( float ) (SCR_WIDTH / 4),
      ( float ) 0.0, //(SCR_HEIGHT / 2 - SCR_HEIGHT / 4 ),
      ( float ) (SCR_HEIGHT / 4),
      ( float ) 0.0f,
      ( float ) 100.0f
    );
    #endif
    //projection[0][0]= SCR_WIDTH / 2;
    //projection[1][1]= SCR_HEIGHT / 2;

		// TEMP CODE: Ball MOve
		{
			static float ballTheta = 0.0;

  			ballX += ballTheta;
				ballZ += ballTheta;
  			ballY = g_pPlayfield->getHeightAt( ballX, ballZ ) + 0.5;
				if( ballX > 192.0f || ballX < 8.0f ) {
					ballTheta = -ballTheta;
				}
		}

    glm::mat4 view = camera.GetViewMatrix();
    //camera.setPitch( 0.0 );

    glm::vec3 delta = cameraReflect.getPosition() - camera.getPosition();
    cameraReflect.setPosition( glm::vec3( ballX, ballY, ballZ ) );
    // Calculate angle to player eye position
    float yaw = atan2(
      ( double )( camera.getPosition().z - cameraReflect.getPosition().z ),
      ( double )( camera.getPosition().x - cameraReflect.getPosition().x )
    );

    // Calculate pitch
    double xzDelta = sqrt(
        ( double ) ( delta.x * delta.x ) +
        ( double ) ( delta.z * delta.z )
    );
    float pitch = atan2( ( double )delta.y, (double )xzDelta );

    glm::vec3 normalSurface = { 1.0, 0.0, 0.0 };

    // Calculate incidence:
    //R = 2 * (I . N) * N - I)
    float dp = glm::dot(glm::normalize( delta ), normalSurface );
    glm::vec3 lookAt = (2 * dp) * normalSurface - glm::normalize( delta );

    cameraReflect.UpdateVectors();
    #if 1
    glm::mat4 projectionReflect = glm::perspective( ( float ) ( M_PI / 2 + M_PI / 4 ) /*glm::radians(cameraReflect.Zoom)*/, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 200.0f);
    #else
    glm::mat4 projectionReflect = glm::ortho(
      ( float ) 0.0f, // (SCR_WIDTH / 2 - SCR_WIDTH / 4),
      ( float ) (SCR_WIDTH / 4),
      ( float ) 0.0, //(SCR_HEIGHT / 2 - SCR_HEIGHT / 4 ),
      ( float ) (SCR_HEIGHT / 4),
      ( float ) 0.0f,
      ( float ) 100.0f
    );
    #endif
    glm::mat4 viewReflect = cameraReflect.GetLookAtMatrix( lookAt );
    glm::vec3 eyeReflect = { camera.Position.x - ballX, camera.Position.y - ballY, camera.Position.z - ballZ };
    //glm::vec3 eyeReflect = { ballX - camera.Position.x , ballY - camera.Position.y, ballZ - camera.Position.z};

		glm::mat4 mult = {1.0,0.0,0.0,1.0,
			0.0,-1.0,0.0,1.0,
			0.0,0.0,1.0,1.0,
			1.0,1.0,1.0,1.0};
		//viewReflect = viewReflect * mult;

#if 1
    glm::mat4 normal = {
      1.0, 0.0, 0.0, 1.0,
      0.0, 1.0, 0.0, 1.0,
      0.0, 0.0, 1.0, 1.0,
      0.0, 0.0, 0.0, 1.0
    };

    glm::mat4 modelReflect;
    reflectShader.use();
    reflectShader.setMat4( "mNormal", normal );
    reflectShader.setVec3( "uEye", eyeReflect);
    reflectShader.setMat4( "projection", projectionReflect);
    reflectShader.setMat4( "view", viewReflect);
    reflectShader.setMat4( "model", modelReflect);
#endif

    // render the SEM for projection onto ball
    // render( projectionReflect, viewReflect, lightingShader, false);
static int debugShader = 0;
		debugShader++;

    render( projectionReflect, viewReflect, ( debugShader & 0x40 ) ? reflectShader : lightingShader, true);
    glFlush();

    // Render the view
    render( projection, view, lightingShader, false);

#if 1
    // TEST MESH
    //		lightingShader.use();
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, renderedTexture);

    static float mtheta = 0.0;
    //mtheta += 0.04;

#if 0
    //peye = normalize(mp.xyz - eye);

    //projection = glm::perspective(( float ) (M_PI - 0.000001), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    // normal = glm::perspective( ( float ) (M_PI / 2), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);

    reflectShader.setMat4("projection", projection);
    reflectShader.setMat4("view", view);
    reflectShader.setMat4( "mNormal", normal );
    reflectShader.setVec3( "uEye", eye);
    glm::mat4 model2;
    model2 = glm::translate(
        model2,
        glm::vec3(ballX,
          g_pPlayfield->getHeightAt(ballX, ballZ) + 1.0,
          ballZ)
    );
    model2 = glm::scale(model2, glm::vec3(2.0f, 2.0f, 2.0f));
    model2 = glm::rotate(model2, mtheta, glm::vec3(1.0f, 0.0f, 0.0f));

    reflectShader.setMat4("model", model2);
    //reflectShader.setMat4("model", model2);
    ourModel.Draw(reflectShader, renderedTexture);
#else

		// DRAW RENDERED SEM BALL

  	glActiveTexture(renderedTexture);
		lightingShader.use();
    lightingShader.setMat4("projection", projection);
    lightingShader.setMat4("view", view);
    //lightingShader.setMat4( "mNormal", normal );
    //lightingShader.setVec3( "uEye", camera.Position);
    glm::mat4 model2;
    model2 = glm::translate(
        model2,
        glm::vec3(ballX,
          g_pPlayfield->getHeightAt(ballX, ballZ) + 1.0,
          ballZ)
        );
    //model2 = glm::scale(model2, glm::vec3(2.0f, 2.0f, 2.0f));
    //model2 = glm::rotate(model2, ( float ) (-M_PI / 2.0f), glm::vec3(0.0f, 0.0f, 1.0f));
    //model2 = glm::rotate(model2, ( float ) (-M_PI / 2.0f), glm::vec3(0.0f, 1.0f, 0.0f));

		glm::vec3 vRotate = {
			1.0,
			0,
		  0
		};
		vRotate = glm::normalize( vRotate );

		model2 = glm::rotate(model2, ( float ) ( ( -yaw ) + M_PI / 4 ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
		model2 = glm::rotate(model2, ( float ) ( pitch ), vRotate );
		//model2 = glm::rotate(model2, ( float ) ( M_PI ), glm::vec3( 0.0f, 1.0f, 0.0f ) );

    lightingShader.setMat4("model", model2);
    //lightingShader.setMat4("model", model2);
    ourModel.Draw(lightingShader, renderedTexture);
#endif

#endif

    // glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
    glfwSwapBuffers(window);
    glfwPollEvents();
  }

  // optional: de-allocate all resources once they've outlived their purpose:
  ////glDeleteVertexArrays(1, &VAO);
  ////glDeleteVertexArrays(1, &lightVAO);
  ////glDeleteBuffers(1, &VBO);

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
    camera.ProcessKeyboard(FORWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
    camera.ProcessKeyboard(BACKWARD, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? LEFT : TURNLEFT, deltaTime);
  }
  if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
    camera.ProcessKeyboard( ctrlPressed ? RIGHT : TURNRIGHT, deltaTime);
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

