// Object manager class

#ifndef OBJMANAGER_H_
#define OBJMANAGER_H_

#include "camera.h"
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "objimpl.h"


class ObjManager
{
  public:
    ObjManager();
    virtual ~ObjManager();

    void add( ObjImpl *pO );
    void del( ObjImpl *pO );
    void update();
    void drawAll();
    void drawSEMAll(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view);

  protected:
    virtual void initSentinels();
    ObjType   m_type;
    ObjImpl   m_headSentinel;
    ObjImpl   m_tailSentinel;
};
#endif // #ifndef OBJMANAGER_H_
