#include <raylib.h>

#ifndef FRAMEANIMATOR_H
#define FRAMEANIMATOR_H

typedef struct FrameAnimator
{
    Texture2D texture;
    Rectangle currentRec;
    int currentFrame;
    int framesSpeed;
    int framesCounter;
    int framesCount;
    int currentRow;
    int rowsCount;
} FrameAnimator;

void UpdateFrameAnimator(FrameAnimator *fa);

void DrawFrameAnimator(FrameAnimator fa, Vector2 position);
FrameAnimator LoadFrameAnimator(char *texturePath, int framesCount, int rowsCount, int framesSpeed);

#endif