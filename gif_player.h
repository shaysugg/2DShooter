#include "raylib.h"

typedef struct Gif
{
    Texture2D textureAnim;
    Image imageAnim;
    int animFrames;
} Gif;

typedef struct GifPlayer
{
    Gif gif;
    int animFrames;
    int currentAnimFrame;
    unsigned int nextFrameDataOffset;
    int frameDelay;
    int frameCounter;
} GifPlayer;

GifPlayer *LoadGifPlayer(char *path, int frameDelay);
void GoToNextFrame(GifPlayer *gifPlayer);
void DrawCurrentFrame(GifPlayer gifPlayer, int posX, int posY, Color tint);
void UnloadGifPlayer(GifPlayer gifPlayer);