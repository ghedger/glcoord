// gopmanager.cpp
//
// Copyright (C) 2018 Gregory P. Hedger

#include "common.h"
#include "gopmanager.h"

GOPItem *GOPManager::getCurrentItem() {
  GOPItem *item = 0;
  if( GOP_QUEUESIZE > m_queueIdx ) {
    item = m_queue + ( m_queueIdx );
  }
  return item;
}

void GOPManager::push() {
  m_queueIdx++;
}

GOPItem *GOPManager::pop() {
  GOPItem *item = 0;
  if( m_queueIdx ) {
    item = m_queue + (--m_queueIdx);
  }
  return item;
}

void GOPManager::update()
{
  GOPItem *item;
  while( ( item = pop() ) ) {
    switch( item->m_opType ) {
      case GOP_STANDARD:
        //drawStandard( item );
        break;
      case GOP_REFLECTANDFACE:
        drawReflect( item );
        break;
      default:
        break;
    }
  }
}

// RenderWorld
void GOPManager::drawWorldToBuffer( GOPItem *item )
{
  glm::vec3 normalSurface = { 1.0, 0.0, 0.0 };

  // Calculate incidence:
  //R = 2 * (I . N) * N - I)
  ///float dp = glm::dot(glm::normalize( delta ), normalSurface );
  ///glm::vec3 lookAt = (2 * dp) * normalSurface - glm::normalize( delta );
  glm::vec3 lookAt = { item->m_eyePos.x - item->m_pos.x, item->m_eyePos.y - item->m_pos.y, item->m_eyePos.z - item->m_pos.z };
  Camera cameraReflect(
    item->m_pos,
    glm::normalize(glm::vec3(0.0f, 1.0f, 0.0))
  );

  cameraReflect.setPosition( glm::vec3( item->m_pos.x, item->m_pos.y, item->m_pos.z ) );
  cameraReflect.UpdateVectors();

  glm::mat4 projectionReflect = glm::perspective(
    ( float ) ( M_PI / 2 + M_PI / 4 + M_PI / 8 ) /*glm::radians(cameraReflect.Zoom)*/,
    (float)SCR_WIDTH / (float)SCR_HEIGHT,
    0.01f,
    MAX_VIEWING_DISTANCE
  );

  glm::mat4 viewReflect = cameraReflect.GetLookAtMatrix( lookAt );
  glm::mat4 normal(1.0f);
  glm::mat4 modelReflect(1.0f);
  item->m_pShader->use();
  item->m_pShader->setMat4( "projection", projectionReflect);
  item->m_pShader->setMat4( "view", viewReflect);
  item->m_pShader->setMat4( "model", modelReflect);

  // Render the view to the offscreen buffer for environment mapping
  render( projectionReflect, viewReflect, *item->m_pShader, true);
}

// Different draw ops
void GOPManager::drawReflect( GOPItem *item )
{
  // First, need to render the scene from reflective object's point of view
  drawWorldToBuffer( item );

  glActiveTexture(GL_TEXTURE0);
  glBindTexture(GL_TEXTURE_2D, item->m_customTexture );

  glm::vec3 delta = item->m_pos - item->m_eyePos;
  //cameraReflect.setPosition( item->m_pos );

  // DRAW RENDERED SEM BALL WITH NORMAL SHADER
  item->m_pShader->use();
  // TODO: needs to be parameterized
  // directional light
  #if 0
  item->m_pShader->setVec3("dirLight.direction", -0.1f, -0.8f, -0.1f);
  item->m_pShader->setVec3("dirLight.ambient", 0.6f, 0.6f, 0.6f);
  item->m_pShader->setVec3("dirLight.diffuse", 0.8f, 0.8f, 0.8f);
  item->m_pShader->setVec3("dirLight.specular", 0.9f, 0.9f, 0.9f);
  #endif

  item->m_pShader->setMat4("projection", item->m_projection);
  item->m_pShader->setMat4("view", item->m_view);
  glm::mat4 model3(1.0f);
  model3 = glm::translate(
      model3,
      item->m_pos
      );
  // Calculate angle to player eye position
  float yaw = atan2(
      ( double )( item->m_eyePos.z - item->m_pos.z ),
      ( double )( item->m_eyePos.x - item->m_pos.x )
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
  item->m_pShader->setMat4("model", model3);
  // TODO: Why pass by instance and not by pointer/reference?
  item->m_pMeshModel->Draw( *item->m_pShader, item->m_customTexture);
}


