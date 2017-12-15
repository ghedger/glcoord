// Implementation of main ball object

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
}

void BallObj::update()
{
  // Update velocity
  #define BALL_MAX_VEL 0.01f
  glm::vec3 norm = g_pPlayfield->getNormalAt( m_x, m_z );
  m_xv += norm.x / 2000.0f;
  m_zv += norm.y / 2000.0f;
  if( m_xv > BALL_MAX_VEL )
    m_xv = BALL_MAX_VEL;
  if( m_xv < -BALL_MAX_VEL )
    m_xv = -BALL_MAX_VEL;
  if( m_zv > BALL_MAX_VEL )
    m_zv = BALL_MAX_VEL;
  if( m_zv < -BALL_MAX_VEL )
    m_zv = -BALL_MAX_VEL;

  // Update position
  move();

  // Update playfield collision
  m_y = g_pPlayfield->getHeightAt( m_x, m_z );
}
