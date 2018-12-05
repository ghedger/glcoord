#include "playfield.h"
#include "objm.h"
#include "objimpl.h"

// TEMP; This shouldn't in the end be a global...
extern Playfield *g_pPlayfield;

ObjManager::ObjManager()
{
  initSentinels();
  setSubject(nullptr);

}

ObjManager::~ObjManager()
{
  // TODO: Implement and call DeleteAll();
}

void ObjManager::initSentinels()
{
  m_headSentinel.setPrev( (ObjImpl *) 0 );
  m_headSentinel.setNext( &m_tailSentinel );
  m_tailSentinel.setPrev( &m_headSentinel );
  m_tailSentinel.setNext( (ObjImpl *) 0 );
}

void ObjManager::add( ObjImpl *pO )
{
  if( pO ) {
    pO->insert( m_tailSentinel.getPrev() );
  }
}

void ObjManager::del( ObjImpl *pO )
{
  if( pO ) {
    pO->remove();
  }
}

void ObjManager::update()
{
  ObjImpl *pO = 0;
  if( ( pO = m_headSentinel.getNext() ) ) {
    while( pO != &m_tailSentinel )
    {
      pO->update();
      pO = pO->getNext();
    }
  }
}

// getSubject
// Get the subject the camera is following
// Entry: -
// Exit: pointer to object, or nullptr if none
ObjImpl *ObjManager::getSubject()
{
  return m_pSubject;
}

// setSubject
// Set the object that the camera should follow
// Entry: pointer to object
// Exit: -
void ObjManager::setSubject( ObjImpl *pO )
{
  m_pSubject = pO;
}

// getSubjectPos
// Get position and heading of object being followed by camera
void ObjManager::getSubjectPos( glm::vec3& pos, glm::vec3& posPrev, glm::vec3& dir )
{
  ObjImpl *pO = nullptr;
  pO = getSubject();
  if( nullptr == pO ) {
    setSubject( pO = m_headSentinel.getNext() );
  }
  if( pO ) {

    pos.x = pO->m_x - 5.0;
    pos.y = pO->m_y + 10.0;
    pos.z = pO->m_z - 5.0;
    float k = g_pPlayfield->getHeightAt( pos.x, pos.z ) + 0.5;
    if( pos.y < k ) {
      pos.y = k;
    }

    dir.x = pO->m_x - pos.x;
    dir.y = (pO->m_y - pos.y);
    dir.z = pO->m_z - pos.z;
    glm::normalize(dir);

#if 0
    posPrev.x = pO->m_xp;
    posPrev.y = pO->m_yp;
    posPrev.z = pO->m_zp;

    dir.x = pO->m_x - pO->m_xp;
    dir.y = pO->m_y - pO->m_yp;
    dir.z = pO->m_z - pO->m_zp;
    glm::normalize(dir);

    pos.x = pO->m_x - dir.x * 50.0;
    pos.y = pO->m_y - dir.y * 50.0 - 20.0;
    pos.z = pO->m_z - dir.z * 50.0;

    float k = g_pPlayfield->getHeightAt( pos.x, pos.z ) + 0.5;
    if( pos.y < k ) {
      pos.y = k;
    }

    dir.x = pO->m_x - pos.x;
    dir.y = (pO->m_y - pos.y) * 2.0;
    dir.z = pO->m_z - pos.z;
    glm::normalize(dir);
#endif

  }
}


void ObjManager::drawAll()
{
  ObjImpl *pO = 0;
  if( ( pO = m_headSentinel.getNext() ) ) {
    while( pO != &m_tailSentinel )
    {
      pO->draw();
      pO = pO->getNext();
    }
  }
}

void ObjManager::drawSEMAll(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view)
{
  ObjImpl *pO = 0;
  if( ( pO = m_headSentinel.getNext() ) ) {
    while( pO != &m_tailSentinel )
    {
      pO->drawSEM(renderedTexture, camera, projection, view);
      pO = pO->getNext();
    }
  }
}
