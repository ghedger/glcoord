// gameObject
// Declaration for gameObject, a common ancestor 

#pragma once

class GameObj
{
  public:
    GameObj();
    virtual ~GameObj();

    virtual bool init() = 0;
    virtual void update() = 0;
    virtual void draw() = 0;
    virtual void move();
  protected:

    double m_x;
    double m_y;
    double m_z;

    double m_xr;
    double m_yr;
    double m_zr;

    double m_xv;
    double m_yv;
    double m_zv;
};
