// Object manager class

#pragma once

#include "objimpl.h"


class ObjManager
{
  public:
    ObjManager();
    virtual ~ObjManager();

    void add( ObjImpl *pO );
    void del( ObjImpl *pO );
    void update();

  protected:
    virtual void initSentinels();
    ObjType   m_type;
    ObjImpl   m_headSentinel;
    ObjImpl   m_tailSentinel;
};

