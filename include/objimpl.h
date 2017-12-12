// Pure virtual class for game object implementation

#pragma once

enum ObjType
{
  OT_INVALID = 0,
  PLAYER,
  ENEMY
};



class ObjImpl
{
  public:
    ObjImpl();
    virtual ~ObjImpl();
    virtual void Update() = 0;

    // Insert into linked list
    void Insert( ObjImpl * pAfter )
    {
      this->_pPrev = pAfter;
      this->_pNext = pAfter->_pNext;
      pAfter->_pNext = this;
      this->_pNext->_pPrev = this;
    }

    // Remove from linked list
    void Remove()
    {
      this->_pPrev->_pNext = this->_pNext;
      this->_pNext->_pPrev = this->_pPrev;
    }

    // getters/setters
    ObjType getType() { return _type; }
    void setType( ObjType t ) { _type = t; }


  protected:
    class ObjImpl * _pNext;
    class ObjImpl * _pPrev;

    ObjType     _type;
};
