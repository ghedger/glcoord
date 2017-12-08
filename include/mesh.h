// Generic mesh class provides access to vertice primitives and calculates face normals


#pragma once

#include <glm/glm.hpp>

template<class T>
class Mesh
{
  public:
    Mesh();
    Mesh(  unsigned int verticeMax, unsigned int verticeSize );
    virtual ~Mesh();
    bool init( unsigned int verticeMax, unsigned int verticeSize );
    bool addFace( const glm::vec3 *a, const glm::vec3 *b, const glm::vec3 *c );
    bool canAddVertices( unsigned int tot );

  protected:
    T *       m_vertices;
    unsigned int  m_verticeTot;
    unsigned int  m_verticeMax;
    unsigned int  m_verticeSize;
    const unsigned int DEFAULT_VERTICE_SIZE = 8;
};
