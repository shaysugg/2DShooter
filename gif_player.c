#include <stdlib.h>
#include "raylib.h"
#include "gif_player.h"

GifPlayer *LoadGifPlayer(char *path, int frameDelay)
{
    int animFrames = 0;
    Image imageAnim = LoadImageAnim(path, &animFrames);
    Texture2D texAnim = LoadTextureFromImage(imageAnim);
    Gif gif = {texAnim, imageAnim, animFrames};
    GifPlayer *gifPlayer;
    gifPlayer = malloc(sizeof(GifPlayer));
    gifPlayer->animFrames = animFrames;
    gifPlayer->currentAnimFrame = 0;
    gifPlayer->frameCounter = 0;
    gifPlayer->frameDelay = frameDelay;
    gifPlayer->gif = gif;
    gifPlayer->nextFrameDataOffset = 0;
    return gifPlayer;
}

void GoToNextFrame(GifPlayer *gifPlayer)
{
    gifPlayer->frameCounter++;
    if (gifPlayer->frameCounter >= gifPlayer->frameDelay)
    {
        gifPlayer->currentAnimFrame++;
        if (gifPlayer->currentAnimFrame >= gifPlayer->animFrames)
            gifPlayer->currentAnimFrame = 0;
        gifPlayer->nextFrameDataOffset = gifPlayer->gif.imageAnim.width * gifPlayer->gif.imageAnim.height * 4 * gifPlayer->currentAnimFrame;
        UpdateTexture(gifPlayer->gif.textureAnim, ((unsigned char *)gifPlayer->gif.imageAnim.data) + gifPlayer->nextFrameDataOffset);
        gifPlayer->frameCounter = 0;
    }
}

void DrawCurrentFrame(GifPlayer gifPlayer, int posX, int posY, Color tint)
{
    DrawTexture(gifPlayer.gif.textureAnim, posX, posY, tint);
}
void UnloadGifPlayer(GifPlayer gifPlayer)
{
    UnloadTexture(gifPlayer.gif.textureAnim);
    UnloadImage(gifPlayer.gif.imageAnim);
}