#include <raylib.h>
#include <raymath.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include "frame_animator.h"

#define WWidth 950
#define WHeight 450
#define FPS 60

#define COVER_COUNT 1
#define BULLETS_MAX_COUNT 100
#define ENEMIES_BULLETS_MAX_COUNT 100
#define ENEMIES_MAX_COUNT 50
#define ENEMY_DEATH_TIME 2
#define ENEMY_PUSH_BACK 30

const double coverMargin = 10;
float groundPosV;

typedef enum CharacterMovement
{
	idle,
	moving,
	cover,
	blocked,
} CharacterMovement;

typedef enum MovementDirection
{
	left,
	right
} MovementDirection;

typedef struct Cover
{
	Rectangle rec;
} Cover;

typedef struct Aiming
{
	Vector2 endPos;
	float angle;
	float lentgh;
} Aiming;

typedef struct Character
{
	float speed;
	Vector2 pos;
	float width;
	float height;
	CharacterMovement movement;
	MovementDirection direction;
	Cover behindCover;
	Aiming *aiming;
	FrameAnimator fa;
	int health;
} Character;

Character character;

Cover covers[COVER_COUNT];

typedef struct Bullet
{
	Vector2 pos;
	float angle;
	float distance;
	float speed;
	Vector2 initPos;
} Bullet;

Bullet bullets[BULLETS_MAX_COUNT] = {};
int bulletsCount = 0;

Bullet enemiesBullets[ENEMIES_BULLETS_MAX_COUNT] = {};
int enemiesBulletsCount = 0;

typedef enum EnemyStatus
{
	alive,
	dead
} EnemyStatus;

typedef struct EnemyShooting
{
	float shootingCoolDownTime;
	float shootingDelay;
	float numberOfBullets;
	float currentShootingCoolDownTime;
	float currentShootingDelay;
	float currentNumberOfBullets;
} EnemyShooting;

typedef struct Enemy
{
	Vector2 pos;
	float width;
	float height;
	FrameAnimator fa;
	EnemyStatus status;
	int deathTime;
	int health;
	EnemyShooting *shooting;

} Enemy;

Enemy enemies[ENEMIES_MAX_COUNT] = {};
int enemiesCount = 0;

void LoadResources();
void LoadInitial();

void HandleCharacterMovements();
void HandleCharacterAiming();
void UpdateAnimations();
void HandleBullets();
void HandleCharacterShooting();
void HandleEnemies();
void UpdateBulletPosition(Bullet *bullet);
void RemoveBullet(int index, Bullet from[], int *bulletCount);
void HandleEnemyShooting(EnemyShooting *shooting, Vector2 enemYPos);

void DrawCharacter();
void DrawEnemy(Enemy);
void DrawCover(Cover);
void DrawUI();

void PutCharacterBehindCover(Character *, Cover);
void PassCharacterOverCover(Character *, Cover);

Rectangle GetCharacterRectangle(Character character);
float DistanceBetweenRectanglesX(Rectangle rec1, Rectangle rec2);
float DistanceBetweenRectanglesX(Rectangle rec1, Rectangle rec2);

Vector2 CharacterCenter(Character);
Vector2 EnemyCenter(Enemy);

int main()
{
	Camera2D camera = {0};
	camera.offset = (Vector2){WWidth / 2, WHeight / 2};
	camera.rotation = 0;
	camera.zoom = 1;

	SetTargetFPS(FPS);
	InitWindow(WWidth, WHeight, "2DShooter");

	LoadInitial();
	LoadResources();

	while (!WindowShouldClose())
	{
		HandleCharacterMovements();
		HandleCharacterAiming();
		HandleCharacterShooting();
		HandleBullets();
		HandleEnemies();
		UpdateAnimations();

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

		for (int i = 0; i < enemiesBulletsCount; i++)
			DrawCircleV(enemiesBullets[i].pos, 2, BLACK);

		for (int i = 0; i < enemiesCount; i++)
			DrawEnemy(enemies[i]);

		DrawLine(0, groundPosV, GetScreenWidth(), groundPosV, BLACK);
		DrawUI();

		EndDrawing();
	}

	UnloadTexture(character.fa.texture);

	// TODO: is this working correct
	for (int i = 0; i < enemiesCount; i++)
		free(enemies[i].shooting);

	// TODO more free when we have alloc
	CloseWindow();
}

void LoadInitial()
{
	groundPosV = GetScreenHeight() / 2 + 100;
	float cHeight = 255 / 4;
	float cWidth = 175 / 3;
	character = (Character){
		.pos = (Vector2){
			50,
			groundPosV - cHeight,
		},
		.movement = idle,
		.speed = 3,
		.width = cWidth,
		.height = cHeight,
		.health = 5};

	enemiesCount = 3;
	for (int i = 0; i < enemiesCount; i++)
	{
		EnemyShooting *shooting = (EnemyShooting *)malloc(sizeof(EnemyShooting));
		shooting->numberOfBullets = 3,
		shooting->shootingCoolDownTime = 2 * FPS,
		shooting->shootingDelay = 0.2 * FPS,

		enemies[i] = (Enemy){
			.pos = (Vector2){
				(i + 1) * 180,
				groundPosV - cHeight + (i % 2 == 0 ? 100 : -100),
			},
			.status = alive,
			.deathTime = -1,
			.width = cWidth,
			.height = cHeight,
			.health = 1,
			.shooting = shooting,
		};
	}

	float coverHeight = 70;
	float coverWidth = 20;
	for (int i = 0; i < COVER_COUNT; i++)
		covers[i] = (Cover){(Rectangle){(i + 1) * 100, groundPosV - coverHeight, coverWidth, coverHeight}};
}

void LoadResources()
{
	character.fa = LoadFrameAnimator("resources/character.png", 3, 4, 1, 6);
	for (int e = 0; e < enemiesCount; e++)
		enemies[e].fa = LoadFrameAnimator("resources/enemy.png", 3, 4, 3, 6);
}

void UpdateAnimations()
{
	switch (character.movement)
	{
	case idle:
		character.fa.currentRow = 0;
		break;
	case moving:
		character.fa.currentRow = character.direction == left ? 1 : 2;
		break;
	default:
		character.fa.currentRow = 0;
		break;
	}
	if (character.aiming != NULL)
		character.fa.currentRow = 0;

	UpdateFrameAnimator(&character.fa);

	for (int e = 0; e < enemiesCount; e++)
		UpdateFrameAnimator(&enemies[e].fa);
}

void DrawCharacter()
{
	DrawRectangleRec(GetCharacterRectangle(character), character.health > 0 ? YELLOW : ORANGE);
	DrawFrameAnimator(character.fa, character.pos);
}

void DrawCover(Cover cover)
{
	DrawRectangleRec(cover.rec, DARKGRAY);
}

void DrawEnemy(Enemy enemy)
{
	DrawRectangleRec((Rectangle){enemy.pos.x, enemy.pos.y, enemy.fa.currentRec.width, enemy.fa.currentRec.height}, enemy.status == dead ? RED : YELLOW);
	DrawFrameAnimator(enemy.fa, enemy.pos);
}

void DrawUI()
{
	DrawRectangle(0, 0, GetScreenWidth(), 30, ColorAlpha(DARKGRAY, 200));
	DrawText(TextFormat("Health: %d", character.health), 10, 10, 10, WHITE);

	DrawText(TextFormat("Enemy cool down: %f", enemies[0].shooting->currentShootingCoolDownTime), 10, 20, 10, WHITE);
	DrawText(TextFormat("Enemy shooting delay: %f", enemies[0].shooting->currentShootingDelay), 10, 30, 10, WHITE);
	DrawText(TextFormat("Enemy number of bullets delayyy: %f", enemies[0].shooting->currentNumberOfBullets), 10, 40, 10, WHITE);
	DrawText(TextFormat("bc: %d", enemiesBulletsCount), 10, 50, 10, WHITE);
	DrawText(TextFormat("bc: %d", bulletsCount), 10, 60, 10, WHITE);
	// if (enemiesBulletsCount > 0)
	// {

	// 	DrawText(TextFormat("b: %f , %f", enemiesBullets[0].pos.x, enemiesBullets[0].pos.y), 10, 60, 10, WHITE);
	// }
}

void HandleCharacterMovements()
{
	// out of cover
	if (IsKeyPressed(KEY_C) && character.movement == cover)
	{
		if (character.pos.x < character.behindCover.rec.x)
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

		if (CheckCollisionRecs((Rectangle){character.pos.x, character.pos.y, character.fa.currentRec.width, character.fa.currentRec.height}, covers[i].rec))
		{
			character.movement = blocked;
			character.behindCover = covers[i];
			PutCharacterBehindCover(&character, covers[i]);
		}

		if (DistanceBetweenRectanglesX(GetCharacterRectangle(character), covers[i].rec) < coverMargin && IsKeyPressed(KEY_C))
		{
			character.movement = cover;
			character.behindCover = covers[i];
			PutCharacterBehindCover(&character, character.behindCover);
		}
	}

	// Go left and right
	if (character.movement == moving)
		character.movement = idle;

	if (IsKeyDown(KEY_LEFT) &&
		character.pos.x > 0 &&
		!(character.movement == blocked && character.pos.x > character.behindCover.rec.x) &&
		character.movement != cover)
	{
		character.pos.x -= character.speed;
		character.movement = moving;
		character.direction = left;
	}
	else if (IsKeyDown(KEY_RIGHT) &&
			 character.pos.x < WWidth &&
			 !(character.movement == blocked && character.pos.x < character.behindCover.rec.x) &&
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
		character.aiming = malloc(sizeof(Aiming));
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
			.initPos = (Vector2){character.pos.x + character.width, character.pos.y + character.height / 2},
			.speed = 8};

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

void HandleEnemyShooting(EnemyShooting *shooting, Vector2 enemyCenter)
{
	// improve this shit
	// try to apply const on necessary variables
	if (shooting->currentShootingCoolDownTime == 0)
	{
		if (shooting->currentShootingDelay == 0)
		{
			if (shooting->currentNumberOfBullets == 0)
			{
				// restart
				shooting->currentShootingCoolDownTime = shooting->shootingCoolDownTime;
				shooting->currentNumberOfBullets = shooting->numberOfBullets;
				shooting->currentShootingDelay = shooting->shootingDelay;
			}
			else
			{
				shooting->currentShootingDelay = shooting->shootingDelay;
				shooting->currentNumberOfBullets--;

				// shoot
				if (enemiesBulletsCount < ENEMIES_BULLETS_MAX_COUNT)
				{
					Vector2 characterCenter = CharacterCenter(character);
					enemiesBulletsCount++;
					enemiesBullets[enemiesBulletsCount - 1] = (Bullet){
						.angle = RAD2DEG * atan((enemyCenter.y - characterCenter.y) / (enemyCenter.x - characterCenter.x)) + (characterCenter.x < enemyCenter.x ? 180 : 0),
						.initPos = enemyCenter,
						.speed = 6};
				}
			}
		}
		else
		{
			shooting->currentShootingDelay--;
		}
	}
	else
	{
		shooting->currentShootingCoolDownTime--;
	}
}

void HandleBullets()
{
	for (int i = 0; i < enemiesBulletsCount; i++)
	{
		UpdateBulletPosition(&enemiesBullets[i]);

		if (enemiesBullets[i].pos.x > WWidth || enemiesBullets[i].pos.x < 0 || enemiesBullets[i].pos.y < 0 || enemiesBullets[i].pos.y > WHeight)
		{
			RemoveBullet(i, enemiesBullets, &enemiesBulletsCount);
		}

		// check if bullets collide with character
		if (CheckCollisionCircleRec(enemiesBullets[i].pos, 5, GetCharacterRectangle(character)))
		{
			// TODO: FIX
			if (character.behindCover.rec.x != 0 || character.aiming != NULL)
			{
				RemoveBullet(i, enemiesBullets, &enemiesBulletsCount);
				character.health--;
			}
		};
	}

	for (int i = 0; i < bulletsCount; i++)
	{
		UpdateBulletPosition(&bullets[i]);

		// check if it can be removed from bullets cache
		if (bullets[i].pos.x > WWidth || bullets[i].pos.x < 0 || bullets[i].pos.y < 0 || bullets[i].pos.y > WHeight)
		{
			RemoveBullet(i, bullets, &bulletsCount);
		}

		// check if bullets collide with an enemy
		for (int e = 0; e < enemiesCount; e++)
		{
			if (enemies[e].status == alive &&
				CheckCollisionCircleRec(bullets[i].pos, 5, (Rectangle){enemies[e].pos.x, enemies[e].pos.y, enemies[e].width, enemies[e].height}))
			{
				// remove bullet
				RemoveBullet(i, bullets, &bulletsCount);
				// mark enemy as dead
				enemies[e].health--;
				if (enemies[e].health == 0)
					enemies[e].status = dead;
				enemies[e].deathTime = ENEMY_DEATH_TIME * 60;
			}
		}
	}
}

void UpdateBulletPosition(Bullet *bullet)
{
	bullet->distance += bullet->speed;
	bullet->pos.x = cosf(bullet->angle * DEG2RAD) * bullet->distance + bullet->initPos.x;
	bullet->pos.y = sinf(bullet->angle * DEG2RAD) * bullet->distance + bullet->initPos.y;
}

void RemoveBullet(int index, Bullet from[], int *bc)
{
	if (index != *bc - 1)
		for (int j = index + 1; j < *bc; j++)
			from[j - 1] = from[j];

	int temp = *bc;
	*bc = temp - 1;
}

void HandleEnemies()
{
	for (int i = 0; i < enemiesCount; i++)
	{
		// removing enemies after certain time
		if (enemies[i].deathTime > 0 && enemies[i].status == dead)
			enemies[i].deathTime--;

		if (enemies[i].deathTime <= 0 && enemies[i].status == dead)
		{
			if (i != enemiesCount - 1)
				for (int j = i + 1; j < enemiesCount; j++)
					enemies[j - 1] = enemies[j];

			enemiesCount--;
		};

		// enemies and character collison
		if (CheckCollisionRecs(
				(Rectangle){character.pos.x, character.pos.y, character.width, character.height},
				(Rectangle){enemies[i].pos.x, enemies[i].pos.y, enemies[i].width, enemies[i].height}))
		{
			character.health--;

			character.pos.x -= ENEMY_PUSH_BACK;
		}

		if (enemies[i].status == alive)
			HandleEnemyShooting(enemies[i].shooting, EnemyCenter(enemies[i]));
	}
}

void PutCharacterBehindCover(Character *character, Cover cover)
{

	if (character->pos.x < cover.rec.x)
		character->pos.x = cover.rec.x - character->fa.currentRec.width;
	else
		character->pos.x = cover.rec.x + cover.rec.width;
}
void PassCharacterOverCover(Character *character, Cover cover)
{
	if (character->pos.x < cover.rec.x)
		character->pos.x = cover.rec.x + cover.rec.width;
	else
		character->pos.x = cover.rec.x - character->fa.currentRec.width;
}

Rectangle GetCharacterRectangle(Character character)
{
	return (Rectangle){character.pos.x, character.pos.y, character.fa.currentRec.width, character.fa.currentRec.height};
}

float DistanceBetweenRectanglesX(Rectangle rec1, Rectangle rec2)
{
	if (CheckCollisionRecs(rec1, rec2))
	{
		return 0;
	}

	if (rec1.x < rec2.x)
		return rec2.x - (rec1.x + rec1.width);
	else
		return rec1.x - (rec2.x + rec2.width);
}

Vector2 CharacterCenter(Character character)
{
	return (Vector2){character.pos.x + character.width / 2, character.pos.y + character.height / 2};
}

Vector2 EnemyCenter(Enemy enemy)
{
	return (Vector2){enemy.pos.x + enemy.width / 2, enemy.pos.y + enemy.height / 2};
}