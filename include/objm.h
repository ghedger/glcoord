// Object manager class

#ifndef OBJMANAGER_H_
#define OBJMANAGER_H_

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "camera.h"
#include "objimpl.h"


class ObjManager
{
  public:
    ObjManager();
    virtual ~ObjManager();

    void add( ObjImpl *pO );
    void del( ObjImpl *pO );
    void update(Camera* camera);
    void setSubject( ObjImpl *pO );
    ObjImpl *getSubject();
    void getSubjectPos( glm::vec3& pos, glm::vec3& posPrev, glm::vec3& dir );
    void drawAll();
    void drawSEMAll(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view);
    void setControl(const unsigned int ctl) { getSubject()->setControl(ctl); }
    void clearControl(const unsigned int ctl) { getSubject()->clearControl(ctl); }

  protected:
    virtual void initSentinels();
    ObjType   m_type;
    ObjImpl   m_headSentinel;
    ObjImpl   m_tailSentinel;
    ObjImpl * m_pSubject;
    unsigned int m_ctl;
};
#endif // #ifndef OBJMANAGER_H_
