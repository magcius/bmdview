#ifndef SIMPLE_GL_COMMON_H
#define SIMPLE_GL_COMMON_H SIMPLE_GL_COMMON_H

void updateProjectionMatrix(int w, int h);
void handleCamera();

void setLastFrameSeconds(float s);
float getLastFrameSeconds();
float getAverageSecondsPerFrame();

#endif //SIMPLE_GL_COMMON_H
