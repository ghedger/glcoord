// heightplane.h
// Implement height plane

#pragma once

#include "gameobj.h"
#include <glm/glm.hpp>

#define HP_MAX_PARAMS 4

#define HP_PARAM_CHECKERBOARD_IDX 0

class HeightPlane : public GameObj
{
  public:
    HeightPlane();
    ~HeightPlane();


    // Interface implementation
    virtual void update() {};


    // Our implementations

    virtual float getHeightAt( const float x, const float y );
    glm::vec3 getNormalAt( const float x, const float y );
    bool getColdetAdj( const float x, const float y, const float z, const float radius, glm::vec3 *ap);

    virtual void setParam( const int paramIdx, const int val )
    {
      m_param[ paramIdx ] = val;
    };

    virtual int getParam( const int paramIdx )
    {
      return( m_param[ paramIdx ] );
    };

    virtual float getXMid() { return HP_XMID; };
    virtual float getYMid() { return HP_YMID; };


// TEST CODE; REMOVE
//  NOTE: All this vertice stuff needs to be encapsulated into its own mesh class
    float _theta = 0.0;
    virtual int testOffset( int y ) {
      return 40 * 6 * 8 + ( y * 6 * 8 * ( HP_XSIZE ) );
    };
    virtual void *testGetRowPtr( int y ) {
      return &m_vertices[ 40 * 6 * 8 + ( y * 6 * 8 * HP_XSIZE ) ];
    }
    virtual void test() {
#ifdef HP_TEST_HACK
      for( int y = 40; y < 60; y++ ) {
        for( int x = 40; x < 60; x++ ) {
          m_vertices[ x * 6 * 8 + 1  + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_vertices[ x * 6 * 8 + 9  + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_vertices[ x * 6 * 8 + 17 + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_vertices[ x * 6 * 8 + 25 + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_vertices[ x * 6 * 8 + 33 + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_vertices[ x * 6 * 8 + 41 + ( y * 6 * 8 * HP_XSIZE ) ] += sin( _theta ) / 7;
          m_heightPlane[ x ][ y ] += sin( _theta ) / 7;
        }
      }
      _theta += 0.05;
      _theta = fmod(_theta, M_PI * 2 );
#endif
    }
// END TEST CODE 

    virtual float *getVertices() { return m_vertices; };
    virtual int getVerticeTot() { return m_verticeTot; };
    virtual int getVerticeBufSize() { return ( int ) ( getVerticeTot() * 5 * sizeof( float ) ); };

  protected:
    bool initHeightPlane();
    void smoothNormals();
    double heightFn(double, double);

    float (*m_heightPlane)[ HP_YSIZE ];
    float calcZ(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3, const float x, const float y);

    int m_param[ HP_MAX_PARAMS ];
    float *m_vertices;
    int m_verticeTot;
};

