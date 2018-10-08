#include <vector>
#include <iostream>
#include <string>
#include "rsdl.h"
#include <sstream>
using namespace std;
#define TRANSFER -2
#define SUGAR -1
#define DOWN 0
#define LEFT 1
#define RIGHT 2
#define UP 3
#define UPRIGHT 4
#define UPLEFT 5

#define JUMPNORMAL 10
#define JUMPUP2DIRECTION 11
#define JUMPDIRECTION 12
#define JUMPUPDIRECTION 13
#define JUMPUP 14

#define SPEED 2
#define NUMOFQASHAPES 10
#define RECSIZE 40
#define DELAYOFMOVE 20
#define HITSTAR 100

#define MAKESUGAR 1
#define NORMAL 2
#define WON 1
#define LOST -1
#define GETALLSTARS 2

#define WINIMGWIDTH 200
#define WINIMGHEIGHT 80

struct QAPoint {
	int x;
	int y;
	int direction;
};
struct Item {
	int x;
	int y;
};
void resetSugarMap(vector<string>& sugarsMap) {
	for (int i = 0; i < sugarsMap.size(); ++i) {
		sugarsMap[i] = string(sugarsMap[0].size(), 'w');
	}
}
void updateSugarsMap(vector<string>& sugarsMap, vector<QAPoint>& qaPoint) {
	resetSugarMap(sugarsMap);
	for (int i = 0; i < qaPoint.size(); ++i) {
		sugarsMap[qaPoint[i].y /RECSIZE][qaPoint[i].x /RECSIZE] = 'b';
	}
}
void updateGameMap(vector<string>& gameMap, int x, int y) {
	for (int i = 0; i < gameMap.size(); ++i)
		for (int j = 0; j < gameMap[0].size(); ++j)
			if (gameMap[i][j] == 's') {
				gameMap[i][j] = 'w';
				gameMap[y /RECSIZE][x /RECSIZE] = 's';
				return;
			}
}
void draw(window& gameWindow, string shape, int x, int y) {
	if (shape == "WHITE")
		gameWindow.fill_rect(x, y, RECSIZE, RECSIZE, WHITE);
	else {
		gameWindow.draw_bmp("images/"+shape, x, y, RECSIZE, RECSIZE); // print qand asal
		gameWindow.update_screen();
	}
}
bool blockChars(char mapChar, bool droppedSugars) { // which characters are blocked
	if (droppedSugars) {
		if (mapChar != 'w' && mapChar != 'q' && mapChar != 'x')
			return true;
	} else if (mapChar != 'w' && mapChar != 's' && mapChar != 'x' && mapChar != 'e' && mapChar != 't')
		return true;
	return false;
}
int howqaHitsBlock(vector <string> gameMap, int direction, int x, int y, bool droppedSugars = false) {
	bool bottom2Direction =false, bottomDirection =false, topDirection =false, top2Direction =false, top =false;
	for (int i = 0; i < gameMap.size(); ++i)
		for (int j = 0; j < gameMap[0].size(); ++j) {
			if (blockChars(gameMap[i][j], droppedSugars)) {
				if (direction == UPRIGHT && x + 2*RECSIZE == j*RECSIZE || direction == UPLEFT && x - 2*RECSIZE == j*RECSIZE) {
					if (bottom2Direction == false && y == i*RECSIZE)
						bottom2Direction = true;
					if (top2Direction == false && y - RECSIZE == i*RECSIZE)
						top2Direction = true;
				}
				if ((direction == UPRIGHT || direction == RIGHT) && x+RECSIZE == j*RECSIZE || (direction == UPLEFT || direction == LEFT) && x-RECSIZE == j*RECSIZE) {
					if (bottomDirection == false &&	y == i*RECSIZE)
						bottomDirection = true;
					if (topDirection == false &&	y - RECSIZE == i*RECSIZE)
						topDirection = true;
				}
				if ((direction >= UP) && y - RECSIZE == i*RECSIZE && x == j*RECSIZE)
					top = true;
				if	(direction == DOWN && y + RECSIZE == i*RECSIZE && x == j*RECSIZE)
					return 0;
			}
		}
	if (direction == DOWN) return 1;
	if ((direction >= UP && top) || (direction < UP && bottomDirection)) return 0;
	if ((direction == UP && !top) || (direction < UP && !bottomDirection)) return 1;
	if (topDirection)
		return JUMPUP;
	if (top2Direction) {
		if (bottomDirection)
			return JUMPUPDIRECTION;
		return JUMPDIRECTION;
	}
	if (bottom2Direction) {
		if (bottomDirection)
			return JUMPUPDIRECTION;
		return JUMPUP2DIRECTION;
	}
	if (bottomDirection)
		return JUMPUPDIRECTION;
	return JUMPNORMAL;
}
void removeLastSugar(window& gameWindow, vector<string>& sugarsMap, vector<QAPoint>& qaPoint) {
	draw(gameWindow, "WHITE", qaPoint[qaPoint.size()-1].x, qaPoint[qaPoint.size()-1].y);
	gameWindow.update_screen();
	updateSugarsMap(sugarsMap, qaPoint);
	qaPoint.erase(qaPoint.begin()+(qaPoint.size()-1));
}
void removeAllSugars(window& gameWindow, vector<string> sugarsMap, vector<QAPoint>& qaPoint, bool forceAll = false) {
	while (qaPoint.size() > 1)
		removeLastSugar(gameWindow, sugarsMap, qaPoint);
	if (forceAll)
		removeLastSugar(gameWindow, sugarsMap, qaPoint);
}
void createSugar(window& gameWindow, vector<string> gameMap, vector<string>& sugarsMap, vector<QAPoint>& qaPoint, int currentDirection, int direction) {
	bool canCreate = howqaHitsBlock(gameMap, direction, qaPoint[qaPoint.size()-1].x, qaPoint[qaPoint.size()-1].y);
	if (canCreate && direction == DOWN && qaPoint[qaPoint.size()-1].x == qaPoint[0].x && qaPoint[qaPoint.size()-1].y == qaPoint[0].y) // can't create sugar bellow qa
		canCreate = false;
	if (canCreate)
		canCreate = howqaHitsBlock(sugarsMap, direction, qaPoint[qaPoint.size()-1].x, qaPoint[qaPoint.size()-1].y); // can't create sugar on itself
	if (canCreate) {
		qaPoint.push_back(QAPoint());
		int sugarSize = qaPoint.size();
		if (direction == RIGHT) {
			qaPoint[sugarSize-1].x = qaPoint[sugarSize-2].x + RECSIZE;
			qaPoint[sugarSize-1].y = qaPoint[sugarSize-2].y;
		} else if (direction == LEFT) {
			qaPoint[sugarSize-1].x = qaPoint[sugarSize-2].x - RECSIZE;
			qaPoint[sugarSize-1].y = qaPoint[sugarSize-2].y;
		} else if (direction == UP) {
			qaPoint[sugarSize-1].x = qaPoint[sugarSize-2].x;
			qaPoint[sugarSize-1].y = qaPoint[sugarSize-2].y - RECSIZE;
		} else if (direction == DOWN) {
			qaPoint[sugarSize-1].x = qaPoint[sugarSize-2].x;
			qaPoint[sugarSize-1].y = qaPoint[sugarSize-2].y + RECSIZE;
		}
		sugarsMap[ qaPoint[qaPoint.size()-1].y /RECSIZE ][ qaPoint[qaPoint.size()-1].x /RECSIZE ] = 'b'; // add the suger in the sugers' map
		qaPoint[qaPoint.size()-1].direction = direction;
		draw(gameWindow, "sugar/0.bmp", qaPoint[qaPoint.size()-1].x, qaPoint[qaPoint.size()-1].y);
	}
}
void setDroppedSugarsOnMap(vector<string>& gameMap, vector<QAPoint> droppedSugars, char onMap = 'q') {
	if (onMap == 'q')
		for (int i = 0; i < gameMap.size(); ++i)
			for (int j = 0; j < gameMap[0].size(); ++j)
				if (gameMap[i][j] == 'q')
					gameMap[i][j] = 'w';
	for (int i = 0; i < droppedSugars.size(); ++i)
		gameMap[droppedSugars[i].y /RECSIZE][droppedSugars[i].x /RECSIZE] = onMap;
}
void startCreatingSugar(window& gameWindow, vector<string> sugarsMap, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<QAPoint>& droppedSugars, int currentDirection) {
	removeAllSugars(gameWindow, sugarsMap, qaPoint);
	if (droppedSugars.size() != 0) {
		setDroppedSugarsOnMap(gameMap, droppedSugars, 'w'); // delete all dropped sugers
		removeAllSugars(gameWindow, sugarsMap, droppedSugars, true);
	}
	if (currentDirection == RIGHT)
		draw(gameWindow, "make_sugar/0.bmp", qaPoint[0].x, qaPoint[0].y);
	else
		draw(gameWindow, "make_sugar/1.bmp", qaPoint[0].x, qaPoint[0].y);
}
bool qaFalls(vector<string> gameMap, vector<QAPoint>& qaPoint, bool droppedSugars = false) {
	bool canMove = false;
	for (int i = 0; i < qaPoint.size(); ++i) {
		canMove = howqaHitsBlock(gameMap, DOWN, qaPoint[i].x, qaPoint[i].y, droppedSugars);
		if (!canMove)
			return true;
	}
	return false;
}
string getPath(int currentDirection, int loop, int direction = UP, bool lastShape = false) {
	if (lastShape) {
		if (currentDirection == SUGAR)
			return "sugar/1.bmp";
		if (loop==0)
			return ((currentDirection == RIGHT && direction == UP) || direction == UPRIGHT) ? "right/0.bmp" : "left/0.bmp";
		return "sugar/0.bmp";
	} else {
		if (currentDirection == SUGAR)
			return "sugar/1.bmp";
		if (loop==0)
			return ((currentDirection == RIGHT && direction == UP) || direction == UPRIGHT) ? "jump/0.bmp" : "jump/1.bmp";
		return "sugar/0.bmp";
	}
}
int checkEndPoint(window& gameWindow, vector<QAPoint>& qaPoint, int numOfStars, int* endPoint) {
	if (qaPoint[0].x == endPoint[0] && qaPoint[0].y == endPoint[1]) {
		if (numOfStars == 0)
			return WON;
		else
			return GETALLSTARS;
	}
	return 0;
}
int checkHitItem(window& gameWindow, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& items, bool isTrap = false) {
	// check if current home has star or trap
	for (int i = 0; i < items.size(); ++i)
		if (items[i].x == qaPoint[0].x && items[i].y == qaPoint[0].y) {
			if (isTrap)
				return LOST;
			gameMap[items[i].y /RECSIZE][items[i].x /RECSIZE] = 'w'; // delete the star from the map
			items.erase(items.begin()+i, items.begin()+i+1);
			draw(gameWindow, "crown/1.bmp", qaPoint[0].x, qaPoint[0].y); // show qa with star on it
			gameWindow.update_screen();
			DELAY(300);
		}
	gameMap[qaPoint[0].y /RECSIZE][qaPoint[0].x /RECSIZE] = 's';
	return 0;
}
void checkGravity(window& gameWindow, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int& winState, bool isSugar = false, int* endPoint=0) {
	bool dontMove;
	dontMove = qaFalls(gameMap, qaPoint, isSugar);
	string path;
	int speed = RECSIZE/NUMOFQASHAPES;
	while (!dontMove && winState != WON && winState != LOST) {
		for (int i = 0; i <= NUMOFQASHAPES; ++i) {
			DELAY(DELAYOFMOVE);
			for (int j = 0; j < qaPoint.size(); ++j) {
				if (i == NUMOFQASHAPES) { // draw the last shape
					if (isSugar)
						path = getPath(SUGAR, j, UP, true);
					else
						path = getPath(qaPoint[0].direction, j, UP, true);
					draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
					break;
				}
				draw(gameWindow, "WHITE", qaPoint[j].x, qaPoint[j].y);
				qaPoint[j].y += speed;
				if (isSugar)
					path = getPath(SUGAR, j);
				else
					path = getPath(qaPoint[0].direction, j);
				draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
			}
		}
		if (!isSugar) { // check if qa hits a star or trap or door
			winState = checkHitItem(gameWindow, gameMap, qaPoint, traps, true);
			checkHitItem(gameWindow, gameMap, qaPoint, stars);
			if (winState != WON && winState != LOST)
				winState = checkEndPoint(gameWindow, qaPoint, stars.size(), endPoint);
		}
		dontMove = qaFalls(gameMap, qaPoint, isSugar);
	}
	if (isSugar) // its a dropped sugar
		setDroppedSugarsOnMap(gameMap, qaPoint);
}
void descend(window& gameWindow, vector<QAPoint>& qaPoint, int currentDirection, int direction, int maxHowJump) {
	int speed = RECSIZE/NUMOFQASHAPES;
	string path;
	for (int i = 0; i <= NUMOFQASHAPES; ++i) {
		DELAY(DELAYOFMOVE);
		for (int j = 0; j < qaPoint.size(); ++j) {
			draw(gameWindow, "WHITE", qaPoint[j].x, qaPoint[j].y);
			if (i == NUMOFQASHAPES) { // draw the last shape
				path = getPath(currentDirection, j, direction, true);
				draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
				break;
			}
			path = getPath(currentDirection, j, direction);
			if (maxHowJump != JUMPUPDIRECTION && maxHowJump != JUMPUP2DIRECTION) // decide how qa should come down (descend)
				qaPoint[j].y += speed;
			if (maxHowJump == JUMPNORMAL || maxHowJump == JUMPUPDIRECTION || maxHowJump == JUMPUP2DIRECTION) // decide how qa should come down (descend)
				qaPoint[j].x = (direction == UPRIGHT) ? qaPoint[j].x + speed : qaPoint[j].x - speed;
			draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
		}
	}
}
void ascend(window& gameWindow, vector<QAPoint>& qaPoint, int currentDirection, int direction, int maxHowJump) {
	int speed = RECSIZE/NUMOFQASHAPES;
	string path;
	for (int i = 0; i < NUMOFQASHAPES; ++i) {
		DELAY(DELAYOFMOVE);
		for (int j = 0; j < qaPoint.size(); ++j) {
			draw(gameWindow, "WHITE", qaPoint[j].x, qaPoint[j].y);
			path = getPath(currentDirection, j, direction);
			qaPoint[j].y -= speed;
			if (maxHowJump == JUMPDIRECTION || maxHowJump == JUMPNORMAL || maxHowJump == JUMPUP2DIRECTION) // decide how qa should come down (descend)
				qaPoint[j].x = (direction == UPRIGHT) ? qaPoint[j].x + speed : qaPoint[j].x - speed;
			draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
		}
	}
}
void jump(window& gameWindow, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int currentDirection, int direction, int& winState, int* endPoint=0) {
	int howJump, maxHowJump=0;
	for (int i = 0; i < qaPoint.size(); ++i) {
		if (currentDirection == SUGAR)
			howJump = howqaHitsBlock(gameMap, direction, qaPoint[i].x, qaPoint[i].y, true); // if it's dropped sugar, blockChars function, chooses different characters for it
		else
			howJump = howqaHitsBlock(gameMap, direction, qaPoint[i].x, qaPoint[i].y);
		if (!howJump)
			return;
		if (maxHowJump < howJump) // maxHowJump decides how the qa can jump based on blocks near it
			maxHowJump = howJump;
	}
	ascend(gameWindow, qaPoint, currentDirection, direction, maxHowJump);
	if (currentDirection != SUGAR) { // check if qa hits a star or trap or door
		winState = checkHitItem(gameWindow, gameMap, qaPoint, traps, true);
		checkHitItem(gameWindow, gameMap, qaPoint, stars);
		if (winState != WON && winState != LOST)
			winState = checkEndPoint(gameWindow, qaPoint, stars.size(), endPoint);
	}
	if (winState != WON && winState != LOST) // if qa hit a trap, just stop the qa from moving
		descend(gameWindow, qaPoint, currentDirection, direction, maxHowJump);
}
void walk(window& gameWindow, vector<string>& gameMap, vector<QAPoint>& qaPoint, int direction) {
	int speed = RECSIZE/NUMOFQASHAPES;
	string path, nameOfPhoto;
	int canMove;
	for (int i = 0; i < qaPoint.size(); ++i) {
		canMove = howqaHitsBlock(gameMap, direction, qaPoint[i].x, qaPoint[i].y);
		if (!canMove)
			return;
	}
	for (int i = 1; i <= NUMOFQASHAPES+1; ++i) {
		DELAY(DELAYOFMOVE);
		for (int j = 0; j < qaPoint.size(); ++j) {
			draw(gameWindow, "WHITE", qaPoint[j].x, qaPoint[j].y);
			if (i > NUMOFQASHAPES) { // draw the last shape
				if (j == 0) {
					if (direction == RIGHT)
						draw(gameWindow, "right/0.bmp", qaPoint[j].x, qaPoint[j].y);
					else
						draw(gameWindow, "left/0.bmp", qaPoint[j].x, qaPoint[j].y);
				} else
					draw(gameWindow, "sugar/0.bmp", qaPoint[j].x, qaPoint[j].y);
				break;
			}
			if (direction == RIGHT) {
				qaPoint[j].x += speed;
				path = "right";
			} else {
				qaPoint[j].x -= speed;
				path = "left";
			}
			// converting i to string for image names
			stringstream cenvertStoI;
			cenvertStoI << i;
			nameOfPhoto = cenvertStoI.str();
			path += (string) "/" + nameOfPhoto + ".bmp";
			if (j!=0)
				path = "sugar/0.bmp";
			draw(gameWindow, path, qaPoint[j].x, qaPoint[j].y);
		}
	}
}
void drawQAinPosition(window& gameWindow, vector<QAPoint>& qaPoint) {
	if (qaPoint[0].direction == RIGHT)
		draw(gameWindow, "right/0.bmp", qaPoint[0].x, qaPoint[0].y);
	else
		draw(gameWindow, "left/0.bmp", qaPoint[0].x, qaPoint[0].y);
}
void transferToLastSugar(window& gameWindow, vector<string>& sugarsMap, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int* endPoint, int& currentState, int& winState) {
	int tempX, tempY, direction;
	tempX = qaPoint[qaPoint.size()-1].x;
	tempY = qaPoint[qaPoint.size()-1].y;
	removeAllSugars(gameWindow, sugarsMap, qaPoint);
	draw(gameWindow, "WHITE", qaPoint[0].x, qaPoint[0].y);
	qaPoint[0].x = tempX;
	qaPoint[0].y = tempY;
	drawQAinPosition(gameWindow, qaPoint);
	updateGameMap(gameMap, qaPoint[0].x, qaPoint[0].y);
}
void move(window& gameWindow, vector<string>& sugarsMap, vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int direction, int* endPoint, int& currentState, int& winState) {
	if (direction == DOWN)
		checkGravity(gameWindow, gameMap, qaPoint, stars, traps, winState, false, endPoint);
	else if (direction == RIGHT || direction == LEFT) {
		walk(gameWindow, gameMap, qaPoint, direction);
		qaPoint[0].direction = direction;
		currentState = NORMAL; // now user can't make any suger
	} else if (direction == TRANSFER) {
		transferToLastSugar(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, endPoint, currentState, winState);
		currentState = NORMAL; // now user can't make any suger
	} else {
		jump(gameWindow, gameMap, qaPoint, stars, traps, qaPoint[0].direction, direction, winState, endPoint);
		if (direction == UPRIGHT)
			qaPoint[0].direction = RIGHT;
		else if (direction == UPLEFT)
			qaPoint[0].direction = LEFT;
		currentState = NORMAL; // now user can't make any suger
	}
	checkHitItem(gameWindow, gameMap, qaPoint, stars);
	if (winState != WON && winState != LOST) {
		winState = checkHitItem(gameWindow, gameMap, qaPoint, traps, true);
		checkGravity(gameWindow, gameMap, qaPoint, stars, traps, winState, false, endPoint);
		checkHitItem(gameWindow, gameMap, qaPoint, stars);
	}
	if (winState != WON && winState != LOST)
		winState = checkHitItem(gameWindow, gameMap, qaPoint, traps, true);
	if (winState != WON && winState != LOST)
		winState = checkEndPoint(gameWindow, qaPoint, stars.size(), endPoint);
}
void colorizeDroppedSugars(window& gameWindow, vector<QAPoint>& droppedSugars) {
	for (int i = 0; i < droppedSugars.size(); ++i) {
		draw(gameWindow, "sugar/1.bmp", droppedSugars[i].x, droppedSugars[i].y);
	}
}
void dropSugars(window& gameWindow, vector<string>& gameMap, vector<string>& sugarsMap, vector<QAPoint>& qaPoint, vector<QAPoint>& droppedSugars, int direction, int& currentState) {
	int temp;
	vector<Item> tempS;
	droppedSugars = qaPoint;
	droppedSugars.erase(droppedSugars.begin(), droppedSugars.begin()+1);
	removeAllSugars(gameWindow, sugarsMap, qaPoint);
	colorizeDroppedSugars(gameWindow, droppedSugars);
	jump(gameWindow, gameMap, droppedSugars, tempS, tempS, SUGAR, direction, temp); // drop the dropped sugers
	checkGravity(gameWindow, gameMap, droppedSugars, tempS, tempS, temp, true); // for dropped sugers
	setDroppedSugarsOnMap(gameMap, droppedSugars);
	currentState = NORMAL; // now user can't make any suger
}
window drawMap(vector<string>& gameMap, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int* endPoint, int width, int height) {
	window gameWindow(width, height);
	for (int i = 0; i < gameMap.size(); ++i)
		for (int j = 0; j < gameMap[0].length(); ++j)
			switch (gameMap[i][j]) {
				case 'b':	gameWindow.fill_rect(j*RECSIZE, i*RECSIZE, RECSIZE, RECSIZE, DGRAY);	break;
				case 'w':	gameWindow.fill_rect(j*RECSIZE, i*RECSIZE, RECSIZE, RECSIZE, WHITE);	break;
				case 'e':	draw(gameWindow, "door/0.bmp", j*RECSIZE, i*RECSIZE);					break;
				case 's':	gameWindow.fill_rect(j*RECSIZE, i*RECSIZE, RECSIZE, RECSIZE, GREEN);	break;
				case 'x':
					draw(gameWindow, "crown/0.bmp", j*RECSIZE, i*RECSIZE);
					stars.push_back(Item());
					stars[stars.size()-1].x = j*RECSIZE;
					stars[stars.size()-1].y = i*RECSIZE;
				break;
				case 't':
					draw(gameWindow, "trap/0.bmp", j*RECSIZE, i*RECSIZE);
					traps.push_back(Item());
					traps[traps.size()-1].x = j*RECSIZE;
					traps[traps.size()-1].y = i*RECSIZE;
				break;
				default:	gameWindow.fill_rect(j*RECSIZE, i*RECSIZE, RECSIZE, RECSIZE, WHITE);	break;
			}
	draw(gameWindow, "right/0.bmp", qaPoint[0].x, qaPoint[0].y);
	return gameWindow;
}
void getMapInfo(string& worldName, int& sugarCubeLimit, int& rewardsCount, vector<QAPoint>& qaPoint, int* endPoint, vector<string>& gameMap, vector<string>& sugarsMap, int& width, int& height) {
	string row;
	cin >> worldName
		>> sugarCubeLimit
		>> rewardsCount
		>> qaPoint[0].x
		>> qaPoint[0].y
		>> endPoint[0]
		>> endPoint[1]
		>> width
		>> height;
	while (cin >> row) {
		gameMap.push_back(row);
		sugarsMap.push_back(string(row.size(), 'w'));
	}
	sugarsMap[qaPoint[0].y][qaPoint[0].x] = 'b';
	qaPoint[0].x *= RECSIZE;
	qaPoint[0].y *= RECSIZE;
	qaPoint[0].direction = RIGHT;
	endPoint[0] *= RECSIZE;
	endPoint[1] *= RECSIZE;
	width *= RECSIZE;
	height *= RECSIZE;
}
void endgame(window& gameWindow, int& winState, int width, int height, int capturedStars) {
	DELAY(500);
	if (winState == WON)
		gameWindow.draw_bmp("images/win.bmp", (width-WINIMGWIDTH)/2, (height-WINIMGHEIGHT)/2, WINIMGWIDTH, WINIMGHEIGHT);
	else
		gameWindow.draw_bmp("images/lose.bmp", (width-WINIMGWIDTH)/2, (height-WINIMGHEIGHT)/2, WINIMGWIDTH, WINIMGHEIGHT);
	cout << "You captured " << capturedStars << " star!" << endl;
	gameWindow.update_screen();
	DELAY(1000);
}
void drawtrapsStars(window& gameWindow, vector<QAPoint>& droppedSugars, vector<QAPoint>& qaPoint, vector<Item>& items, bool isTrap = false) {
	int dontDraw = 0;
	for (int i = 0; i < items.size(); ++i) { // for each item
		dontDraw =0;
		for (int j = 0; j < qaPoint.size(); ++j) // for each sugar
			if (qaPoint[j].x == items[i].x && qaPoint[j].y == items[i].y) // if a sugar is on the item
				dontDraw = 1; // don't draw item :)
		for (int k = 0; k < droppedSugars.size(); ++k) // for each dropped sugar
			if (droppedSugars[k].x == items[i].x && droppedSugars[k].y == items[i].y) // if a sugar is on the item
				dontDraw = 1; // don't draw item :)
		if (!dontDraw) {
			if (isTrap)
				draw(gameWindow, "trap/0.bmp", items[i].x, items[i].y);
			else
				draw(gameWindow, "crown/0.bmp", items[i].x, items[i].y);
		}
	}
}
void drawConstItem(window& gameWindow, vector<QAPoint>& droppedSugars, vector<QAPoint>& qaPoint, vector<Item>& stars, vector<Item>& traps, int* endPoint) {
	int dontDraw = 0;
	for (int j = 0; j < qaPoint.size(); ++j)
		if (qaPoint[j].x == endPoint[0] && qaPoint[j].y == endPoint[1]) // if there's no sugar on the door draw it
			dontDraw = 1;
	for (int k = 1; k < droppedSugars.size(); ++k)
		if (droppedSugars[k].x == endPoint[0] && droppedSugars[k].y == endPoint[1]) // if there's no dropped sugar on the door draw it
			dontDraw = 1;
	if (!dontDraw) {
		if (stars.size()== 0)
			draw(gameWindow, "door/1.bmp", endPoint[0], endPoint[1]);
		else
			draw(gameWindow, "door/0.bmp", endPoint[0], endPoint[1]);
	}
	drawtrapsStars(gameWindow, droppedSugars, qaPoint, stars);
	drawtrapsStars(gameWindow, droppedSugars, qaPoint, traps, true);
}
void decideDrawOrRemoveSugar(window& gameWindow, vector<string>& gameMap, vector<string>& sugarsMap, vector<QAPoint>& qaPoint, int sugarCubeLimit, int direction ,int currentState) {
	if (currentState != MAKESUGAR)
		return;
	// if we should erase the last sugar
	if( (qaPoint.size() != 1) && (
		(direction == DOWN && qaPoint[qaPoint.size()-1].direction == UP) ||
		(direction == UP && qaPoint[qaPoint.size()-1].direction == DOWN) ||
		(direction == RIGHT && qaPoint[qaPoint.size()-1].direction == LEFT) ||
		(direction == LEFT && qaPoint[qaPoint.size()-1].direction == RIGHT)
		)
	)
		removeLastSugar(gameWindow, sugarsMap, qaPoint);
	else if (sugarCubeLimit > qaPoint.size()-1)
		createSugar(gameWindow, gameMap, sugarsMap, qaPoint, qaPoint[0].direction, direction);
}
void runGame(window& gameWindow, vector<string>& gameMap, vector<string>& sugarsMap, vector<QAPoint>& qaPoint, vector<QAPoint>& droppedSugars, vector<Item>& stars, vector<Item>& tempS, vector<Item>& traps, int& sugarCubeLimit, int& rewardsCount, int& currentState, int& winState, int& temp, int* endPoint) {
	bool quit = false;
	while (!quit) {
		HANDLE (
			QUIT (quit = true);
			KEY_PRESS(RIGHT,	{move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, RIGHT,		endPoint, currentState, winState); break;});
			KEY_PRESS(LEFT,		{move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, LEFT,		endPoint, currentState, winState); break;});
			KEY_PRESS(UP,		{move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, UP,		endPoint, currentState, winState); break;});
			KEY_PRESS(e,		{move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, UPRIGHT,	endPoint, currentState, winState); break;});
			KEY_PRESS(q,		{move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, UPLEFT,	endPoint, currentState, winState); break;});
			KEY_PRESS(LCTRL,	{startCreatingSugar(gameWindow, sugarsMap, gameMap, qaPoint, droppedSugars, qaPoint[0].direction);
				currentState = MAKESUGAR; break;});
			KEY_PRESS(w,		{decideDrawOrRemoveSugar(gameWindow, gameMap, sugarsMap, qaPoint, sugarCubeLimit, UP, currentState);	break;});
			KEY_PRESS(s,		{decideDrawOrRemoveSugar(gameWindow, gameMap, sugarsMap, qaPoint, sugarCubeLimit, DOWN, currentState);	break;});
			KEY_PRESS(d,		{decideDrawOrRemoveSugar(gameWindow, gameMap, sugarsMap, qaPoint, sugarCubeLimit, RIGHT, currentState);	break;});
			KEY_PRESS(a,		{decideDrawOrRemoveSugar(gameWindow, gameMap, sugarsMap, qaPoint, sugarCubeLimit, LEFT, currentState);	break;});
			KEY_PRESS(c,		{if (qaPoint.size() > 1) dropSugars(gameWindow, gameMap, sugarsMap, qaPoint, droppedSugars, UPRIGHT, currentState);					break;});
			KEY_PRESS(z,		{if (qaPoint.size() > 1) dropSugars(gameWindow, gameMap, sugarsMap, qaPoint, droppedSugars, UPLEFT, currentState);					break;});
			KEY_PRESS(SPACE,	{if (qaPoint.size() > 1) {
				move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, TRANSFER, endPoint, currentState, winState);
				move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, DOWN, endPoint, currentState, winState);
			}	break;});
		);
		if (winState != WON && winState != LOST)
			move(gameWindow, sugarsMap, gameMap, qaPoint, stars, traps, DOWN, endPoint, currentState, winState);
		if (droppedSugars.size() != 0)
			checkGravity(gameWindow, gameMap, droppedSugars, tempS, tempS, temp, true);
		updateSugarsMap(sugarsMap, qaPoint);
		updateGameMap(gameMap, qaPoint[0].x, qaPoint[0].y);
		drawConstItem(gameWindow, droppedSugars, qaPoint, stars, traps, endPoint);
		if (winState == WON || winState == LOST) {
			draw(gameWindow, "door/0.bmp", endPoint[0], endPoint[1]);
			quit = true;
		}
	}
}
int main () {
	int sugarCubeLimit, rewardsCount, currentState = NORMAL, winState = 0, temp, endPoint[2], width, height;
	string worldName;
	vector<string> gameMap, sugarsMap;
	vector<QAPoint> qaPoint, droppedSugars;
	vector<Item> stars, tempS, traps;

	qaPoint.push_back(QAPoint());
	getMapInfo(worldName, sugarCubeLimit, rewardsCount, qaPoint, endPoint, gameMap, sugarsMap, width, height);
	window gameWindow = drawMap(gameMap, qaPoint, stars, traps, endPoint, width, height);

	runGame(gameWindow, gameMap, sugarsMap, qaPoint, droppedSugars, stars, tempS, traps, sugarCubeLimit, rewardsCount, currentState, winState, temp, endPoint);
	endgame(gameWindow, winState, width, height, rewardsCount-stars.size());
	return 0;
}