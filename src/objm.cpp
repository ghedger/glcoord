#include "playfield.h"
#include "objm.h"
#include "objimpl.h"

// TEMP; This shouldn't in the end be a global...
extern Playfield *g_pPlayfield;

//
// OBJECT MANAGEMENT
//

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

void ObjManager::update(Camera *camera)
{
  ObjImpl *pO = 0;
  if( ( pO = m_headSentinel.getNext() ) ) {
    while( pO != &m_tailSentinel )
    {
      pO->update(camera);
      pO = pO->getNext();
    }
  }
}

//
// CUSTOM OBJECT UPDATE CODE
//

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
#if 0
    glm::vec3 look_at;
    look_at.x = cos(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    look_at.y = sin(glm::radians(Pitch));
    look_at.z = sin(glm::radians(Yaw)) * cos(glm::radians(Pitch));
    //glm::vect3 look_at_norm Front = glm::normalize(look_at);
#endif

    // TEMP: Should not be static and should not be here!
    static float cam_x = 0.0;
    static float cam_z = 0.0;

    // Only update camera direction if object is moving at
    // any reasonable speed.  If object is almost stopped,
    // camera goes haywire.
    if (abs(pO->m_xv) + abs(pO->m_yv) + abs(pO->m_zv) > 0.06) {

      glm::vec3 normdir = glm::normalize(glm::vec3(pO->m_xv, pO->m_yv, pO->m_zv));



      cam_x += ( ( pO->m_x - 5.0 * normdir.x) - cam_x) / 256.0;
      cam_z += ( ( pO->m_z - 5.0 * normdir.z) - cam_z) / 256.0;
    }

    pos.x = cam_x; //pO->m_x - 5.0 * normdir.x;
    pos.y = pO->m_y + 4.0;
    pos.z = cam_z; //pO->m_z - 5.0 * normdir.z;
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



//
// RENDERING
//


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

// drawSEMAll
// Draw Spherical Environment Mapped object
// Entry: texture
//        camera
//        projection matrix
//        view matrix
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
