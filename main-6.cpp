#define _USE_MATH_DEFINES
#include<math.h>
#include<stdio.h>
#include<string.h>
#include "draw_elements.h"
#include <cstdlib>
#include <ctime>
#include <time.h>
#include <Windows.h>
#include <fileapi.h>

extern "C" {
#include"./SDL2-2.0.10/include/SDL.h"
#include"./SDL2-2.0.10/include/SDL_main.h"
}

#define SCREEN_WIDTH 640
#define SCREEN_HEIGHT 480
#define START_X	320
#define START_Y 300
#define ESCAPE_KEY SDLK_ESCAPE
#define KEY_G 'g'
#define KEY_N 'n'
#define KEY_R 'r'
#define KEY_P 'p'
#define KEY_L 'l'
#define KEY_S 's'
#define MAX_SPRITE	5
#define ROAD_RIGHT_BORDER 480
#define ROAD_LEFT_BORDER 160
#define FLAG_UP 1
#define FLAG_DOWN 0
#define BULLET_SPEED 3
#define FILEID "GRA"
#define IDLEN 3
#define MAX_FILES 10
#define SPEED_INC 0.016
#define SPEED_NORM 0.02
#define ADD_POINTS 150
#define SIZE_SPRITE_X 30
#define SIZE_SPRITE_Y 40
#define RESET_VALUE 600
#define START_TREE -100

void ShowGameOptions();
void CreateDisplay();
int Game();
int ShowMainMenu();
void DestroyDisplay();
void ClearDisplay();
void DrawMap(int color1, int color2, int color3);
int ShowRankingView();
int CheckIfOutOfRoad();
void ShowEnemies();
int CheckCollision();
void SetStartValues();
void ToggleMultipleButtons();
void ToggleBullet();
int SaveGame(const char* plik);
int LoadGame(const char* plik);
int getDir();
void SaveCurrState();
int CkeckBulletCollision();
void CheckEndGame();

	int t1, t2, quit, x, right, left, up, init, pause;
	double delta, worldTime, score, speed, curTime;
	int czarny, zielony, czerwony, niebieski, szary, zolty;
	char* fileTab[MAX_FILES];
	SDL_Event event, menuEvents;
	SDL_Surface *screen, *charset;
	SDL_Surface *heroCar;
	SDL_Texture *scrtex;
	SDL_Window *window;
	SDL_Renderer *renderer;

	struct {
		int y;
		int x;
		int lives;
	}sprite;

	typedef struct {
		int y;
		int x;
		SDL_Surface* obj;
	}spriteEnemy;

	spriteEnemy spriteTable[MAX_SPRITE];
	
	typedef struct {
		int x;
		int y;
		SDL_Surface* obj;
	}tree;
	tree treeObj;
	tree treeObj1;
	typedef struct {
		int x;
		int y;
		bool active;
		SDL_Surface* bullet;
	}bullet;
	
	bullet bulletTable;

#ifdef __cplusplus
extern "C"
#endif
int main(int argc, char **argv) {
	getDir();
	srand(time(NULL)); 
	char zn;
	CreateDisplay();
	do {
		zn = Game();
	} while (zn!=ESCAPE_KEY);
	DestroyDisplay();
	return 0;
	};

//glowna rozgrywka
int Game() {

	ClearDisplay(); 
	int cmd = ShowMainMenu();
	if (cmd == ESCAPE_KEY) return ESCAPE_KEY;
	if (cmd == KEY_G) SetStartValues();//ustawienie odpowiednich wartosci poczatkowych

	while (!quit) {
		t2 = SDL_GetTicks();
		delta = (t2 - t1) * 0.001;
		t1 = t2;
		if (!pause) {
			worldTime += delta;
			//sprawdzanie czy samochod nie wyjechal poza obszar dozwolony/zmniejszanie punktow/////
			if (CheckIfOutOfRoad()) score += delta * 10;
			//rysowanie elementow 
			DrawMap(zielony, szary, zolty);
			//mozliwosc uzywania kilku przyciskow na raz
			ToggleMultipleButtons();
			//rysowanie wrogow po 2 sekundach od rozpoczêcia gry
			if (worldTime > 2) ShowEnemies();
			ToggleBullet();
			if (bulletTable.active == true) DrawSurface(screen, bulletTable.bullet, bulletTable.x, bulletTable.y);
			//rysowanie samochodu
			DrawSurface(screen, heroCar, sprite.x, sprite.y);
			if (CheckCollision()) sprite.lives -= 1;
			CheckEndGame();//sprawdzanie konca gry
			ShowGameOptions();//wyswiatlanie elementow opisu rozgrywki
			
			if (CkeckBulletCollision()) score += ADD_POINTS;
		}
		if (SDL_PollEvent(&event)) {
			switch (event.type) {
			case SDL_KEYDOWN:
				if (event.key.keysym.sym == SDLK_ESCAPE) return ESCAPE_KEY;
				else if (event.key.keysym.sym == KEY_N) {//blad
					pause = 0;
					return KEY_N;
				}
				else if (event.key.keysym.sym == KEY_P) pause = pause ? FLAG_DOWN : FLAG_UP;
				else if (event.key.keysym.sym == SDLK_UP) up = FLAG_UP;
				else if (event.key.keysym.sym == SDLK_RIGHT) {
					x = FLAG_UP;
					init = SDL_GetTicks();
					right = FLAG_UP;
				}
				else if (event.key.keysym.sym == SDLK_LEFT) {
					x = -FLAG_UP;
					init = SDL_GetTicks();
					left = FLAG_UP;
				}
				else if (event.key.keysym.sym == SDLK_SPACE) {
					bulletTable.active = true;
					bulletTable.x = sprite.x;
					bulletTable.y = START_Y;
				}
				else if (event.key.keysym.sym == KEY_S) {
					time_t rawtime;
					struct tm* timeinfo;
					char filename[80];

					time(&rawtime);
					timeinfo = localtime(&rawtime);

					strftime(filename, 80, "%Y-%m-%d-%H%M%S.sav", timeinfo);
					int files = getDir();
					if (files >= MAX_FILES) {
						DeleteFileA(fileTab[0]);
					}
					SaveGame(filename);
				}
				break; 
			case SDL_KEYUP:
				if (event.key.keysym.sym == SDLK_RIGHT) right = FLAG_DOWN;
				else if (event.key.keysym.sym == SDLK_LEFT) left = FLAG_DOWN;
				else if (event.key.keysym.sym == SDLK_UP) {
					up = FLAG_DOWN;
					speed = SPEED_NORM;
				}
				break;
			case SDL_QUIT: //wylaczenie gry guzikiem x
				quit = FLAG_DOWN;
				return ESCAPE_KEY;
				break;
			};
		};
		SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
		SDL_RenderCopy(renderer, scrtex, NULL, NULL); 
		SDL_RenderPresent(renderer);
	};
}

//rysowanie mozliwych opcji wyboru w menu
void ShowGameOptions() {
		char text[128];
		DrawRectangle(screen, 0, 0, SCREEN_WIDTH - 8, 50, czerwony, niebieski);

		sprintf(text, "TIMELEFT = %.1lf s SCORE = %.0f points lives=%d", worldTime, score, sprite.lives);
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 10, text, charset);

		sprintf(text, "arrows: left=move left, right=move right, up=speed up");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 20, text, charset);

		sprintf(text, "n=new game, esc=quit, p=pause/unpause, shoot=space, save game=s");
		DrawString(screen, screen->w / 2 - strlen(text) * 8 / 2, 30, text, charset);
}

//usuwanie elementow bibloteki z okna gry
void DestroyDisplay() {
	// zwolnienie powierzchni 
	SDL_FreeSurface(charset);
	SDL_FreeSurface(screen);
	SDL_FreeSurface(heroCar);
	for (int i = 0; i < MAX_SPRITE; i++) {
		SDL_FreeSurface(spriteTable[i].obj);
	}
	SDL_FreeSurface(bulletTable.bullet);
	SDL_DestroyTexture(scrtex);
	SDL_DestroyRenderer(renderer);
	SDL_DestroyWindow(window);

	SDL_Quit();
}

//tworzenie elementow biblioteki
void CreateDisplay() {
		
		SDL_Init(SDL_INIT_EVERYTHING);
		int rc = SDL_CreateWindowAndRenderer(SCREEN_WIDTH, SCREEN_HEIGHT, 0,
			&window, &renderer);

		SDL_SetHint(SDL_HINT_RENDER_SCALE_QUALITY, "linear");
		SDL_RenderSetLogicalSize(renderer, SCREEN_WIDTH, SCREEN_HEIGHT);
		SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);

		SDL_SetWindowTitle(window, "Mateusz Michalski, 193355");


		screen = SDL_CreateRGBSurface(0, SCREEN_WIDTH, SCREEN_HEIGHT, 32,
			0x00FF0000, 0x0000FF00, 0x000000FF, 0xFF000000);

		scrtex = SDL_CreateTexture(renderer, SDL_PIXELFORMAT_ARGB8888,
			SDL_TEXTUREACCESS_STREAMING,
			SCREEN_WIDTH, SCREEN_HEIGHT);

		SDL_ShowCursor(SDL_DISABLE);

		charset = SDL_LoadBMP("./cs8x8.bmp");
		SDL_SetColorKey(charset, true, 0x000000);

		heroCar = SDL_LoadBMP("./hero_car.bmp");
		SDL_SetColorKey(heroCar, true, 0x000000);

		for (int i = 0; i < MAX_SPRITE; i++) {
			spriteTable[i].obj = SDL_LoadBMP("./enemy_car.bmp");
			SDL_SetColorKey(spriteTable[i].obj, true, 0x000000);

			spriteTable[i].x = rand()%35+ROAD_LEFT_BORDER+10+(i*65);
		}

		bulletTable.bullet = SDL_LoadBMP("./bullet.bmp");

		treeObj.obj = SDL_LoadBMP("./tree.bmp");
		SDL_SetColorKey(treeObj.obj, true, 0x000000);
		treeObj1.obj = SDL_LoadBMP("./tree.bmp");
		SDL_SetColorKey(treeObj1.obj, true, 0x000000);

		czarny = SDL_MapRGB(screen->format, 0x00, 0x00, 0x00);
		zielony = SDL_MapRGB(screen->format, 0x00, 0x64, 0x00);
		czerwony = SDL_MapRGB(screen->format, 0xFF, 0x00, 0x00);
		niebieski = SDL_MapRGB(screen->format, 0x11, 0x11, 0xCC);
		szary = SDL_MapRGB(screen->format, 0x80, 0x80, 0x80);
		zolty = SDL_MapRGB(screen->format, 255, 255, 30);
		
}

//tworzenie okna glownego menu
int ShowMainMenu() {
	char menuElements[128];
	sprintf(menuElements, "WELCOME TO SPY HUNTER");
	DrawString(screen, screen->w / 2 - strlen(menuElements) * 8 / 2, 40, menuElements, charset);
	sprintf(menuElements, "Press g to play");
	DrawString(screen, screen->w / 2 - strlen(menuElements) * 8 / 2, 60, menuElements, charset);
	sprintf(menuElements, "Press esc to quit");
	DrawString(screen, screen->w / 2 - strlen(menuElements) * 8 / 2, 70, menuElements, charset);
	sprintf(menuElements, "LOAD ALREADY PLAYED GAMES");
	DrawString(screen, screen->w / 2 - strlen(menuElements) * 8 / 2, START_X-20, menuElements, charset);
	if (getDir()) {
		int i = 0;
		while (fileTab[i] && i<10) {
			sprintf(menuElements, "%d - %s", i, fileTab[i]);
			DrawString(screen, screen->w / 2 - strlen(menuElements) * 8 / 2, START_X + (i * 10), menuElements, charset);
			i++;
		}
	}
	SDL_UpdateTexture(scrtex, NULL, screen->pixels, screen->pitch);
	SDL_RenderClear(renderer);
	SDL_RenderCopy(renderer, scrtex, NULL, NULL);
	SDL_RenderPresent(renderer);
	//tutaj petla while do przyciskow
	while (true) {
		SDL_PollEvent(&event);
		switch (event.type) {
		case SDL_KEYDOWN:
			if (event.key.keysym.sym == SDLK_ESCAPE) return ESCAPE_KEY;
			else if (event.key.keysym.sym == KEY_G) return KEY_G;
			else if (event.key.keysym.sym >= '0' && event.key.keysym.sym <= '9') {
				int key = event.key.keysym.sym - '0';
				if (fileTab[key]) {
					LoadGame(fileTab[key]);
					return KEY_L;
				}
			}
			break;
		case SDL_QUIT:
			return ESCAPE_KEY;
			break;
		};
	}

	return 0;
}

//przywracanie ekranu do czarnego tla
void ClearDisplay() {
	SDL_FillRect(screen, NULL, czarny);
}

//rysowanie mapy rozgrywki
void DrawMap(int color1, int color2, int color3) {
	DrawRectangle(screen, 0, 0, SCREEN_WIDTH / 4, SCREEN_HEIGHT, color1, color1);
	DrawRectangle(screen, ROAD_LEFT_BORDER, 0, SCREEN_WIDTH / 2, SCREEN_HEIGHT, color2, color2);
	DrawRectangle(screen, ROAD_RIGHT_BORDER, 0, SCREEN_WIDTH / 4, SCREEN_HEIGHT, color1, color1);
	DrawRectangle(screen, ROAD_RIGHT_BORDER - 10, 0, SCREEN_WIDTH / 4 - 145, SCREEN_HEIGHT, color3, color3);
	DrawRectangle(screen, ROAD_LEFT_BORDER - 10, 0, SCREEN_WIDTH / 4 - 145, SCREEN_HEIGHT, color3, color3);
	DrawSurface(screen, treeObj.obj, treeObj.x, treeObj.y++);
	DrawSurface(screen, treeObj1.obj, treeObj1.x, treeObj1.y++);
	if (treeObj.y > SCREEN_HEIGHT) {
		treeObj.x = rand() % 160 + 490;
		treeObj.y = START_TREE;
	}
	if (treeObj1.y > SCREEN_HEIGHT) {
		treeObj1.x = rand() % 140 + 10;
		treeObj1.y = START_TREE;
	}
	char points[128];
	sprintf(points, "a,b,c,d,e,f,g,i,j,k");
	DrawString(screen, SCREEN_WIDTH-154, SCREEN_HEIGHT-20, points, charset);
}

//tworzenie przeciwnikow
void ShowEnemies() { 
	int counterR = 0;
	int counterL = 0;
	for (int i = 0; i < MAX_SPRITE; i++) { //maxsprite
		if (spriteTable[i].obj) {
			DrawSurface(screen, spriteTable[i].obj, spriteTable[i].x, spriteTable[i].y);
		}
	}
	if ((worldTime - curTime) >= speed) {
		curTime = worldTime;
		for (int i = 0; i < MAX_SPRITE; i++) {

			if (spriteTable[i].obj) {
				if (i % 2 == 1) {
					spriteTable[i].y -= rand()%3+i;
				}
				else {
					spriteTable[i].y += rand()%3+i+1;
				}
				if (spriteTable[i].y < -RESET_VALUE) {
					spriteTable[i].y = SCREEN_HEIGHT+RESET_VALUE;
					counterR = rand() % 3;
					if (counterR) {
						spriteTable[i].x += 3;
					}
					else {
						spriteTable[i].x -= 6;
					}
				}
				else if (spriteTable[i].y > SCREEN_HEIGHT+RESET_VALUE) {
					spriteTable[i].y = -RESET_VALUE;
					counterL = rand() % 3;
					if (counterL) {
						spriteTable[i].x -= 3;
					}
					else {
						spriteTable[i].x += 6;
					}
				}
			}
			else {
				spriteTable[i].obj = SDL_LoadBMP("./enemy_car.bmp");
				SDL_SetColorKey(spriteTable[i].obj, true, 0x000000); //zdejmowanie t³a z obrazu
				spriteTable[i].y = -RESET_VALUE;
			}
		}
	}
}

//sprawdzanie wyjechania poza obszar drogi
int CheckIfOutOfRoad() {
	if (sprite.x > ROAD_RIGHT_BORDER-10 || sprite.x < ROAD_LEFT_BORDER+10) {	
		sprite.x = START_X;
		for (int i = 0; i < MAX_SPRITE; i++) {
			spriteTable[i].y = -200;
		}
		sprite.lives--;
	}
	if ((sprite.x > ROAD_RIGHT_BORDER - 25 && sprite.x < ROAD_RIGHT_BORDER - 10) || sprite.x > ROAD_LEFT_BORDER + 10 && sprite.x < ROAD_LEFT_BORDER + 20) {
		for (int j = 0; j < MAX_SPRITE; j++) {
			if (j % 2 == 1) {
				spriteTable[j].y += 2;
			}
			else {
				speed = SPEED_INC;
			}
		}
		return 0;
	}
	else {
		speed = SPEED_NORM;
		return 1;
	}
}

//sprawdzanie kolizji gracza z samochodami
int CheckCollision() { 
	for (int i = 0; i < MAX_SPRITE; i++) {
		if (spriteTable[i].obj) {
			if (
				spriteTable[i].x < sprite.x + SIZE_SPRITE_X &&
				spriteTable[i].x + SIZE_SPRITE_X > sprite.x &&
				spriteTable[i].y < sprite.y + SIZE_SPRITE_Y &&
				spriteTable[i].y + SIZE_SPRITE_Y > sprite.y
				)
			{
				SDL_FreeSurface(spriteTable[i].obj);
				spriteTable[i].obj = NULL;
				sprite.x = START_X;
				for (int j = 0; j < MAX_SPRITE; j++) {
					spriteTable[j].y = -RESET_VALUE;
				}
				return 1;
			}
		}
	}
		return 0;
}

//ustawienie wartosci poczatkowych
void SetStartValues() {
	t1 = SDL_GetTicks();
	quit = 0;
	worldTime = 0;
	sprite.x = START_X;
	sprite.y = START_Y;
	curTime = worldTime;
	speed = SPEED_NORM;
	score = 0;
	sprite.lives = 5;
	left = 0;
	right = 0;
	up = 0;
	pause = 0;
	for (int i = 0; i < MAX_SPRITE; i++) {
		spriteTable[i].y = -RESET_VALUE;
	}
	bulletTable.active = false;
	init = SDL_GetTicks();
	treeObj.y = 0;
	treeObj.x = 520;
	treeObj1.y = 100;
}

//mozliwosc uzywania kilku przyciskow na raz
void ToggleMultipleButtons(){
	if (x == FLAG_UP && right == FLAG_UP) {
		sprite.x += (SDL_GetTicks() - init) / 5; 
		init = SDL_GetTicks();
	}
	else if (x == -FLAG_UP && left == FLAG_UP) {
		sprite.x -= (SDL_GetTicks() - init) / 5;
		init = SDL_GetTicks();
	}
	if (up == FLAG_UP && (sprite.x>ROAD_LEFT_BORDER+20 && sprite.x<ROAD_RIGHT_BORDER-50)) {
		speed = SPEED_INC;
		score += delta * 2 * 10;
		treeObj.y += 1;
		treeObj1.y += 1;
	}
}

//ustawianie pozycji kuli
void ToggleBullet() {
			if (bulletTable.y >= 175) {
				bulletTable.y -= BULLET_SPEED;
			}
			else {
				bulletTable.active = false;
			}
}

//zapisywanie wszystkie elementy gry
int SaveGame(const char* plik) {
		FILE* f = fopen(plik, "w+"); 
		if (f) {
			fwrite(FILEID, strlen(FILEID), 1, f);		
			fwrite(&score, sizeof(double), 1, f);
			fwrite(&worldTime, sizeof(double), 1, f);
			fwrite(&curTime, sizeof(double), 1, f);
			fwrite(&sprite, sizeof(sprite), 1, f);

			for (int i = 0; i < MAX_SPRITE; i++) {
				fwrite(&spriteTable[i].x, sizeof(int), 1, f);	
				fwrite(&spriteTable[i].y, sizeof(int), 1, f);	
			}
			fclose(f);
			return 1;
		}
	return 0;														
}

//ladowanie wartosci z plikow 
int LoadGame(const char* plik) {
		FILE* f = fopen(plik, "r");
		if (f) {
			char fileid[IDLEN + 1];

			fread(fileid, strlen(FILEID), 1, f);				
			if (memcmp(FILEID, fileid, strlen(FILEID)) == 0) {	

					fread(&score, sizeof(double), 1, f);
					fread(&worldTime, sizeof(double), 1, f);
					fread(&curTime, sizeof(double), 1, f);
					fread(&sprite, sizeof(sprite), 1, f); 

					for (int i = 0; i < MAX_SPRITE; i++) {
						fread(&spriteTable[i].x, sizeof(int), 1, f);
						fread(&spriteTable[i].y, sizeof(int), 1, f);
					}
					fclose(f);
					return 1;
			}
			fclose(f);
		}
	return 0;
}

//wywolywanie z folderu plikow z .sav gry
int getDir() {
	HANDLE hFind;
	WIN32_FIND_DATA FindFileData;
	int i = 0;

	memset(fileTab, 0, sizeof(fileTab));
	if ((hFind = FindFirstFile(L"*", &FindFileData)) != INVALID_HANDLE_VALUE) {
		do {
			if (wcsstr(FindFileData.cFileName, L".sav")) { //zwracany wskaznik do pierwszego pliku wyszukanego
				char mb[260];
				sprintf(mb, "%ls", FindFileData.cFileName);
				fileTab[i] = strdup(mb); //zwraca lancuch znaków
				i++;
			}
		} while (FindNextFile(hFind, &FindFileData) && i < MAX_FILES);
		FindClose(hFind);
	}
	return i;
}

//sprawdzanie kolizji kuli z innymi samochodami
int CkeckBulletCollision() {
	for (int i = 0; i < MAX_SPRITE; i++) {
		if (spriteTable[i].obj && bulletTable.active == true) {
			if (
				spriteTable[i].x < bulletTable.x + (SIZE_SPRITE_X/3) &&
				spriteTable[i].x + SIZE_SPRITE_X > bulletTable.x &&
				spriteTable[i].y < bulletTable.y + (SIZE_SPRITE_Y/4) &&
				spriteTable[i].y + SIZE_SPRITE_Y > bulletTable.y
				)
			{
				SDL_FreeSurface(spriteTable[i].obj); //zwalnianie obiektu, brak renderowania
				spriteTable[i].obj = NULL;
				bulletTable.active = false;
				return 1;
			}
		}
	}
	return 0;
}

//sprawdzenie konca rozgrywki przez skonczenie zyc
void CheckEndGame() {
	if (sprite.lives == 0) {//sprawdzanie konca gry
		char gameOverText[128];
		sprintf(gameOverText, "GAME OVER");
		DrawString(screen, screen->w / 2 - strlen(gameOverText) * 8 / 2, SCREEN_HEIGHT / 3, gameOverText, charset);
		sprintf(gameOverText, "press n to go back to menu");
		DrawString(screen, screen->w / 2 - strlen(gameOverText) * 8 / 2, SCREEN_HEIGHT / 3 + 15, gameOverText, charset);
		pause = 1;
	}
}
