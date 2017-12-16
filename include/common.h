#pragma once

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

#define EYE_HEIGHT 0.5
#define MAX_VIEWING_DISTANCE 200.0f
