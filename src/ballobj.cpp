// Implementation of main ball object


#include <math.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "ballobj.h"
#include "playfield.h"

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
  // Do nothing; only drawing for this obj is via SEM
}

void BallObj::drawSEM(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view)
{
  ObjImpl::drawSEM( renderedTexture, camera, projection, view );
}

void BallObj::update()
{
  // Update velocity
  #define BALL_MAX_VEL 0.075f
  glm::vec3 norm = g_pPlayfield->getNormalAt( m_x, m_z );
  m_xv += norm.x / 1000.0f;
  m_zv += norm.y / 1000.0f;
  double delta;

  // clamp velocity
  m_yv = m_y - m_yp;
  if ((delta = sqrt( m_xv * m_xv + m_zv * m_zv + m_yv * m_yv ) ) > BALL_MAX_VEL ) {
    m_xv = m_xv * ( BALL_MAX_VEL / delta );
    m_zv = m_zv * ( BALL_MAX_VEL / delta );
    m_yv = m_yv * ( BALL_MAX_VEL / delta );
  }

  // Update position
  move();

  // Update playfield collision
  //m_y = g_pPlayfield->getHeightAt( m_x, m_z );
  m_y = (double) g_pPlayfield->getHeightAt( (float)m_x, (float)m_z ) + 0.5;
}
