// Pure virtual class for game object implementation
// This implements only the linkage and contains no pure virtual functions.
// It is implmented twice in the instantiation of the object manager for the
// head and tail sentinel nodes.
// All other object types that want to be in the update loop should inherit from this class.

#ifndef _OBJIMPL_H_
#define _OBJIMPL_H_

#include <shader.h>
#include <model.h>
#include <gopmanager.h>
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
    ObjImpl() { _model = nullptr; _shader = nullptr; }
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
    virtual void draw() {
      if (_model && _shader) {
        _shader->use();
        glm::mat4 ourMat(1.0f);

        // world transformation
        ourMat = glm::translate(
            ourMat,
            glm::vec3(m_x, m_y, m_z)
        );

        static float robotTheta = 0.0f;
        robotTheta += 0.001f;

        // TODO: Optimize so we do rotation all at once rather than three separate ones
        ourMat = glm::rotate( ourMat, (float) m_xr, glm::vec3 { 1.0, 0.0, 0.0 } );
        ourMat = glm::rotate( ourMat, (float) m_yr, glm::vec3 { 0.0, 1.0, 0.0 } );
        ourMat = glm::rotate( ourMat, (float) m_zr, glm::vec3 { 0.0, 0.0, 1.0 } );
        _shader->setMat4("model", ourMat);

        _model->Draw(*_shader);
      }
    }

    virtual void drawSEM( unsigned int renderedTexture, Camera *camera, glm::mat4 *projection, glm::mat4 *view) {
      GOPItem *pI = _gop_manager->getCurrentItem();

      printf( "OI: %f %f %f\n", (float) m_x, (float) m_y, (float) m_z );
      if( pI ) {
        // Set up our ball
        pI->m_pos = glm::vec3( (float) m_x, (float) m_y, (float) m_z );
        pI->m_eyePos = camera->getPosition();
        pI->m_view = *view;
        pI->m_projection = *projection;
        pI->m_opType = GOP_REFLECTANDFACE;
        pI->m_customTexture = renderedTexture;
        pI->m_pMeshModel = _model;
        pI->m_pShader = _shader;

        _gop_manager->push();
      }
    }

    virtual void update() {};

    // Interface functions
    // (none yet)

    // getters/setters
    ObjType getType() { return _type; }
    void setType( ObjType t ) { _type = t; }
    class ObjImpl * getNext() { return _pNext; }
    class ObjImpl * getPrev() { return _pPrev; }
    void setNext( ObjImpl *pO ) { _pNext = pO; }
    void setPrev( ObjImpl *pO ) { _pPrev = pO; }

    void setModel(Model *model) { _model = model; }
    void setShader(Shader *shader) { _shader = shader; }
    void setGopManager(GOPManager *gop_manager) { _gop_manager = gop_manager; }

  protected:
    class ObjImpl * _pNext;
    class ObjImpl * _pPrev;

    ObjType     _type;
    Model *     _model;
    Shader *    _shader;
    GOPManager *_gop_manager;
};

#endif // #define _OBJIMPL_H_
