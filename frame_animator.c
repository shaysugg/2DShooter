#include "frame_animator.h"

void UpdateFrameAnimator(FrameAnimator *fa)
{
    fa->framesCounter++;

    if (fa->framesCounter >= (60 / fa->framesSpeed))
    {
        fa->framesCounter = 0;
        fa->currentFrame++;

        if (fa->currentFrame >= fa->framesCount)
            fa->currentFrame = 0;

        fa->currentRec.x = (float)fa->currentFrame * (float)fa->texture.width / fa->framesCount;
        fa->currentRec.y = (float)fa->currentRow * (float)fa->texture.height / fa->rowsCount;
    }
}

void DrawFrameAnimator(FrameAnimator fa, Vector2 position)
{
    DrawTextureRec(fa.texture, fa.currentRec, position, WHITE);
}

FrameAnimator LoadFrameAnimator(char *texturePath, int framesCount, int rowsCount, int currentRow, int framesSpeed)
{
    Texture2D texture = LoadTexture(texturePath); // Texture loading
    FrameAnimator fa = {.texture = texture,
                        .currentFrame = 0,
                        .framesCount = framesCount,
                        .framesCounter = 0,
                        .currentRow = currentRow,
                        .rowsCount = rowsCount,
                        .framesSpeed = framesSpeed,
                        .currentRec = {0.0f, 0.0f, (float)texture.width / framesCount, (float)texture.height / rowsCount}};
    return fa;
}