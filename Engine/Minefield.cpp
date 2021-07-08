#include "Minefield.h"
#include "SpriteCodex.h"
#include <assert.h>
#include <random>

void Minefield::Tile::SpawnMine()
{
	assert(!hasMine);
	hasMine = true;
}

bool Minefield::Tile::HasMine() const
{
	return hasMine;
}

void Minefield::Tile::Draw(const Vei2& screenPos, Minefield::State gameState, Graphics& gfx) const
{
	if (gameState != Minefield::State::BlownUp)
	{
		switch (state)
		{
		case State::Hidden:
			SpriteCodex::DrawTileButton(screenPos, gfx);
			break;
		case State::Flagged:
			SpriteCodex::DrawTileButton(screenPos, gfx);
			SpriteCodex::DrawTileFlag(screenPos, gfx);
			break;
		case State::Revealed:
			if (!HasMine())
			{
				SpriteCodex::DrawTileNumber(screenPos, nNeighbourBombs, gfx);
			}
			else
			{
				SpriteCodex::DrawTileBomb(screenPos, gfx);
			}
			break;
		}
	}
	else
	{
		switch (state)
		{
		case State::Hidden:
			if (HasMine())
			{
				SpriteCodex::DrawTileBomb(screenPos, gfx);
			}
			else
			{
				SpriteCodex::DrawTileButton(screenPos, gfx);
			}
			break;
		case State::Flagged:
			if (HasMine())
			{
				SpriteCodex::DrawTileBomb(screenPos, gfx);
				SpriteCodex::DrawTileFlag(screenPos, gfx);
			}
			else
			{
				SpriteCodex::DrawTileBomb(screenPos, gfx);
				SpriteCodex::DrawTileCross(screenPos, gfx);
			}
			break;
		case State::Revealed:
			if (!HasMine())
			{
				SpriteCodex::DrawTileNumber(screenPos, nNeighbourBombs, gfx);
			}
			else
			{
				SpriteCodex::DrawTileBombRed(screenPos, gfx);
			}
			break;
		}
	}
}

void Minefield::Tile::Reveal()
{
	assert(state == State::Hidden);
	state = State::Revealed; 
}

bool Minefield::Tile::IsRevealed() const
{
	return state == State::Revealed;
}

void Minefield::Tile::ToggleFlag()
{
	assert(!IsRevealed());
	if (state == State::Hidden)
	{
		state = State::Flagged; 
	}
	else
	{
		state = State::Hidden;
	}
}

bool Minefield::Tile::IsFlagged() const
{
	return state == State::Flagged;
}

void Minefield::Tile::SetNeighbourMineCount(int mineCount)
{
	assert(nNeighbourBombs == -1);
	nNeighbourBombs = mineCount;
}

Minefield::Minefield(const Vei2 centre, int nMines)
	:
	topLeft(centre - Vei2(width * SpriteCodex::tileSize, height * SpriteCodex::tileSize) / 2)
{
	assert(nMines > 0 && nMines < width* height);

	std::random_device rd;
	std::mt19937 rng(rd());
	std::uniform_int_distribution<int> xDist(0, width - 1);
	std::uniform_int_distribution<int> yDist(0, height - 1);

	for (int nSpawned = 0; nSpawned < nMines; nSpawned++)
	{
		Vei2 spawnPos;
		do
		{
			spawnPos = { xDist(rng), yDist(rng) }; 
		} 
		while (TileAt(spawnPos).HasMine());

		TileAt(spawnPos).SpawnMine(); 
	}
	for (Vei2 gridPos = { 0,0 }; gridPos.y < height; gridPos.y++)
	{
		for (gridPos.x = 0; gridPos.x < width; gridPos.x++)
		{
			TileAt(gridPos).SetNeighbourMineCount(CountNeighbourMines(gridPos));
		}
	}
}

void Minefield::Draw(Graphics& gfx) const
{
	gfx.DrawRect(GetRect().GetExpanded(borderThickness), borderColor); 
	gfx.DrawRect(GetRect(), SpriteCodex::baseColor); 

	for (Vei2 gridPos = { 0,0 }; gridPos.y < height; gridPos.y++)
	{
		for (gridPos.x = 0; gridPos.x < width; gridPos.x++)
		{
			TileAt(gridPos).Draw(topLeft + gridPos * SpriteCodex::tileSize, gameState, gfx); 
		}
	}
}

RectI Minefield::GetRect() const
{
	return RectI(topLeft, width * SpriteCodex::tileSize, height * SpriteCodex::tileSize);
}

void Minefield::onRevealClick(const Vei2 screenPos)
{
	if (gameState == State::Mining)
	{

		const Vei2 gridPos = ScreenToGrid(screenPos);
		Tile& tile = TileAt(gridPos);
		if (!tile.IsRevealed() && !tile.IsFlagged())
		{
			tile.Reveal();
			if (tile.HasMine())
			{
				gameState = State::BlownUp;
				sndLose.Play(); 
			}
			else if (GameIsWon())
			{
				gameState = State::Won;
			}
		}
	}
}

void Minefield::onFlagClick(const Vei2 screenPos)
{
	if (gameState == State::Mining)
	{
		const Vei2 gridPos = ScreenToGrid(screenPos);
		Tile& tile = TileAt(gridPos);
		if (!tile.IsRevealed())
		{
			tile.ToggleFlag();
		}
	}
}

Minefield::Tile& Minefield::TileAt(const Vei2& gridPos)
{
	return field[gridPos.y * width + gridPos.x];
}

const Minefield::Tile& Minefield::TileAt(const Vei2& gridPos) const
{
	return field[gridPos.y * width + gridPos.x];
}

Vei2 Minefield::ScreenToGrid(const Vei2& screenPos)
{
	return (screenPos - topLeft) / SpriteCodex::tileSize;
}

int Minefield::CountNeighbourMines(const Vei2& gridPos)
{
	const int xStart = std::max(0, gridPos.x - 1); 
	const int yStart = std::max(0, gridPos.y - 1); 
	const int xEnd = std::min(width - 1, gridPos.x + 1);
	const int yEnd = std::min(height - 1, gridPos.y + 1); 

	int count = 0;
	for (Vei2 gridPos = { xStart,yStart }; gridPos.y <= yEnd; gridPos.y++)
	{
		for (gridPos.x = xStart; gridPos.x <= xEnd; gridPos.x++)
		{
			if (TileAt(gridPos).HasMine())
			{
				count++;
			}
		}
	}
	return count; 
}

bool Minefield::GameIsWon() const
{
	for (const Tile& t : field)
	{
		if ((t.HasMine() && !t.IsFlagged()) ||
			(!t.HasMine() && !t.IsRevealed()))
		{
			return false;
		}
	}
	return true;
}

Minefield::State Minefield::GetState() const
{
	return gameState;
}

