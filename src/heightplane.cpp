// HeightPlane implementation

#include "common.h"
#include "heightplane.h"

#include <GL/glut.h>
#include <GL/glu.h>
#include <glm/glm.hpp>
#include <glm/vec3.hpp>

#include <unistd.h>
#include <math.h>
#include <iostream>
#include <memory.h>
#include <stdio.h>

using namespace std;

HeightPlane::HeightPlane()
{
	m_param[0] = 0;
	m_param[1] = 0;
	m_param[2] = 0;
	m_param[3] = 0;

	m_vertices = new float[ VBO_BUFSIZE ];
	memset( m_vertices, 0, sizeof( float ) * VBO_BUFSIZE );

	// int(*ary)[sizeX] = new int[sizeY][sizeX];

	m_heightPlane = new float[ HP_XSIZE ][ HP_YSIZE ];
}

HeightPlane::~HeightPlane()
{
	m_verticeTot = 0;
	if( m_vertices ) {
		delete( m_vertices );
		m_vertices = NULL;
	}
	if( m_heightPlane ) {
		delete( m_heightPlane );
		m_heightPlane = NULL;
	}
}

double HeightPlane::heightFn( double x, double y )
{
	float h = sin( x * ( M_PI * 2 ) / HP_XSIZE ) * 6.2 +
		sin( y * ( M_PI * 2 ) / HP_YSIZE ) * 6.2;

	h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 2 ) * 0.3 +
		sin( y * ( M_PI * 2 ) / HP_YSIZE * 2 ) *  0.3;

	h -= sin( x * ( M_PI * 2 ) / HP_XSIZE * 4 ) * 2.1 +
		sin( y * ( M_PI * 2 ) / HP_YSIZE * 4 ) * 3.1;

	h += sin( x * ( M_PI * 2 ) / HP_XSIZE * 8 ) * 0.3 +
		sin( y * ( M_PI * 2 ) / HP_YSIZE * 8 ) * 0.2;
	return h;
}


// initHeightPlane
//
bool HeightPlane::initHeightPlane()
{
	bool bRet = true;
	int x, y;
	float texIdx = 1.0;
	int i = 0;
	double h = 0;

	// We allocate a slightly larger temporary buffer to satisfy
	// the (literal) edge conditions of vertice generation.
	// We then copy this buffer minus 1-unit border around the edges into
	// m_heightPlane.
	float (*pHp)[ HP_YSIZE + 2 ] = new float[ HP_XSIZE + 2 ][ HP_YSIZE + 2 ];

	for( y = 0; y < HP_YSIZE + 2; y++ ) {
		for( x = 0; x < HP_XSIZE + 2; x++ ) {
			h = heightFn(x, y);
			if( x > 0 ) {
				h += 1.0 / (HP_XSIZE + 2 - x );
				h += 1.0 / x;
			}
			if( y > 0 ) {
				h += 1.0 / (HP_YSIZE + 2 - y );
				h += 1.0 / y;
			}

#if 1
      if( !((x == HP_XSIZE / 2) && (y == HP_YSIZE / 2)) ) {
        h -= 0.2 / (sqrt(pow(abs(HP_XSIZE / 2 - x),2) + pow(abs(HP_YSIZE / 2 - y),2))/2);
      } else {
        h -= 0.2;
      }
#endif
			pHp[ x ][ y ] = h * 1.0;

			if(
					!(
						( !x ) ||
						( !y ) ||
						( HP_XSIZE + 1 ) <= x ||
						( HP_YSIZE + 1 ) <= y
					 )
				) {

				// NOTE: All this vertice stuff needs to be encapsulated into its own mesh class
				// Adjust model to it's actual final world position
				float xv = ( x - 1 ) * HP_GRIDSIZE;
				float zv = ( y - 1 ) * HP_GRIDSIZE;

				// Tile mapping
				unsigned int tile = 0; //( unsigned int ) ( ( ( x * ( M_PI * 2 ) / ( HP_XSIZE * 2 ) ) * 16) +
                                 //              ( ( y * ( M_PI * 2 ) / ( HP_YSIZE * 2 ) ) * 16) ) & 0xff;
        if ((rand() & 0x7fff) < 512) {
          tile = rand() & 0x0e;
        }
				float txs = ( float ) ( tile & 0x0f ) / 16.0;
				float tys = ( float ) ( ( tile >> 4 ) & 0x0f ) / 16.0;
				float txe = ( float ) ( (txs + 1.0 / 16.0));
				float tye = ( float ) ( (tys + 1.0 / 16.0));

				//cout << txs << " " <<tys << " " <<txe << " " <<tye << " " << std::endl;

				// Face A  - TOP LEFT HALF
				int j = i;    // for convenience
				m_vertices[ i++ ] = xv;
				m_vertices[ i++ ] = pHp[ x - 1 ][ y - 1 ];
				m_vertices[ i++ ] = zv;
				i += 3;   // skip normal (computed after vertice generation), but leave space
				m_vertices[ i++ ] = txs;
				m_vertices[ i++ ] = tys;

				m_vertices[ i++ ] = xv  + HP_GRIDSIZE;
				m_vertices[ i++ ] = pHp[ x ][ y - 1 ];
				m_vertices[ i++ ] = zv;
				i += 3;
				m_vertices[ i++ ] = txe;
				m_vertices[ i++ ] = tys;

				m_vertices[ i++ ] = xv;
				m_vertices[ i++ ] = pHp[ x - 1 ][ y ];
				m_vertices[ i++ ] = zv + HP_GRIDSIZE;
				i += 3;
				m_vertices[ i++ ] = txs;
				m_vertices[ i++ ] = tye;

				glm::vec3 a = { m_vertices[ j + 0 ], m_vertices[ j + 0 + 1 ], m_vertices[ j + 0 + 2 ] };
				glm::vec3 b = { m_vertices[ j + 8 ], m_vertices[ j + 8 + 1 ], m_vertices[ j + 8 + 2 ] };
				glm::vec3 c = { m_vertices[ j +16 ], m_vertices[ j +16 + 1 ], m_vertices[ j +16 + 2 ] };
				glm::vec3 normalA = glm::normalize( glm::cross( c - a, b - a ) );

				// Set the face to the same normal for now
				m_vertices[ j + 3 + 0 ] = m_vertices[ j + 11 + 0 ] = m_vertices[ j + 19 + 0 ] = normalA.x;
				m_vertices[ j + 3 + 1 ] = m_vertices[ j + 11 + 1 ] = m_vertices[ j + 19 + 1 ] = normalA.y;
				m_vertices[ j + 3 + 2 ] = m_vertices[ j + 11 + 2 ] = m_vertices[ j + 19 + 2 ] = normalA.z;

				// Face  B - BOTTOM RIGHT HALF
				j = i;    // for convenience
				m_vertices[ i++ ] = xv  + HP_GRIDSIZE;
				m_vertices[ i++ ] = pHp[ x ][ y ];
				m_vertices[ i++ ] = zv  + HP_GRIDSIZE;
				i += 3;
				m_vertices[ i++ ] = txe;
				m_vertices[ i++ ] = tye;

				m_vertices[ i++ ] = xv;
				m_vertices[ i++ ] = pHp[ x - 1 ][ y ];
				m_vertices[ i++ ] = zv + HP_GRIDSIZE;
				i += 3;
				m_vertices[ i++ ] = txs;
				m_vertices[ i++ ] = tye;

				m_vertices[ i++ ] = xv + HP_GRIDSIZE;
				m_vertices[ i++ ] = pHp[ x ][ y - 1 ];
				m_vertices[ i++ ] = zv;
				i += 3;
				m_vertices[ i++ ] = txe;
				m_vertices[ i++ ] = tys;

				a = { m_vertices[ j + 0 ], m_vertices[ j + 0 + 1 ], m_vertices[ j + 0 + 2 ] };
				b = { m_vertices[ j + 8 ], m_vertices[ j + 8 + 1 ], m_vertices[ j + 8 + 2 ] };
				c = { m_vertices[ j +16 ], m_vertices[ j +16 + 1 ], m_vertices[ j +16 + 2 ] };
				glm::vec3 normalB = glm::normalize( glm::cross( c - a, b - a ) );

				// Set the face to the same normal for now; it will be smoothed in a later step
				m_vertices[ j + 3 + 0 ] = m_vertices[ j + 11 + 0 ] = m_vertices[ j + 19 + 0 ] = normalB.x;
				m_vertices[ j + 3 + 1 ] = m_vertices[ j + 11 + 1 ] = m_vertices[ j + 19 + 1 ] = normalB.y;
				m_vertices[ j + 3 + 2 ] = m_vertices[ j + 11 + 2 ] = m_vertices[ j + 19 + 2 ] = normalB.z;
			}
		}
		// cout << i / 48 / HP_XSIZE << std::endl;
	}

	// Offset the height plane by 1x, 1y:
	//
	// xxxxxxxxxxxx
	// xxxxxxxxxxxx.
	// xxxxxxxxxxxx.
	//  ............
	for( y = 0; y < HP_YSIZE ; y++ ) {
		for( x = 0; x < HP_XSIZE ; x++ ) {
			m_heightPlane[ x ][ y ] = pHp[ x  ][ y  ];
		}
	}
	delete pHp;

	std::cout << i << " " << i / 5 << " " << i / 8 <<   std::endl;

	m_verticeTot = i / 5;
	return bRet;
}

// smoothNormals
// Called after height plane has been generated to average out the normals.
//
void HeightPlane::smoothNormals()
{
	// Surface Normal
#define SN_X(a) src[a + 3 + 0]
#define SN_Y(a) src[a + 3 + 1]
#define SN_Z(a) src[a + 3 + 2]

	// Surface Normal Destination
#define SND_X(a) m_vertices[a + 3 + 0]
#define SND_Y(a) m_vertices[a + 3 + 1]
#define SND_Z(a) m_vertices[a + 3 + 2]

	// Vertice index by face-vertice
	const int tla = 0;
	const int tra = 8;
	const int bla = 16;

	const int brb = 24;
	const int blb = 32;
	const int trb = 40;

	int a, b, l, r, al, ar, bl, br, i, x, y;
	// Since we will be performing non-orthogonal operations on the data
	// we need to copy it into its own source, read-only buffer, temporarily.
	float *src = new float[ VBO_BUFSIZE ];
	for( i = 0; i < VBO_BUFSIZE; i++ ) {
		src[ i ] = m_vertices[ i ];
	}

	i = 0;
	for( y = 0; y < HP_YSIZE; y++ ) {
		for( x = 0; x < HP_XSIZE; x++ ) {
			if( x == 0 || x == HP_XSIZE - 1 || y == 0 || y == HP_YSIZE - 1 ) {
				continue;
			}
			// Vertice-index arrangement:
			// TL-A:00
			// BL-A:16   BR-A:08
			//
			// TL-B:32   TR-B:40
			//           BR-B:24
			i = ( x * 8 * 6 ) + ( y * HP_XSIZE * 8 * 6 );
			a = i - ( HP_XSIZE ) * 8 * 6 ;										// above
			b = i + ( HP_XSIZE ) * 8 * 6 ;										// below
			l = i - 8 * 6;																// left
			r = i + 8 * 6;																// right

			al = a - 8 * 6;
			ar = a + 8 * 6;
			bl = b - 8 * 6;
			br = b + 8 * 6;

			// Average top-right corner (top-left face)
			SND_X( i + tra ) = ( SN_X( i + tra ) + SN_X( a + brb ) + SN_X( r + tla ) + SN_X( ar + bla ) ) / 4;
			SND_Y( i + tra ) = ( SN_Y( i + tra ) + SN_Y( a + brb ) + SN_Y( r + tla ) + SN_Y( ar + bla ) ) / 4;
			SND_Z( i + tra ) = ( SN_Z( i + tra ) + SN_Z( a + brb ) + SN_Z( r + tla ) + SN_Z( ar + bla ) ) / 4;

			// Average top-right corner (bottom-right face)
			SND_X( i + trb ) = ( SN_X( i + trb ) + SN_X( a + brb ) + SN_X( r + tla ) + SN_X( ar + blb ) ) / 4;
			SND_Y( i + trb ) = ( SN_Y( i + trb ) + SN_Y( a + brb ) + SN_Y( r + tla ) + SN_Y( ar + blb ) ) / 4;
			SND_Z( i + trb ) = ( SN_Z( i + trb ) + SN_Z( a + brb ) + SN_Z( r + tla ) + SN_Z( ar + blb ) ) / 4;

			// Average bottom-left corner (top-left face)
			SND_X( i + bla ) = ( SN_X( i + bla ) + SN_X( b + tla ) + SN_X( l + brb ) + SN_X( bl + tra ) ) / 4;
			SND_Y( i + bla ) = ( SN_Y( i + bla ) + SN_Y( b + tla ) + SN_Y( l + brb ) + SN_Y( bl + tra ) ) / 4;
			SND_Z( i + bla ) = ( SN_Z( i + bla ) + SN_Z( b + tla ) + SN_Z( l + brb ) + SN_Z( bl + tra ) ) / 4;

			// Average bottom-left corner (bottom-right face)
			SND_X( i + blb ) = ( SN_X( i + blb ) + SN_X( b + tla ) + SN_X( l + brb ) + SN_X( bl + trb ) ) / 4;
			SND_Y( i + blb ) = ( SN_Y( i + blb ) + SN_Y( b + tla ) + SN_Y( l + brb ) + SN_Y( bl + trb ) ) / 4;
			SND_Z( i + blb ) = ( SN_Z( i + blb ) + SN_Z( b + tla ) + SN_Z( l + brb ) + SN_Z( bl + trb ) ) / 4;

			// Average top-left corner (top-left)
			SND_X( i + tla ) = ( SN_X( i + tla ) + SN_X( a + bla ) + SN_X( l + tra ) + SN_X( al + brb ) ) / 4;
			SND_Y( i + tla ) = ( SN_Y( i + tla ) + SN_Y( a + bla ) + SN_Y( l + tra ) + SN_Y( al + brb ) ) / 4;
			SND_Z( i + tla ) = ( SN_Z( i + tla ) + SN_Z( a + bla ) + SN_Z( l + tra ) + SN_Z( al + brb ) ) / 4;

			// Average bottom right corner (bottom-right)
			SND_X( i + brb ) = ( SN_X( i + brb ) + SN_X( r + blb ) + SN_X( b + trb ) + SN_X( br + tla ) ) / 4;
			SND_Y( i + brb ) = ( SN_Y( i + brb ) + SN_Y( r + blb ) + SN_Y( b + trb ) + SN_Y( br + tla ) ) / 4;
			SND_Z( i + brb ) = ( SN_Z( i + brb ) + SN_Z( r + blb ) + SN_Z( b + trb ) + SN_Z( br + tla ) ) / 4;

#if 0
			SND_X( i+tla ) = 1.0;
			SND_Y( i+tla ) = 0.0;
			SND_Z( i+tla ) = 0.0;
#endif
		}
	}

	// clean up copy of vertices
	delete src;
}

// TODO: Move this to a vector math lib
glm::vec3 crossProduct( const glm::vec3 v1,  const glm::vec3 v2 )
{
	glm::vec3 cpv;

	cpv.x = v1.y * v2.z - v2.y * v1.z;
	cpv.y = v2.x * v1.z - v1.x * v2.z;
	cpv.z = v1.x * v2.y - v2.x * v1.y;

	return cpv;
}

// TODO: Move this to a vector math lib
// Normalize a vec
glm::vec3 normalizeVec( glm::vec3 v )
{
	glm::vec3 nv;
	float sum = 0.0;

	sum = abs(v.x) + abs(v.y) + abs(v.z);
	nv.x = v.x / sum;
	nv.y = v.y / sum;
	nv.z = v.z / sum;

	return nv;
}


// TODO: Move this to a vector math lib
float dotProduct( glm::vec3& u, glm::vec3& v )
{
	return( u.x * v.x + u.y * v.y + u.z * v.z );
}

#define dot(u,v)   ((u).x * (v).x + (u).y * (v).y + (u).z * (v).z)
// norm = length of vector
#define norm(v)    sqrt(dot(v,v))
// distance = norm of difference
#define d(u,v)     norm(u-v)

// TODO: Move this to a vector math lib
float closestPointOnPlane( const glm::vec3 v0, const glm::vec3 v1, glm::vec3& origin, glm::vec3& p, glm::vec3 *cp, glm::vec3 *pn )
{
	glm::vec3 closest;
	glm::vec3 n;
	n = normalizeVec(crossProduct( v0, v1 ));

	float     sb, sn, sd;
	glm::vec3 diff;
	diff.x = p.x - origin.x;
	diff.y = p.y - origin.y;
	diff.z = p.z - origin.z;

	sn = -(dot( n, diff ));

	sd = dot( n, n );
	sb = sn / sd;

	closest.x = p.x + (n.x * sb);
	closest.y = p.y + (n.y * sb);
	closest.z = p.z + (n.z * sb);

	cp->x = closest.x;
	cp->y = closest.y;
	cp->z = closest.z;

	diff.x = p.x - closest.x;
	diff.y = p.y - closest.y;
	diff.z = p.z - closest.z;

	float sign = dotProduct( n, diff );
	*pn = n;

	float dp = abs(dotProduct( p, diff ));

	return sqrt( dp ) * (sign > 0 ? 1 : -1);
}

bool HeightPlane::getColdetAdj( const float fx, const float fy, const float fz, const float radius, glm::vec3 *ap )
{
	bool bRet = false;
	int x = (int) (fx / HP_GRIDSIZE);
	int y = (int) (fy / HP_GRIDSIZE);
	glm::vec3 cp = { fx, fy, fz };
	if( x >= 0 && x < HP_XSIZE - 1 ) {
		if( y >= 0 && y < HP_YSIZE - 1 ) {
			// Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
			float ix, iy;
			ix = fmod( fx, HP_GRIDSIZE );
			iy = fmod( fy, HP_GRIDSIZE );

			//assert(ix >= 0.0 && ix < HP_GRIDSIZE);
			//assert(iy >= 0.0 && iy < HP_GRIDSIZE);

			glm::vec3 n = {0,0,0};
			glm::vec3 p = { fx, fy, fz };
			glm::vec3 origin;
			glm::vec3 v0;
			glm::vec3 v1;
			if( ix > ( HP_GRIDSIZE - iy ) ) {
				// top/left
				origin = { fx - ix, fy - iy, m_heightPlane[ x ][ y ] };
				v1 = { HP_GRIDSIZE, 0.0, m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x ][ y ] };
				v0 = { 0.0, HP_GRIDSIZE, m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x ][ y ] };
			} else {
				// bottom/right
				origin = {  ( fx - ix ) + HP_GRIDSIZE, ( fy - iy ) + HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y + 1 ] };
				v0 = { 0, -HP_GRIDSIZE, m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x + 1 ][ y + 1 ] };
				v1 = { -HP_GRIDSIZE, 0, m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x + 1 ][ y + 1 ] };
			}

			float dn = closestPointOnPlane( v0, v1, origin, p, &cp, &n );
			glm::vec3 surfacePoint = {
				fx + ( n.x * radius ),
				fy + ( n.y * radius ),
				fz + ( n.z * radius )
			};
			// Now that we have the closest point on the plane to the center,
			// we must call again with the surface so we can an appropriate distance.
			dn = closestPointOnPlane( v0, v1, origin, surfacePoint, &cp, &n );
			if( dn > 0 && -sqrt( pow(cp.x - fx, 2) + pow(cp.y - fy, 2) + pow(cp.z - fz, 2)) < radius ) {
				ap->x = cp.x - ( n.x * radius );
				ap->y = cp.y - ( n.y * radius );
				ap->z = cp.z - ( n.z * radius );
				bRet = true;
				printf("b");
			}
#if 1
			printf("BR { %2.2f %2.2f %2.2f }  { %2.2f %2.2f %2.2f }  %2.2f\n",
					cp.x, cp.y, cp.z,
					n.x, n.y, n.z,
					dn
					);
#endif
		}
	}
	return bRet;
}

//#define DEBUG_HP
// getHeightAt
// Get precise height on height plane at position { x, y }
// This facilitate smooth movement along the surface.
// (Nasty little function chock full of linear algeraic formulae)
// Entry:  x position
//        y position
// Exit:  z position
//
float HeightPlane::getHeightAt( const float fx, const float fy )
{
	float h = 0.0;
	int x = (int) (fx / HP_GRIDSIZE);
	int y = (int) (fy / HP_GRIDSIZE);
	if( x >= 0 && x < HP_XSIZE - 1 ) {
		if( y >= 0 && y < HP_YSIZE - 1 ) {
//#define NO_HEIGHT_INTERPOLATION
#ifdef NO_HEIGHT_INTERPOLATION
			return m_heightPlane[ x ][ y ];
#endif
			// Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
			float ix, iy;
			float ax, ay;
			ix = fmod( fx, HP_GRIDSIZE );
			iy = fmod( fy, HP_GRIDSIZE );

			//assert(ix >= 0.0 && ix < HP_GRIDSIZE);
			//assert(iy >= 0.0 && iy < HP_GRIDSIZE);

			ax = fx - ix;
			ay = fy - iy;

			// What's the vector, Victor?
			//
			// Spatial point convention:
			//   1   2
			//   3   4
			//
			// Origin:
			//   1
			// Vect { 2, 4 } has (2) as the origin.
			if( ix > ( HP_GRIDSIZE - iy ) ) {
				//iy = HP_GRIDSIZE - iy;
				// top/left
				// mean z
				glm::vec3 v1 = {
					ax,
					ay,
					m_heightPlane[ x ][ y ]
				};
				glm::vec3 v2 = {
					( ax + HP_GRIDSIZE ),
					ay,
					m_heightPlane[ x + 1 ][ y ]
				};
				glm::vec3 v3 = {
					ax,
					( ay + HP_GRIDSIZE ),
					m_heightPlane[ x ][ y + 1 ]
				};
				h = calcZ( v1, v2, v3, fx, fy );
			} else {
				// bottom/right
				glm::vec3 v2 = {
					( ax + HP_GRIDSIZE),
					ay,
					m_heightPlane[ x + 1 ][ y ]
				};
				glm::vec3 v4 = {
					( ax + HP_GRIDSIZE ),
					( ay + HP_GRIDSIZE ),
					m_heightPlane[ x + 1 ][ y + 1 ]
				};
				glm::vec3 v3 = {
					ax,
					(ay + HP_GRIDSIZE),
					m_heightPlane[ x ][ y + 1 ]
				};
				h = calcZ( v2, v3, v4, fx, fy );
			}
		}
	}
	return h;
}

glm::vec3 HeightPlane::getNormalAt( const float fx, const float fy )
{
	int x = (int) (fx / HP_GRIDSIZE);
	int y = (int) (fy / HP_GRIDSIZE);
	glm::vec3 cp = { 0, 0, 0 };
	if( x >= 0 && x < HP_XSIZE - 1 ) {
		if( y >= 0 && y < HP_YSIZE - 1 ) {
			// Find which triangular half of the grid square { fx, fy } is in (top/right or bottom/left)
			float ix, iy;
			ix = fmod( fx, HP_GRIDSIZE );
			iy = fmod( fy, HP_GRIDSIZE );

			//assert(ix >= 0.0 && ix < HP_GRIDSIZE);
			//assert(iy >= 0.0 && iy < HP_GRIDSIZE);

			if( ix > ( HP_GRIDSIZE - iy ) ) {
				//iy = HP_GRIDSIZE - iy;
				// top/left
				// mean z
				glm::vec3 v12 = {
					HP_GRIDSIZE,
					0.0,
					m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x ][ y ]
				};
				glm::vec3 v13 = {
					0.0,
					HP_GRIDSIZE,
					m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x ][ y ]
				};
				cp = normalizeVec(crossProduct( v12, v13 ));

			} else {
				// bottom/right
				glm::vec3 v42 = {
					0,
					-HP_GRIDSIZE,
					m_heightPlane[ x + 1 ][ y ] - m_heightPlane[ x + 1 ][ y + 1 ]
				};
				glm::vec3 v43 = {
					-HP_GRIDSIZE,
					0,
					m_heightPlane[ x ][ y + 1 ] - m_heightPlane[ x + 1 ][ y + 1 ]
				};
				cp = normalizeVec( crossProduct( v43, v42 ) );
			}
		}
	}
	return cp;
}

// calcZ
// Use Barycentric coordinate algorithm to derive z from y on heightplane triangle
// Entry:  p1 vector1
//        p2 vector2
//        p3 vector3
//        x on triangle
//        y on triangle
// Exit:  z at { x, y }
float HeightPlane::calcZ(const glm::vec3 p1, const glm::vec3 p2, const glm::vec3 p3, const float x, const float y)
{
	float det = (p2.y - p3.y) * (p1.x - p3.x) + (p3.x - p2.x) * (p1.y - p3.y);
	float l1 = ((p2.y - p3.y) * (x - p3.x) + (p3.x - p2.x) * (y - p3.y)) / det;
	float l2 = ((p3.y - p1.y) * (x - p3.x) + (p1.x - p3.x) * (y - p3.y)) / det;
	float l3 = 1.0f - l1 - l2;
	float z = l1 * p1.z + l2 * p2.z + l3 * p3.z;
#ifdef DEBUG_BARYCENTRIC
	printf("{%2.2f %2.2f %2.2f} {%2.2f %2.2f %2.2f} {%2.2f %2.2f %2.2f} [%2.2f %2.2f %2.2f] %2.2f %2.2f  %2.4f\n",
			p1.x, p1.y, p1.z,
			p2.x, p2.y, p2.z,
			p3.x, p3.y, p3.z,
			l1, l2, l3,
			x, y, z
			);
#endif
	return z;

	//return l1 * p1.z + l2 * p2.z + l3 * p3.z;
}

