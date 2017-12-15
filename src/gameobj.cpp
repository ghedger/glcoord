// gameobject implementation

#include "gameobj.h"

GameObj::GameObj()
{
}

GameObj::~GameObj()
{
}


void GameObj::move()
{
  m_x += m_xv;
  m_y += m_yv;
  m_z += m_zv;
}

