// gameObject
// Declaration for gameObject, a common ancestor

#ifndef GAMEOBJ_H_
#define GAMEOBJ_H_

#include "camera.h"

class GameObj
{
  public:
    GameObj();
    virtual ~GameObj();

    virtual bool init() = 0;
    virtual void update(Camera *camera) = 0;
    virtual void draw() = 0;
    virtual void move();
    virtual void setPos( double x, double y, double z ) {
      m_x = x;
      m_y = y;
      m_z = z;
    }
    virtual void setVel( double x, double y, double z ) {
      m_xv = x;
      m_yv = y;
      m_zv = z;
    }
    virtual void setRot( double x, double y, double z ) {
      m_xr = x;
      m_yr = y;
      m_zr = z;
    }

  protected:

    // Position
    double m_x;
    double m_y;
    double m_z;

    // Previous position
    double m_xp;
    double m_yp;
    double m_zp;

    // Rotation
    double m_xr;
    double m_yr;
    double m_zr;

    // Cartesian velocity
    double m_xv;
    double m_yv;
    double m_zv;
};
#endif // #ifndef GAMEOBJ_H_
