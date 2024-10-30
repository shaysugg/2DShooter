#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "gif_player.h"

#define WWidth 650
#define WHeight 450

#define COVER_COUNT 3
#define BULLETS_MAX_COUNT 100

const double coverMargin = 10;

enum CharacterMovement
{
	idle,
	moving,
	cover,
	blocked,
};

enum MovementDirection
{
	left,
	right
};

struct Cover
{
	double width;
	Vector2 pos;
};

struct Aiming
{
	Vector2 endPos;
	float angle;
	float lentgh;
};

struct Character
{
	const double speed;
	const double width;
	enum CharacterMovement movement;
	enum MovementDirection direction;
	Vector2 pos;
	struct Cover behindCover;
	struct Aiming *aiming;
	char *animPath;
	GifPlayer *anim;
} character = {
	.movement = idle,
	.speed = 3,
	.width = 20,
	.pos = {550, WHeight / 2},
};

struct Cover covers[COVER_COUNT] = {
	{20, {200, WHeight / 2 - 20}},
	{20, {130, WHeight / 2 - 20}},
	{20, {420, WHeight / 2 - 20}},
};

struct Bullet
{
	Vector2 pos;
	float angle;
	float distance;
	float speed;
};

struct Bullet bullets[BULLETS_MAX_COUNT] = {};
int bulletsCount = 0;

void HandleCharacterMovements();
void HandleCharacterAiming();
void HandleAnimationOfCharacter();
void HandleBullets();
void HandleCharacterShooting();

void DrawCharacter();
void DrawCover(struct Cover);

void PutCharacterInCover(struct Character *, struct Cover);
void PassCharacterOverCover(struct Character *, struct Cover);

int main()
{
	Camera2D camera = {0};
	camera.offset = (Vector2){WWidth / 2, WHeight / 2};
	camera.rotation = 0;
	camera.zoom = 1;

	InitWindow(WWidth, WHeight, "2DShooter");
	SetTargetFPS(60);

	while (!WindowShouldClose())
	{
		HandleCharacterMovements();
		HandleCharacterAiming();
		HandleCharacterShooting();
		HandleBullets();

		HandleAnimationOfCharacter();

		BeginDrawing();

		ClearBackground(GRAY);
		// BeginMode2D(camera);

		DrawCharacter();
		for (int i = 0; i < COVER_COUNT; i++)
			DrawCover(covers[i]);
		// EndMode2D();
		if (character.aiming != NULL)
		{
			DrawCircleV(character.pos, 5, GREEN);
			DrawCircleV(character.aiming->endPos, 5, BLUE);
			DrawLineV(character.pos, character.aiming->endPos, YELLOW);
		}

		for (int i = 0; i < bulletsCount; i++)
			DrawCircleV(bullets[i].pos, 2, RED);

		EndDrawing();
	}

	UnloadGifPlayer(*character.anim);
	CloseWindow();
}

void HandleAnimationOfCharacter()
{
	char *animPath;
	switch (character.movement)
	{
	case idle:
		animPath = "resources/character-idle.gif";
		break;
	case moving:
		animPath = "resources/character-walking.gif";
		break;
	case cover:
		animPath = "resources/character-idle.gif";
		break;
	case blocked:
		animPath = "resources/character-idle.gif";
		break;
	}
	if (character.aiming != NULL)
		animPath = "resources/character-idle.gif";

	// Iniit and change if mode changed
	//  Not really good performant wise
	if (character.animPath == NULL ||
		strcmp(character.animPath, animPath))
	{
		free(character.anim);
		character.animPath = malloc(strlen(animPath));
		strcpy(character.animPath, animPath);
		puts(animPath);
		character.anim = LoadGifPlayer(animPath, 9);
	}
	// puts(animPath);
	// puts(character.animPath);
	// printf("%d", strcmp(character.animPath, animPath));
}

void DrawCharacter()
{
	if (character.anim == NULL)
		return;
	GoToNextFrame(character.anim);
	DrawCurrentFrame(*character.anim, character.pos.x, character.pos.y, WHITE);
}

void DrawCover(struct Cover cover)
{
	DrawRectangle(cover.pos.x, cover.pos.y, cover.width, 60, DARKGRAY);
}

void HandleCharacterMovements()
{
	// out of cover
	if (IsKeyPressed(KEY_C) && character.movement == cover)
	{
		if (character.pos.x < character.behindCover.pos.x)
			character.pos.x -= coverMargin;
		else
			character.pos.x += coverMargin;

		character.movement = idle;
	}

	// jump over cover
	if (IsKeyPressed(KEY_X) && (character.movement == cover || character.movement == blocked))
	{
		PassCharacterOverCover(&character, character.behindCover);
		character.movement = idle;
	}

	// check if covered or blocked
	for (int i = 0; i < COVER_COUNT; i++)
	{
		if (fabs(character.pos.x - covers[i].pos.x) < character.width)
		{
			character.movement = blocked;
			character.behindCover = covers[i];
			PutCharacterInCover(&character, covers[i]);
		}

		if (fabs(character.pos.x - covers[i].pos.x) < character.width + coverMargin && IsKeyPressed(KEY_C))
		{
			character.movement = cover;
			character.behindCover = covers[i];
			PutCharacterInCover(&character, covers[i]);
		}
	}

	// Go left and right
	if (character.movement == moving)
		character.movement = idle;

	if (IsKeyDown(KEY_LEFT) &&
		character.pos.x > 0 &&
		!(character.movement == blocked && character.pos.x > character.behindCover.pos.x) &&
		character.movement != cover)
	{
		character.pos.x -= character.speed;
		character.movement = moving;
		character.direction = left;
	}
	else if (IsKeyDown(KEY_RIGHT) &&
			 character.pos.x < WWidth &&
			 !(character.movement == blocked && character.pos.x < character.behindCover.pos.x) &&
			 character.movement != cover)
	{
		character.pos.x += character.speed;
		character.movement = moving;
		character.direction = right;
	}
}

void HandleCharacterAiming()
{
	KeyboardKey aimingKey = character.direction == right ? KEY_RIGHT : KEY_LEFT;
	if (character.aiming == NULL &&
		IsKeyDown(aimingKey) &&
		character.movement == cover)
	{
		character.aiming = malloc(sizeof(struct Aiming));
		character.aiming->angle = character.direction == right ? 0 : 180;
		character.aiming->lentgh = 100;
	}

	if (character.aiming != NULL &&
		(character.movement != cover || !IsKeyDown(aimingKey)))
		character.aiming = NULL;

	if (character.aiming == NULL)
		return;

	float startAngle = character.direction == left ? 130 : -50;
	float endAngle = character.direction == left ? 230 : 50;
	KeyboardKey keyUp = character.direction == right ? KEY_UP : KEY_DOWN;
	KeyboardKey keyDown = character.direction == right ? KEY_DOWN : KEY_UP;

	if (IsKeyDown(keyUp) && character.aiming->angle > startAngle)
		character.aiming->angle -= 2;
	if (IsKeyDown(keyDown) && character.aiming->angle < endAngle)
		character.aiming->angle += 2;

	character.aiming->endPos.x = cosf(character.aiming->angle * DEG2RAD) * character.aiming->lentgh + character.pos.x;
	character.aiming->endPos.y = sinf(character.aiming->angle * DEG2RAD) * character.aiming->lentgh + character.pos.y;
}

void HandleCharacterShooting()
{

	if (IsKeyPressed(KEY_Z))
	{
		struct Bullet bullet = {
			.distance = 0,
			.pos = character.pos,
			.speed = 4};

		if (character.aiming != NULL)
		{
			bullet.angle = character.aiming->angle;
		}
		else
		{
			float fault = character.movement == moving ? 10 : 3;
			bullet.angle = ((float)rand() / RAND_MAX) * fault * (rand() % 2 ? 1 : -1) +
						   (character.direction == left ? 180 : 0);
		}

		if (bulletsCount < BULLETS_MAX_COUNT)
		{
			bullets[bulletsCount] = bullet;
			bulletsCount++;
		}
	}
}

void HandleBullets()
{
	for (int i = 0; i < bulletsCount; i++)
	{
		bullets[i].distance += bullets[i].speed;
		bullets[i].pos.x = cosf(bullets[i].angle * DEG2RAD) * bullets[i].distance + character.pos.x;
		bullets[i].pos.y = sinf(bullets[i].angle * DEG2RAD) * bullets[i].distance + character.pos.y;
		// check if it can be removed from bullets cache
		if (bullets[i].pos.x > WWidth || bullets[i].pos.y > WHeight)
		{
			if (i != bulletsCount - 1)
				for (int j = i + 1; j < bulletsCount; j++)
					bullets[j - 1] = bullets[j];

			bulletsCount--;
		}
	}
}

void PutCharacterInCover(struct Character *character, struct Cover cover)
{
	if (character->pos.x < cover.pos.x)
		character->pos.x = cover.pos.x - character->width / 2 - cover.width / 2;
	else
		character->pos.x = cover.pos.x + character->width / 2 + cover.width / 2;
}
void PassCharacterOverCover(struct Character *character, struct Cover cover)
{
	if (character->pos.x < character->behindCover.pos.x)
		character->pos.x = character->behindCover.pos.x + character->width;
	else
		character->pos.x = character->behindCover.pos.x - character->width;
}