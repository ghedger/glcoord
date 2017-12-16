// Graphical operations manager
// This manages a per-frame work queue of graphics work in the pipeline.
// These are model-level operations, not primitives, which are handled by 
// the graphics library.
//
// Copyright (C) 2017 Greg Hedger

#pragma once

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

class GOPManager
{
  public:
    GOPManager() { m_queueIdx = 0; }
    virtual ~GOPManager() {};

    GOPItem *getCurrentItem() {
      GOPItem *pI = 0;
      if( GOP_QUEUESIZE > m_queueIdx ) {
        pI = m_queue + ( m_queueIdx );
      }
      return pI;
    }

    void push() {
      m_queueIdx++;
    }

    GOPItem *pop() {
      GOPItem *pI = 0;
      if( m_queueIdx ) {
        pI = m_queue + (--m_queueIdx);
      }
      return pI;
    }

    void update()
    {
      GOPItem *pI;
      while( ( pI = pop() ) ) {
        switch( pI->m_opType ) {
          case GOP_STANDARD:
            //drawStandard( pI );
            break;
          case GOP_REFLECTANDFACE:
            drawReflect( pI );
            break;
          default:
            break;
        }
      }
    }

    // RenderWorld
    void drawWorldToBuffer( GOPItem *pI )
    {
      glm::vec3 normalSurface = { 1.0, 0.0, 0.0 };

      // Calculate incidence:
      //R = 2 * (I . N) * N - I)
      ///float dp = glm::dot(glm::normalize( delta ), normalSurface );
      ///glm::vec3 lookAt = (2 * dp) * normalSurface - glm::normalize( delta );
      glm::vec3 lookAt = { pI->m_eyePos.x - pI->m_pos.x, pI->m_eyePos.y - pI->m_pos.y, pI->m_eyePos.z - pI->m_pos.z };
      Camera cameraReflect(
        pI->m_pos,
        glm::normalize(glm::vec3(0.0f, 1.0f, 0.0))
      );

      cameraReflect.setPosition( glm::vec3( pI->m_pos.x, pI->m_pos.y, pI->m_pos.z ) );
      cameraReflect.UpdateVectors();

      glm::mat4 projectionReflect = glm::perspective( ( float ) ( M_PI / 2 + M_PI / 4 + M_PI / 8 ) /*glm::radians(cameraReflect.Zoom)*/, (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.01f, MAX_VIEWING_DISTANCE );

      glm::mat4 viewReflect = cameraReflect.GetLookAtMatrix( lookAt );
      glm::mat4 normal(1.0f);
      glm::mat4 modelReflect(1.0f);
      pI->m_pShader->use();
      pI->m_pShader->setMat4( "projection", projectionReflect);
      pI->m_pShader->setMat4( "view", viewReflect);
      pI->m_pShader->setMat4( "model", modelReflect);

      // Render the view to the offscreen buffer for environment mapping
      render( projectionReflect, viewReflect, *pI->m_pShader, true);
    }

    // Different draw ops
    void drawReflect( GOPItem *pI )
    {
      // First, need to render the scene from reflective object's point of view
      drawWorldToBuffer( pI );

      glActiveTexture(GL_TEXTURE0);
      glBindTexture(GL_TEXTURE_2D, pI->m_customTexture );

      glm::vec3 delta = pI->m_pos - pI->m_eyePos;
      //cameraReflect.setPosition( pI->m_pos );

      // DRAW RENDERED SEM BALL WITH NORMAL SHADER
      pI->m_pShader->use();
      // TODO: needs to be parameterized
      // directional light
      #if 0
      pI->m_pShader->setVec3("dirLight.direction", -0.1f, -0.8f, -0.1f);
      pI->m_pShader->setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
      pI->m_pShader->setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
      pI->m_pShader->setVec3("dirLight.specular", 0.9f, 0.9f, 0.9f);
      #endif

      pI->m_pShader->setMat4("projection", pI->m_projection);
      pI->m_pShader->setMat4("view", pI->m_view);
      glm::mat4 model3(1.0f);
      model3 = glm::translate(
          model3,
          pI->m_pos
          );
      // Calculate angle to player eye position
      float yaw = atan2(
          ( double )( pI->m_eyePos.z - pI->m_pos.z ),
          ( double )( pI->m_eyePos.x - pI->m_pos.x )
          );

      // Calculate pitch
      //float sign = ( delta.x * delta.z < 0.0f ) ? -1.0f : 1.0f;
      double xzDelta = sqrt(
          ( double ) ( delta.x * delta.x ) +
          ( double ) ( delta.z * delta.z )
          );
      float pitch = atan2( ( double )delta.y, (double )xzDelta );

      model3 = glm::rotate(model3, ( float ) ( ( -yaw ) ), glm::vec3( 0.0f, 1.0f, 0.0f ) );
      model3 = glm::rotate(model3,
          ( float ) ( -pitch ),
          glm::vec3( -1.0f, 0.0f, 1.0f )
          );
      pI->m_pShader->setMat4("model", model3);
      // TODO: Why pass by instance and not by pointer/reference?
      pI->m_pMeshModel->Draw( *pI->m_pShader, pI->m_customTexture);
    }

  protected:
    // TODO: Replace with an ordered list.  For now, though, to keep
    // moving we'll just have a fixed array.
    GOPItem       m_queue[ GOP_QUEUESIZE ];
    unsigned int  m_queueIdx;
};
