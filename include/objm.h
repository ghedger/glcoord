// Object manager class

#pragma once

#include "objimpl.h"


class ObjManager
{
  public:
    ObjManager();
    virtual ~ObjManager();

  protected:
    void UpdateAll();
    ObjType     _type;
    ObjImpl *   _headSentinel;
    ObjImpl *   _tailSentinel;
};

