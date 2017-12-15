// Pure virtual class for game object implementation
// This implements only the linkage and contains no pure virtual functions.
// It is implmented twice in the instantiation of the object manager for the
// head and tail sentinel nodes.
// All other object types that want to be in the update loop should inherit from this class.

#pragma once

#include "gameobj.h"

enum ObjType
{
  OT_INVALID = 0,
  PLAYER,
  ENEMY
};


class ObjImpl : public GameObj
{
  public:
    ObjImpl() {};
    virtual ~ObjImpl() {};

    // Insert into linked list
    void insert( ObjImpl * pAfter )
    {
      this->_pPrev = pAfter;
      this->_pNext = pAfter->_pNext;
      pAfter->_pNext = this;
      this->_pNext->_pPrev = this;
    }

    // Remove from linked list
    void remove()
    {
      this->_pPrev->_pNext = this->_pNext;
      this->_pNext->_pPrev = this->_pPrev;
    }

    // Stock implementation of pure virtuals from GameObj
    bool init() { return true; };
    void draw() {};
    virtual void update() {};

    // Interface functions

    // getters/setters
    ObjType getType() { return _type; }
    void setType( ObjType t ) { _type = t; }
    class ObjImpl * getNext() { return _pNext; }
    class ObjImpl * getPrev() { return _pPrev; }

  protected:
    class ObjImpl * _pNext;
    class ObjImpl * _pPrev;

    ObjType     _type;
};
