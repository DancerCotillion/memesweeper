#pragma once
#include "Vei2.h"
#include "Graphics.h"
#include "Sound.h"

class Minefield
{
public:
	enum class State
	{
		BlownUp,
		Won,
		Mining
	};
private:
	class Tile
	{
	public:
		enum class State
		{
			Hidden,
			Flagged,
			Revealed
		};
	public: 
		void SpawnMine(); 
		bool HasMine()const;
		void Draw(const Vei2& ScreepPos, Minefield::State gameState, Graphics& gfx) const;
		void Reveal();
		bool IsRevealed() const;
		void ToggleFlag();
		bool IsFlagged() const;

		void SetNeighbourMineCount(int mineCount);
	private:
		State state = State::Hidden;
		bool hasMine = false; 

		int nNeighbourBombs = -1;
	};

public:
	Minefield(const Vei2 centre, int nMines);
	void Draw(Graphics& gfx) const;
	RectI GetRect() const; 
	void onRevealClick(const Vei2 screenPos); 
	void onFlagClick(const Vei2 screenPos);
	State GetState() const; 

private:
	static constexpr int width = 20;
	static constexpr int height = 16; 
	static constexpr int borderThickness = 10;
	static constexpr Color borderColor = Colors::Blue; 

	Sound sndLose = Sound(L"..\\Sounds\\spayed.wav"); 

	Vei2 topLeft; 

	Tile field[width * height]; 

	Tile& TileAt(const Vei2& gridPos); 
	const Tile& TileAt(const Vei2& gridPos) const;

	Vei2 ScreenToGrid(const Vei2& screenPos); 

	int CountNeighbourMines(const Vei2& gridPos);

	State gameState = State::Mining; 
	bool Minefield::GameIsWon() const;
};