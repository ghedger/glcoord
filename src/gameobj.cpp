// gameobject implementation

#include "gameobj.h"

GameObj::GameObj()
{
}

GameObj::~GameObj()
{
}


// TODO: Add time delta
void GameObj::move()
{
  m_xp = m_x;
  m_yp = m_y;
  m_zp = m_z;

  m_x += m_xv;
  m_y += m_yv;
  m_z += m_zv;
}

