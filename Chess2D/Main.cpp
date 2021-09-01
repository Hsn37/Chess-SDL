#include <iostream>
#include "SDL.h"			//Main SDL Library
#include "SDL_image.h"		//To display images
#include <string.h>
#include <vector>
#undef main

using namespace std;

const int SQUARE_SIDE = 80;

class Image
{
public:
	SDL_Texture* img;
	SDL_Rect rect;

	Image(){}

	void LoadImage(string path, SDL_Renderer* ren)
	{
		SDL_Surface* surface = IMG_Load(path.c_str());
		img = SDL_CreateTextureFromSurface(ren, surface);
		SDL_FreeSurface(surface);
	}
};

class Piece
{
protected:
	double Gradient(int x1, int y1)
	{
		double a = y1 - y;
		double b = x1 - x;

		// return int max if gradient infinite
		if (b == 0)
			return INT_MAX;
		return a / b;
	}

	double Distance(int x1, int y1)
	{
		double a = pow(y1 - y, 2);
		double b = pow(x1 - x, 2);
		double c = a + b;

		return pow(c, 0.5);
	}

	bool CheckPathEmpty(int x1, int y1, bool diagonal, vector<Piece*> pcs)
	{
		int a = x1 - x;
		int b = y1 - y;

		int startX = x;
		int startY = y;

		int xOffset, yOffset;

		if (!diagonal)
		{
			if (a == 0 && b > 0)
			{
				xOffset = 0;
				yOffset = SQUARE_SIDE;
			}
			else if (a > 0 && b == 0)
			{
				xOffset = SQUARE_SIDE;
				yOffset = 0;
			}
			else if (a < 0 && b == 0)
			{
				xOffset = -SQUARE_SIDE;
				yOffset = 0;
			}
			else if (a == 0 && b < 0)
			{
				xOffset = 0;
				yOffset = -SQUARE_SIDE;
			}
		}
		else
		{
			if (a > 0 && b > 0)
			{
				xOffset = SQUARE_SIDE;
				yOffset = SQUARE_SIDE;
			}
			else if (a > 0 && b < 0)
			{
				xOffset = SQUARE_SIDE;
				yOffset = -SQUARE_SIDE;
			}
			else if (a < 0 && b < 0)
			{
				xOffset = -SQUARE_SIDE;
				yOffset = -SQUARE_SIDE;
			}
			else if (a < 0 && b > 0)
			{
				xOffset = -SQUARE_SIDE;
				yOffset = SQUARE_SIDE;
			}
		}

		bool wait = false;
		while (startX != x1 || startY != y1)
		{
			startX += xOffset;
			startY += yOffset;

			if (startX == x1 && startY == y1)
				return true;

			for (int i = 0; i < pcs.size(); i++)
				if (!pcs[i]->captured && startX == pcs[i]->x && startY == pcs[i]->y)
					return false;
		}
	}

public:
	int x;
	int y;
	int owner;	// player that owns this piece. 0 or 1. 0 = player 1. 1 = player 2
	Image sourceImg;
	bool captured = false;

	Piece(string path, SDL_Renderer* ren, int own, int x, int y)
	{
		this->x = x;
		this->y = y;
		owner = own;

		sourceImg.LoadImage(path, ren);
		sourceImg.rect.x = x;
		sourceImg.rect.y = y;
		sourceImg.rect.w = SQUARE_SIDE;
		sourceImg.rect.h = SQUARE_SIDE;
	}

	virtual bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		return false;
	}
	
	void DisplayPiece(SDL_Renderer* ren)
	{
		if (!captured)
			SDL_RenderCopy(ren, sourceImg.img, NULL, &sourceImg.rect);
	}

	void movePiece(int x, int y)
	{
		this->x = x;
		this->y = y;

		sourceImg.rect.x = x;
		sourceImg.rect.y = y;
	}
};

class King : public Piece
{
public:
	King(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y){}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		double g = Gradient(x1, y1);
		double d = Distance(x1, y1);
		double diagonalDistance = Distance(x + 80, y + 80);

		if (g == 0 || g == 1 || g == -1 || g == (double)INT_MAX)
			if (d == SQUARE_SIDE || d == diagonalDistance)
				return true;

		return false;
	}
};

class Queen : public Piece
{
public:
	Queen(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y) {}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		double g = Gradient(x1, y1);
		if (g == 0 || g == INT_MAX)
		{
			return CheckPathEmpty(x1, y1, false, pcs);
		}
		else if (g == 1 || g == -1)
		{
			return CheckPathEmpty(x1, y1, true, pcs);
		}

		return false;
	}
};

class Bishop : public Piece
{
public:
	Bishop(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y) {}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		double g = Gradient(x1, y1);
		if (g == 1 || g == -1)
		{
			return CheckPathEmpty(x1, y1, true, pcs);
		}

		return false;
	}
};

class Knight : public Piece
{
public:
	Knight(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y) {}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		double g = Gradient(x1, y1);
		double d = Distance(x + 2 * SQUARE_SIDE, y + SQUARE_SIDE);

		if (g == 2 || g == 0.5 || g == -2 || g == -0.5)
			if (Distance(x1, y1) == d)
				return true;
			
		return false;
	}
};

class Pawn : public Piece
{
private:
	bool firstMove = true;
public:
	Pawn(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y) {}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		if (owner == 0)
		{
			if (newP)
			{
				if (newP->y == y - SQUARE_SIDE && newP->x == x + SQUARE_SIDE)
					return true;
				else if (newP->y == y - SQUARE_SIDE && newP->x == x - SQUARE_SIDE)
					return true;

			}
			else
			{
				if (firstMove)
				{
					if (x1 == x && (y1 == y - SQUARE_SIDE || y1 == y - 2 * SQUARE_SIDE))
					{
						firstMove = false;
						return true;
					}
				}
				else
					if (x1 == x && y1 == y - SQUARE_SIDE)
						return true;
			}
		}
		else
		{
			if (newP)
			{
				if (newP->y = y + SQUARE_SIDE && newP->x == x + SQUARE_SIDE)
					return true;
				else if (newP->y = y + SQUARE_SIDE && newP->x == x - SQUARE_SIDE)
					return true;

			}
			else
			{
				if (firstMove)
				{
					if (x1 == x && (y1 == y + SQUARE_SIDE || y1 == y + 2 * SQUARE_SIDE))
					{
						firstMove = false;
						return true;
					}
				}
				else
					if (x1 == x && y1 == y + SQUARE_SIDE)
						return true;
			}
		}


		return false;
	}
};

class Rook: public Piece
{
public:
	Rook(string path, SDL_Renderer* ren, int owner, int x, int y) : Piece(path, ren, owner, x, y) {}

	bool isMoveValid(Piece* newP, vector<Piece*> pcs, int x1, int y1)
	{
		double g = Gradient(x1, y1);
		if (g == 0 || g == INT_MAX)
		{
			return CheckPathEmpty(x1, y1, false, pcs);
		}

		return false;
	}
};

class GameBoard
{
private:
	int width, height;
	Image overlayImg;

	void DisplayBoard(SDL_Renderer* ren)
	{
		SDL_RenderCopy(ren, bg.img, NULL, &bg.rect);
	}

public:
	Image bg;
	vector<Piece*> pieces;

	GameBoard(int w, int h)
	{
		width = w;
		height = h;
	}

	void LoadGameBoard(SDL_Renderer* ren)
	{
		bg.LoadImage("Assets/Board.png", ren);
		bg.rect.h = height;
		bg.rect.w = width;
		bg.rect.x = 0;
		bg.rect.y = 0;


		overlayImg.LoadImage("Assets/Overlay.png", ren);
		overlayImg.rect.w = SQUARE_SIDE;
		overlayImg.rect.h = SQUARE_SIDE;
	}

	void LoadPieces(SDL_Renderer* ren)
	{
		// Black/player 2 pieces
		pieces.push_back(new Rook("Assets/BlackRook.png", ren, 1, 0, 0));
		pieces.push_back(new Knight("Assets/BlackKnight.png", ren, 1, 80, 0));
		pieces.push_back(new Bishop("Assets/BlackBishop.png", ren, 1, 160, 0));
		pieces.push_back(new Queen("Assets/BlackQueen.png", ren, 1, 240, 0));
		pieces.push_back(new King("Assets/BlackKing.png", ren, 1, 320, 0));
		pieces.push_back(new Bishop("Assets/BlackBishop.png", ren, 1, 400, 0));
		pieces.push_back(new Knight("Assets/BlackKnight.png", ren, 1, 480, 0));
		pieces.push_back(new Rook("Assets/BlackRook.png", ren, 1, 560, 0));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 0, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 80, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 160, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 240, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 320, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 400, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 480, 80));
		pieces.push_back(new Pawn("Assets/BlackPawn.png", ren, 1, 560, 80));

		// White/player 1 pieces
		pieces.push_back(new Rook("Assets/WhiteRook.png", ren, 0, 0, 560));
		pieces.push_back(new Knight("Assets/WhiteKnight.png", ren, 0, 80, 560));
		pieces.push_back(new Bishop("Assets/WhiteBishop.png", ren, 0, 160, 560)); 
		pieces.push_back(new Queen("Assets/WhiteQueen.png", ren, 0, 240, 560));
		pieces.push_back(new King("Assets/WhiteKing.png", ren, 0, 320, 560));
		pieces.push_back(new Bishop("Assets/WhiteBishop.png", ren, 0, 400, 560));
		pieces.push_back(new Knight("Assets/WhiteKnight.png", ren, 0, 480, 560));
		pieces.push_back(new Rook("Assets/WhiteRook.png", ren, 0, 560, 560));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 0, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 80, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 160, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 240, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 320, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 400, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 480, 480));
		pieces.push_back(new Pawn("Assets/WhitePawn.png", ren, 0, 560, 480));
		
	}

	void RenderBoard(SDL_Renderer* ren)
	{
		DisplayBoard(ren);
		for (int i = 0; i < pieces.size(); i++)
			pieces[i]->DisplayPiece(ren);	
	}

	void HighlightSelectedPiece(SDL_Renderer* ren, int x, int y)
	{
		overlayImg.rect.x = x;
		overlayImg.rect.y = y;

		SDL_RenderCopy(ren, overlayImg.img, NULL, &overlayImg.rect);
	}

	~GameBoard()
	{
		for (int i = 0; i < pieces.size(); i++)
			delete pieces[i];
	}
};

class MultiPlayerGame
{
private:
	SDL_Renderer* ren;
	GameBoard* board;
	SDL_Window* window;
	bool running = true;
	int turn = 0;

	void IdentifyBox(int& x, int& y)
	{
		x = x - x % SQUARE_SIDE;
		y = y - y % SQUARE_SIDE;

		//cout << "Clicked: (" << x << ", " << y << ")\n";
	}

	Piece* IdentifyPiece(int x, int y)
	{
		for (int i = 0; i < board->pieces.size(); i++)
			if (board->pieces[i]->x == x && board->pieces[i]->y == y && !board->pieces[i]->captured)
				return board->pieces[i];
		
		return NULL;
	}

	void ChangeTurn()
	{
		turn = turn ? 0 : 1;
		cout << "Turn: " << (turn ? "P2" : "P1") << endl;
	}

	Piece* HandleMove(Piece* newP, Piece* currP, int x, int y)
	{
		// first part of the turn
		if (currP == NULL)
		{
			if (newP && newP->owner == turn)
			{
				return newP;
			}
			else
			{
				return NULL;
			}
		}
		// second part of the turn, as there is already a piece selected
		else
		{
			// selected a square with a piece
			if (newP)
			{
				// uncheck the current piece
				if (newP == currP)
					return NULL;
				// chose one of his own pieces, do nothing
				else if (newP->owner == currP->owner)
				{
					ShowInvalidMoveDialog();
					return currP;
				}
				// chose enemy, capture the enemy
				else if (currP->isMoveValid(newP, board->pieces, x, y))
				{
					newP->captured = true;
					currP->movePiece(x, y);
					ChangeTurn();

					return NULL;
				}
				else
					return currP;
			}
			// chose empty square
			else
			{
				if (currP->isMoveValid(newP, board->pieces, x, y))
				{
					currP->movePiece(x, y);
					ChangeTurn();

					return NULL;
				}
				else
					return currP;
			}
		}
	}

	Piece* HandleGameEvents(Piece* currentPiece)
	{
		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				SDL_DestroyWindow(window);
				running = false;
			}

			else if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
			{
				int x = event.button.x;
				int y = event.button.y;

				IdentifyBox(x, y);
				Piece* newPiece = IdentifyPiece(x, y);

				currentPiece = HandleMove(newPiece, currentPiece, x, y);
			}
		}

		return currentPiece;
	}

	void ShowInvalidMoveDialog()
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR, "Error", "Invalid Move", window);
	}

	void ShowGameWonDialog(string t)
	{
		SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "Game Over", t.c_str(), window);
	}
	
	void CheckGameWon()
	{
		for (int i = 0; i < board->pieces.size(); i++)
		{
			if (dynamic_cast<King*>(board->pieces[i]) && dynamic_cast<King*>(board->pieces[i])->captured)
			{
				if (dynamic_cast<King*>(board->pieces[i])->owner == 0)
					ShowGameWonDialog("Player 2 won");
				else
					ShowGameWonDialog("Player 1 won");

				running = false;
			}
		}
	}

public:
	MultiPlayerGame(SDL_Renderer* r, GameBoard* b, SDL_Window* w)
	{
		ren = r;
		board = b;
		window = w;
	}

	void StartGameLoop()
	{
		cout << "CHESS 2D\n";
		cout << "White = Player 1\n";
		cout << "Black = Player 2\n";
		cout << "Turn: " << (turn ? "P2" : "P1") << endl;

		Piece* currentPiece = NULL;
		while (running)
		{
			currentPiece = HandleGameEvents(currentPiece);

			// rendering part
			SDL_RenderClear(ren);
			board->RenderBoard(ren);
			if (currentPiece)
				board->HighlightSelectedPiece(ren, currentPiece->x, currentPiece->y);
			SDL_RenderPresent(ren);

			CheckGameWon();
		}
	}
};

class IntroScreen
{
private:
	Image bg;
	Image b1;
	Image b2;
	SDL_Window* window;
	
	int HandleEvents()
	{
		// return 1 for multiplayer
		// return 2 for single player
		// return 0 for no action
		// return -1 to exit

		SDL_Event event;
		while (SDL_PollEvent(&event))
		{
			if (event.type == SDL_QUIT)
			{
				SDL_DestroyWindow(window);
				return -1;
			}

			if (event.type == SDL_MOUSEBUTTONDOWN && event.button.button == SDL_BUTTON_LEFT)
			{
				if (event.button.x > b2.rect.x && event.button.x < b2.rect.x + b2.rect.w)
					if (event.button.y > b2.rect.y && event.button.y < b2.rect.y + b2.rect.h)
						return 1;
			}
		}

		return 0;
	}

public:

	IntroScreen(SDL_Renderer* ren, SDL_Window* w, string bgpath, string b1path, string b2path)
	{
		window = w;

		bg.LoadImage(bgpath, ren);
		b1.LoadImage(b1path, ren);
		b2.LoadImage(b2path, ren);

		bg.rect.x = 0;
		bg.rect.y = 0;
		bg.rect.w = 640;
		bg.rect.h = 640;

		b1.rect.x = 199;
		b1.rect.y = 330;
		b1.rect.w = 247;
		b1.rect.h = 50;

		b2.rect.x = 171;
		b2.rect.y = 397;
		b2.rect.w = 308;
		b2.rect.h = 49;
	}

	int ShowIntro(SDL_Renderer* ren)
	{
		while (true)
		{
			SDL_RenderClear(ren);
			SDL_RenderCopy(ren, bg.img, NULL, &bg.rect);
			SDL_RenderCopy(ren, b1.img, NULL, &b1.rect);
			SDL_RenderCopy(ren, b2.img, NULL, &b2.rect);
			SDL_RenderPresent(ren);

			int t = HandleEvents();
			if (t != 0)
				return t;
		}
	}
};

class Game
{
public:
	SDL_Window* window;
	SDL_Renderer* ren;
	GameBoard* board;

	Game(string title, int w, int h)
	{
		SDL_Init(SDL_INIT_EVERYTHING);
		window = SDL_CreateWindow(title.c_str(), SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, w, h, false);
		ren = SDL_CreateRenderer(window, -1, 0);
		SDL_SetRenderDrawBlendMode(ren, SDL_BLENDMODE_BLEND);

		board = new GameBoard(640, 640);
		board->LoadGameBoard(ren);
		board->LoadPieces(ren);

		IntroScreen intro(ren, window, "Assets/Intro.jpg", "Assets/ButtonSingle.png", "Assets/ButtonMulti.png");
		int c = intro.ShowIntro(ren);

		if (c == 1)
		{
			MultiPlayerGame g(ren, board, window);
			g.StartGameLoop();
		}
	}

	~Game()
	{
		SDL_DestroyRenderer(ren);
		SDL_Quit();
		delete board;
	}
};

int main()
{	
	Game newGame("Chess2D", 640, 640);

	return 0;
}