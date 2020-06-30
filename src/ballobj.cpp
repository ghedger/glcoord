// Implementation of main ball object


#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "common.h"
#include "ballobj.h"
#include "playfield.h"
#include "camera.h"

// TEMP; This shouldn't in the end be a global...
extern Playfield *g_pPlayfield;

BallObj::BallObj()
{
}

BallObj::~BallObj()
{
}

bool BallObj::init()
{
  return true;
}

void BallObj::draw()
{
  // Do nothing; only drawing for this obj is via SEM, which must be rendered
  // after everything else.
}

void BallObj::drawSEM(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view)
{
  ObjImpl::drawSEM( renderedTexture, camera, projection, view );
}
// TODO: MOVE THESE CONSTANTS
#define BALL_MAX_VEL 0.075f
#define BALL_ACCEL 0.0007f
// update
// Update the main character object
void BallObj::update(Camera *camera)
{

  // update control
  glm::mat4 view = camera->GetViewMatrix();
  glm::vec3 front = camera->GetViewVector();

  if (_ctl & CTL_UP) {
    m_xv += BALL_ACCEL * front.x;
    m_zv += BALL_ACCEL * front.z;
  }

  if (_ctl & CTL_DOWN) {
    m_xv -= BALL_ACCEL * front.x;
    m_zv -= BALL_ACCEL * front.z;

    //m_xv *= 0.98;
    //m_zv *= 0.98;
  }

  if (_ctl & CTL_LEFT) {
    m_xv += BALL_ACCEL * front.z;
    m_zv -= BALL_ACCEL * front.x;
  }


  if (_ctl & CTL_RIGHT) {
    m_xv -= BALL_ACCEL * front.z;
    m_zv += BALL_ACCEL * front.x;

    //camera->Turn(0.3);
  }


  // Update position
  move();

  // Update playfield collision
  //m_y = g_pPlayfield->getHeightAt( m_x, m_z );
  double ydelta = m_y;
  double heightOnPF = (double) g_pPlayfield->getHeightAt( (float)m_x, (float)m_z );
  //if (m_y <= heightOnPF + 0.5) {
    m_y = (double) g_pPlayfield->getHeightAt( (float)m_x, (float)m_z ) + 0.5;
    // Update velocity
    glm::vec3 norm = g_pPlayfield->getNormalAt( m_x, m_z );
    m_xv += norm.x / 1000.0f;
    m_zv += norm.y / 1000.0f;
    m_ylaunchv = m_y - ydelta;
    //m_yv = 0;
#if 0
  } else {
    if (m_ylaunchv != 0.0) {
      m_yv = m_ylaunchv;
      m_ylaunchv = 0.0;
    } else {
      // Gravity
      m_yv -= 0.04;
    }
  }
#endif

  double delta;

  // clamp velocity
  //m_yv = m_y - m_yp;
  if ((delta = sqrt( m_xv * m_xv + m_zv * m_zv + m_yv * m_yv ) ) > BALL_MAX_VEL ) {
    m_xv = m_xv * ( BALL_MAX_VEL / delta );
    m_zv = m_zv * ( BALL_MAX_VEL / delta );
    //m_yv = m_yv * ( BALL_MAX_VEL / delta );
  }

  // Apply friction
  // TODO: This should depend on the tile!
  m_xv *= 0.997;
  m_zv *= 0.997;



}
