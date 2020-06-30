// Implementation of main robot object


#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "robotobj.h"
#include "playfield.h"

// TEMP; This shouldn't in the end be a global...
extern Playfield *g_pPlayfield;

RobotObj::RobotObj()
{
}

RobotObj::~RobotObj()
{
}

bool RobotObj::init()
{
  return true;
}

void RobotObj::draw()
{
  ObjImpl::draw();
}

void RobotObj::drawSEM(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view)
{
  // do nothing
}


void RobotObj::update(Camera *camera)
{
  // Update velocity

  // TODO: This is placeholder only
  m_yr += 0.001f;

  // Update position
  move();

  // Update playfield collision
  m_y = g_pPlayfield->getHeightAt( m_x, m_z ) + 0.6;
}
