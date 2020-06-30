#ifndef _COMMON_H_
#define _COMMON_H_

// Playfield
#define HP_XSIZE 200
#define HP_YSIZE 200
#define HP_GRIDSIZE 1.0
#define HP_XMID (HP_GRIDSIZE * HP_XSIZE / 2)
#define HP_YMID (HP_GRIDSIZE * HP_YSIZE / 2)
#define HP_XMID_GRID (HP_XSIZE / 2)
#define HP_YMID_GRID (HP_YSIZE / 2)
#define HP_SMOOTH_NORMALS

#define FRICTION_COEFF 0.0052

// Motion
#define TURN_DAMPER 12
#define VEL_DAMPER 16
#define VEL_INC 0.1
#define VEL_MAX 0.7

#define VBO_BUFSIZE ((HP_XSIZE) * (HP_YSIZE) * 6 * 8)

#define EYE_HEIGHT 5
#define MAX_VIEWING_DISTANCE 200.0f

// Control
#define CTL_LEFT 1
#define CTL_RIGHT 2
#define CTL_UP 4
#define CTL_DOWN 8
#define SKY_COLOR 0.4f, 0.52f, 0.87f, 1.0f

#endif // #ifndef _COMMON_H_
