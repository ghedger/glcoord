// Generic mesh class provides access to vertice primitives and calculates face normals

#include "mesh.h"
#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

// Default constructor
template <class T> Mesh<T>::Mesh()
{
}

template <class T> Mesh<T>::Mesh( unsigned int verticeMax, unsigned int verticeSize )
{
  init( verticeMax, verticeSize );
}

template <class T> bool Mesh<T>::init( unsigned int verticeMax, unsigned int verticeSize )
{
  bool bRet = true;
  // create vertices
  m_vertices = new float[ verticeMax * verticeSize ];
  m_verticeMax = verticeMax;
  m_verticeTot = 0;
  bRet = ( NULL != m_vertices );
}

// Destructor
template <class T> Mesh<T>::~Mesh()
{
  if( m_vertices ) {
    delete( m_vertices );
    m_vertices = NULL;
  }
}

// canAddVertices
// Check for available space
// Entry:   vertice total to add
// Exit:    space available == true
template <class T> bool Mesh<T>::canAddVertices( unsigned int tot )
{
  return m_verticeTot < ( m_verticeMax - ( 3 * m_verticeSize * tot ) );
}

// addFace
// Add a face
// Entry:   vector a
//          vector b
//          vector c
// Exit:    success == true
// TODO: Add texture index
template <class T> bool Mesh<T>::addFace( const glm::vec3 *a, const glm::vec3 *b, const glm::vec3 *c )
{
  bool bRet = true;
  if( bRet = canAddVertices( 3 ) ) {
    int i, j;
    i = j = m_verticeTot * m_verticeSize;
    // Add vertices
    m_vertices[ i++ ] = a->x;
    m_vertices[ i++ ] = a->y;
    m_vertices[ i++ ] = a->z;
    i += m_verticeSize - 3;

    m_vertices[ i++ ] = b->x;
    m_vertices[ i++ ] = b->y;
    m_vertices[ i++ ] = b->z;

    i += m_verticeSize - 3;
    m_vertices[ i++ ] = c->x;
    m_vertices[ i++ ] = c->y;
    m_vertices[ i++ ] = c->z;

    // Calculate normals
    glm::vec3 normal = glm::normalize( glm::cross( *c - *a, *b - *a ) );
    // Set the face to the same normal for now; it will be smoothed in a later step
    m_vertices[ j + 3 + 0 ] = m_vertices[ j + 11 + 0 ] = m_vertices[ j + 19 + 0 ] = normal.x;
    m_vertices[ j + 3 + 1 ] = m_vertices[ j + 11 + 1 ] = m_vertices[ j + 19 + 1 ] = normal.y;
    m_vertices[ j + 3 + 2 ] = m_vertices[ j + 11 + 2 ] = m_vertices[ j + 19 + 2 ] = normal.z;

    m_verticeTot += 3;    // TODO: Move to inc function
  }
  return bRet;
}

