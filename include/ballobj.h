#pragma once

#include "objimpl.h"

class BallObj : public ObjImpl
{
  public:
    BallObj();
    virtual ~BallObj();

    bool init();
    void draw();
    void update();
};

