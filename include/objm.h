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

  protected:
    void UpdateAll();
    ObjType   m_type;
    ObjImpl   m_headSentinel;
    ObjImpl   m_tailSentinel;
};

