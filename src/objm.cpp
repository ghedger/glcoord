#include "objm.h"
#include "objimpl.h"

ObjManager::ObjManager()
{

}

ObjManager::~ObjManager()
{
  // TODO: Implement and call DeleteAll();
}

void ObjManager::UpdateAll()
{
  ObjImpl *pO;
  if( pO = _headSentinel.getNext() ) {
    while( pO != &_tailSentinel )
    {
      pO->update();
      pO = pO->getNext();
    }
  }
}
