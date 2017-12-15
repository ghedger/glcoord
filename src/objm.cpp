#include "objm.h"
#include "objimpl.h"

ObjManager::ObjManager()
{

}

ObjManager::~ObjManager()
{
  // TODO: Implement and call DeleteAll();
}

void ObjManager::add( ObjImpl *pO )
{
  if( pO ) {
    pO->insert( m_tailSentinel.getPrev() );
  }
}

void ObjManager::del( ObjImpl *pO )
{
  if( pO ) {
    pO->remove();
  }
}


void ObjManager::UpdateAll()
{
  ObjImpl *pO;
  if( pO = m_headSentinel.getNext() ) {
    while( pO != &m_tailSentinel )
    {
      pO->update();
      pO = pO->getNext();
    }
  }
}
