#ifndef ROBOTOBJ_H_
#define ROBOTOBJ_H_

#include "objimpl.h"

class RobotObj : public ObjImpl
{
  public:
    RobotObj();
    virtual ~RobotObj();

    bool init();
    void draw();
    void drawSEM(unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view);
    void update(Camera *camera);
};
#endif // #ifdef ROBOTOBJ_H_
