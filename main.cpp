#include "SFML/Graphics.hpp"
#include <vector>
#include <cstdlib>
#include <algorithm>
#include <queue>
#include <cmath>

class ColorGrid {
private:
	const int gridSize;
	const int squareSize;
	const int windowSize;
	std::vector<sf::Color> colors;
	std::vector<std::vector<sf::RectangleShape>> grid;
	sf::Vector2i selectedPos = { -1, -1 };
	bool processingMatches = false;
	float fallAnimationTime = 0.0f;
	const float fallSpeed = 500.0f;

	struct Bonus {
		sf::Vector2i position;
		sf::Color color;
		std::unique_ptr<sf::CircleShape> shape;
		bool active = false;
		float lifetime = 2.0f;
	};

	std::vector<Bonus> bonuses;

public:
	ColorGrid(int size = 8, int cellSize = 80)
		: gridSize(size), squareSize(cellSize), windowSize(size* cellSize) {

		colors = {
			sf::Color::Red,
			sf::Color::Green,
			sf::Color::Blue,
			sf::Color::Yellow,
			sf::Color::Magenta,
			sf::Color::Cyan,
		};
		grid.resize(gridSize, std::vector<sf::RectangleShape>(gridSize));
		initializeGrid();
	}

	void initializeGrid() {
		for (int i = 0; i < gridSize; ++i) {
			for (int j = 0; j < gridSize; ++j) {
				initSquare(i, j);
			}
		}
		while (findAndRemoveMatches()) {
			collapseColumns();
			refillEmptySpaces();

		}
	}

	void initSquare(int i, int j, bool randomColor = true) {
		grid[i][j].setSize(sf::Vector2f(squareSize - 2, squareSize - 2));
		grid[i][j].setPosition(i * squareSize + 1, j * squareSize + 1);
		if (randomColor) {
			grid[i][j].setFillColor(colors[rand() % colors.size()]);
		}
		grid[i][j].setOutlineThickness(1);
		grid[i][j].setOutlineColor(sf::Color::Black);
	}

	void handleEvent(const sf::Event& event) {
		if (processingMatches) return;

		if (event.type == sf::Event::MouseButtonPressed &&
			event.mouseButton.button == sf::Mouse::Left) {

			int mouseX = event.mouseButton.x;
			int mouseY = event.mouseButton.y;

			int gridX = mouseX / squareSize;
			int gridY = mouseY / squareSize;

			if (gridX >= 0 && gridX < gridSize && gridY >= 0 && gridY < gridSize) {
				if (selectedPos.x == -1) {
					selectedPos = { gridX, gridY };
					grid[gridX][gridY].setOutlineColor(sf::Color::White);
					grid[gridX][gridY].setOutlineThickness(3);
				}
				else {
					if (areNeighbors(selectedPos, { gridX, gridY })) {
						swapSquares(selectedPos, { gridX, gridY });
						processMatches();
					}

					grid[selectedPos.x][selectedPos.y].setOutlineColor(sf::Color::Black);
					grid[selectedPos.x][selectedPos.y].setOutlineThickness(1);
					selectedPos = { -1, -1 };
				}
			}
		}
	}

	bool areNeighbors(const sf::Vector2i& pos1, const sf::Vector2i& pos2) {
		int dx = abs(pos1.x - pos2.x);
		int dy = abs(pos1.y - pos2.y);
		return (dx == 1 && dy == 0) || (dx == 0 && dy == 1);
	}

	void swapSquares(const sf::Vector2i& pos1, const sf::Vector2i& pos2) {
		sf::Color temp = grid[pos1.x][pos1.y].getFillColor();
		grid[pos1.x][pos1.y].setFillColor(grid[pos2.x][pos2.y].getFillColor());
		grid[pos2.x][pos2.y].setFillColor(temp);
	}

	void processMatches() {
		processingMatches = true;
		bool matchesFound = findAndRemoveMatches();

		if (matchesFound) {
			collapseColumns();
			refillEmptySpaces();
			processMatches();
		}
		else {
			processingMatches = false;
		}
	}


	bool findAndRemoveMatches() {
		std::vector<std::vector<bool>> toRemove(gridSize, std::vector<bool>(gridSize, false));
		bool foundMatches = false;

		for (int y = 0; y < gridSize; ++y) {
			for (int x = 0; x < gridSize - 2; ++x) {
				sf::Color color = grid[x][y].getFillColor();
				if (grid[x + 1][y].getFillColor() == color &&
					grid[x + 2][y].getFillColor() == color) {

					int length = 3;
					while (x + length < gridSize && grid[x + length][y].getFillColor() == color) {
						length++;
					}

					for (int i = 0; i < length; ++i) {
						toRemove[x + i][y] = true;
						if (rand() % 100 < 0 && length >= 3) {
							createBonus(x + i, y, color);
						}
					}
					foundMatches = true;
					x += length - 1;
				}
			}
		}

		for (int x = 0; x < gridSize; ++x) {
			for (int y = 0; y < gridSize - 2; ++y) {
				sf::Color color = grid[x][y].getFillColor();
				if (grid[x][y + 1].getFillColor() == color &&
					grid[x][y + 2].getFillColor() == color) {

					int length = 3;
					while (y + length < gridSize && grid[x][y + length].getFillColor() == color) {
						length++;
					}

					for (int i = 0; i < length; ++i) {
						toRemove[x][y + i] = true;
						if (rand() % 100 < 30 && length >= 3) {
							createBonus(x, y + i, color);
						}
					}
					foundMatches = true;
					y += length - 1;
				}
			}
		}

		if (foundMatches) {
			for (int x = 0; x < gridSize; ++x) {
				for (int y = 0; y < gridSize; ++y) {
					if (toRemove[x][y]) {
						grid[x][y].setFillColor(sf::Color::Transparent);
					}
				}
			}
		}

		return foundMatches;
	}

	void collapseColumns() {
		for (int x = 0; x < gridSize; ++x) {
			int emptyY = gridSize - 1;

			for (int y = gridSize - 1; y >= 0; --y) {
				if (grid[x][y].getFillColor() != sf::Color::Transparent) {
					if (y != emptyY) {
						grid[x][emptyY].setFillColor(grid[x][y].getFillColor());
						grid[x][y].setFillColor(sf::Color::Transparent);
					}
					emptyY--;
				}
			}
		}
	}

	void refillEmptySpaces() {
		for (int x = 0; x < gridSize; ++x) {
			for (int y = 0; y < gridSize; ++y) {
				if (grid[x][y].getFillColor() == sf::Color::Transparent) {
					grid[x][y].setFillColor(colors[rand() % colors.size()]);
				}
			}
		}
	}


	void createBonus(int x, int y, sf::Color color) {
		Bonus bonus;
		bonus.position = sf::Vector2i(x, y);
		bonus.color = color;
		bonus.shape = std::make_unique<sf::CircleShape>();
		bonus.shape->setRadius(squareSize / 4);
		bonus.shape->setFillColor(sf::Color(255, 255, 255, 200));
		bonus.shape->setOutlineColor(color);
		bonus.shape->setOutlineThickness(3);
		bonus.shape->setOrigin(squareSize / 4, squareSize / 4);
		bonus.shape->setPosition(x * squareSize + squareSize / 2, y * squareSize + squareSize / 2);
		bonus.active = true;
		bonuses.push_back(std::move(bonus));
	}

	void updateBonuses(float deltaTime) {
		for (auto& bonus : bonuses) {
			if (!bonus.active) continue;

			bonus.lifetime -= deltaTime;
			if (bonus.lifetime < 0.5f) {
				float alpha = 200 * (bonus.lifetime / 0.5f);
				bonus.shape->setFillColor(sf::Color(255, 255, 255, static_cast<sf::Uint8>(alpha)));
			}

			if (bonus.lifetime <= 0) {
				bonus.active = false;
				activateBonus(bonus);

				for (int i = 0; i < 10; ++i) {
					createParticleEffect(bonus.position.x, bonus.position.y, bonus.color);
				}
			}
		}

		bonuses.erase(std::remove_if(bonuses.begin(), bonuses.end(),
			[](const Bonus& b) { return !b.active; }), bonuses.end());
	}

	struct Particle {
		sf::CircleShape shape;
		sf::Vector2f velocity;
		float lifetime;
	};
	std::vector<Particle> particles;

	void createParticleEffect(int x, int y, sf::Color color) {
		Particle p;
		p.shape.setRadius(3 + rand() % 5);
		p.shape.setFillColor(color);
		p.shape.setPosition(
			x * squareSize + squareSize / 2 + (rand() % 20 - 10),
			y * squareSize + squareSize / 2 + (rand() % 20 - 10)
		);
		p.velocity = sf::Vector2f(
			(rand() % 100 - 50) * 2.0f,
			(rand() % 100 - 50) * 2.0f
		);
		p.lifetime = 0.3f + (rand() % 100) * 0.005f;
		particles.push_back(p);
	}

	void updateParticles(float deltaTime) {
		for (auto& p : particles) {
			p.lifetime -= deltaTime;
			p.shape.move(p.velocity * deltaTime);
			p.velocity *= 0.65f;
			float alpha = 255 * (p.lifetime / 0.8f);
			p.shape.setFillColor(sf::Color(
				p.shape.getFillColor().r,
				p.shape.getFillColor().g,
				p.shape.getFillColor().b,
				static_cast<sf::Uint8>(alpha)
			));
		}

		particles.erase(std::remove_if(particles.begin(), particles.end(),
			[](const Particle& p) { return p.lifetime <= 0; }), particles.end());
	}


	void activateBonus(const Bonus& bonus) {
		int centerX = bonus.position.x;
		int centerY = bonus.position.y;

		std::vector<sf::Vector2i> candidates;
		for (int x = std::max(0, centerX - 3); x <= std::min(gridSize - 1, centerX + 3); ++x) {
			for (int y = std::max(0, centerY - 3); y <= std::min(gridSize - 1, centerY + 3); ++y) {
				if (abs(x - centerX) + abs(y - centerY) > 1) {
					candidates.emplace_back(x, y);
				}
			}
		}

		std::random_shuffle(candidates.begin(), candidates.end());

		if (!candidates.empty()) {
			int targetX = candidates[0].x;
			int targetY = candidates[0].y;
			grid[targetX][targetY].setFillColor(bonus.color);
			for (int i = 1; i < std::min(3, (int)candidates.size()); ++i) {
				int x = candidates[i].x;
				int y = candidates[i].y;
				grid[x][y].setFillColor(bonus.color);
			}
		}

		processMatches();
	}


	void update(float deltaTime) {
		if (processingMatches) {
			fallAnimationTime += deltaTime;
			if (fallAnimationTime >= 0.1f) {
				fallAnimationTime = 0.0f;
				processingMatches = false;
			}
		}

		updateBonuses(deltaTime);
		updateParticles(deltaTime);
	}

	void draw(sf::RenderWindow& window) {
		for (const auto& row : grid) {
			for (const auto& square : row) {
				window.draw(square);
			}
		}

		for (const auto& p : particles) {
			window.draw(p.shape);
		}

		for (const auto& bonus : bonuses) {
			if (bonus.active) {
				window.draw(*bonus.shape);
				sf::CircleShape glow(bonus.shape->getRadius() * 1.5f);
				glow.setOrigin(glow.getRadius(), glow.getRadius());
				glow.setPosition(bonus.shape->getPosition());
				glow.setFillColor(sf::Color(bonus.color.r, bonus.color.g, bonus.color.b, 50));
				window.draw(glow);
			}
		}
	}
	int getWindowSize() const {
		return windowSize;
	}
};

int main() {
	ColorGrid grid;
	sf::RenderWindow window(sf::VideoMode(grid.getWindowSize(), grid.getWindowSize()), "Color Match Game with Bonuses");
	sf::Event event;
	sf::Clock clock;

	while (window.isOpen()) {
		float deltaTime = clock.restart().asSeconds();

		while (window.pollEvent(event)) {
			if (event.type == sf::Event::Closed) {
				window.close();
			}
			grid.handleEvent(event);
		}

		grid.update(deltaTime);
		window.clear(sf::Color(50, 50, 50));
		grid.draw(window);
		window.display();
	}

	return 0;
}