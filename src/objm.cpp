#include "objm.h"
#include "objimpl.h"

ObjManager::ObjManager()
{
  initSentinels();

}

ObjManager::~ObjManager()
{
  // TODO: Implement and call DeleteAll();
}

void ObjManager::initSentinels()
{
  m_headSentinel.setPrev( (ObjImpl *) 0 );
  m_headSentinel.setNext( &m_tailSentinel );
  m_tailSentinel.setPrev( &m_headSentinel );
  m_tailSentinel.setNext( (ObjImpl *) 0 );
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

void ObjManager::update()
{
  ObjImpl *pO = 0;
  if( ( pO = m_headSentinel.getNext() ) ) {
    while( pO != &m_tailSentinel )
    {
      pO->update();
      pO = pO->getNext();
    }
  }
}
