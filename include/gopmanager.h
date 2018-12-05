// Graphical operations manager
// This manages a per-frame work queue of graphics work in the pipeline.
// These are model-level operations, not primitives, which are handled by
// the graphics library.
//
// Copyright (C) 2017 Greg Hedger

#ifndef _GOPMANAGER_H_
#define _GOPMANAGER_H_

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include "model.h"
#include "camera.h"

#define GOP_QUEUESIZE 1024

// TODO: VERY TEMPORARY...

extern void render( glm::mat4 projection, glm::mat4 view, Shader& shader, bool toBuffer );
extern unsigned int SCR_WIDTH;
extern unsigned int SCR_HEIGHT;

enum GOPOpType
{
  GOP_INVALID = 0,
  GOP_STANDARD,
  GOP_REFLECTANDFACE
};

struct GOPItem
{
  GOPItem() {
    m_customTexture = 0;
    m_opType = GOP_STANDARD;
  }
  glm::vec3     m_pos;
  glm::vec3     m_eyePos;         // used for reflective objects
  glm::mat4     m_view;
  glm::mat4     m_projection;
  GOPOpType     m_opType;
  unsigned int  m_customTexture;
  Model *       m_pMeshModel;
  Shader *      m_pShader;
};

// GOPManager
// Graphics OPerations manager
// Manages the rendering
class GOPManager
{
  public:
    GOPManager() { m_queueIdx = 0; }
    virtual ~GOPManager() {};

    GOPItem *getCurrentItem();
    void push();
    GOPItem *pop();
    void update();
    void drawWorldToBuffer( GOPItem *pI );

    // Different draw ops
    void drawReflect( GOPItem *pI );

  protected:
    // TODO: Replace with an ordered list.  For now, though, to keep
    // moving we'll just have a fixed array.
    GOPItem       m_queue[ GOP_QUEUESIZE ];
    unsigned int  m_queueIdx;
};
#endif // #ifndef GOPMANAGER_H_
