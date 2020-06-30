#ifndef _BALLOBJ_H_
#define _BALLOBJ_H_

#include "objimpl.h"

class BallObj : public ObjImpl
{
  public:
    BallObj();
    virtual ~BallObj();

    bool init();
    void draw();
    void drawSEM(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view);
    void update(Camera *camera);

  private:
    double m_ylaunchv;
};
#endif // #ifndef _BALLOBJ_H_
