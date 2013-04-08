#ifndef BMD_CAMERA_H
#define BMD_CAMERA_H BMD_CAMERA_H

#include "Matrix44.h"

Matrix44f getCameraMatrix();
void store_camera_matrix(void);
void restore_camera_matrix(void);

void walkLeft(float amount);
void walkRight(float amount);
void walkForward(float amount);
void walkBack(float amount);
void walkUp(float amount);
void walkDown(float amount);
void turnLeft(float amount);
void turnRight(float amount);
void turnUp(float amount);
void turnDown(float amount);
void turnCCW(float amount);
void turnCW(float amount);

void prepareCamera();

#endif //BMD_CAMERA_H
