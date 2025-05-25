#include "SFML/Graphics.hpp"
#include <SFML/Window.hpp>
#include <vector>


using namespace std;
using namespace sf;

constexpr int windowWidth{ 1600 }, windowHeight{ 1200 };
constexpr float ballRadius{ 20.f };
float paddleWidth{ 150.f }; constexpr float paddleHeight{ 25.f }, paddleVelocity{ 11.f };
constexpr float brickWidth{ 150.f }, brickHeight{ 30.f };
constexpr int countBricksX{ 8 }, countBricksY{ 8 };
constexpr float brickPaddingX{ 25.f }, brickPaddingY{ 20.f };
static int counter;

class Ball;
class Brick;
class Paddle;
class BlueBrick;

vector<Ball> extraBalls;
vector<unique_ptr<Paddle>> paddleList;

class Ball {
public:
	float ballVelocity{ 6.f };
	Vector2f velocity{ -ballVelocity, -ballVelocity };
protected:
	bool isBoosted{ false };
	Clock boostClock;

private:
	void resolveCollision(const FloatRect& brickBounds);
public:

	CircleShape shape;
	Ball(float startX, float startY) {
		shape.setPosition(startX, startY);
		shape.setRadius(ballRadius);
		shape.setFillColor(Color::Red);
		shape.setOrigin(ballRadius, ballRadius);
	}
	bool hasBottomProtection = false;

	float left()   const { return shape.getPosition().x - shape.getRadius(); }
	float right()  const { return shape.getPosition().x + shape.getRadius(); }
	float top()    const { return shape.getPosition().y - shape.getRadius(); }
	float bottom() const { return shape.getPosition().y + shape.getRadius(); }

	const CircleShape& getShape() const { return shape; }
	FloatRect getGlobalBounds() const { return shape.getGlobalBounds(); }

	void increaseVelocity();
	void update(const Paddle& paddle, vector<unique_ptr<Brick>>& bricks);
	void draw(RenderWindow& window) const { window.draw(shape); }

};

bool circleRectCollision(const CircleShape& circle, const FloatRect& rect) {
	Vector2f circleCenter = circle.getPosition();
	float radius = circle.getRadius();

	float closestX = std::max(rect.left, std::min(circleCenter.x, rect.left + rect.width));
	float closestY = std::max(rect.top, std::min(circleCenter.y, rect.top + rect.height));

	float distanceX = circleCenter.x - closestX;
	float distanceY = circleCenter.y - closestY;
	float distanceSquared = (distanceX * distanceX) + (distanceY * distanceY);

	return distanceSquared < (radius * radius);
}

class Paddle {
private:
	RectangleShape shape;
	Vector2f velocity;
public:
	Paddle(float startX, float startY) {
		shape.setPosition(startX, startY);
		shape.setSize({ paddleWidth, paddleHeight });
		shape.setFillColor(Color::White);
		shape.setOrigin(paddleWidth / 2.f, paddleHeight / 2.f);
	}

	bool isSticky = false;

	void setSticky(bool sticky) { isSticky = sticky; }
	bool getSticky() const { return isSticky; }

	RectangleShape& getShape() { return shape; }
	const RectangleShape& getShape() const { return shape; }
	float left()   const { return shape.getPosition().x - paddleWidth / 2.f; }
	float right()  const { return shape.getPosition().x + paddleWidth / 2.f; }
	float top()    const { return shape.getPosition().y - paddleHeight / 2.f; }
	float bottom() const { return shape.getPosition().y + paddleHeight / 2.f; }
	void setWidth(float width);
	void draw(RenderWindow& window) const { window.draw(shape); }
	void update(Ball& ball, Paddle& paddle);
};

void Paddle::setWidth(float width) {
	shape.setSize({ width, paddleHeight });
	shape.setOrigin(width / 2.f, paddleHeight / 2.f);
}

void Paddle::update(Ball& ball, Paddle& paddle) {
	shape.move(velocity);
	if (Keyboard::isKeyPressed(Keyboard::Key::Left) && left() > 0) { velocity.x = -paddleVelocity; }
	else if (Keyboard::isKeyPressed(Keyboard::Key::Right) && right() < windowWidth) { velocity.x = paddleVelocity; }
	else velocity.x = 0;
}


class Brick {
public:
	bool bonusSpawned = false;
	bool destroyed = false;
	int hitPoints;
	Color color;

	void markBonusSpawned() { bonusSpawned = true; }

	RectangleShape shape;
	virtual ~Brick() = default;
	Brick(float x, float y, const Color& color, int healthPoints) : hitPoints(healthPoints), color(color)
	{
		shape.setPosition(x, y);
		shape.setSize({ brickWidth, brickHeight });
		shape.setFillColor(this->color);
	}

	float left()   const { return shape.getPosition().x - brickWidth / 2.f; }
	float right()  const { return shape.getPosition().x + brickWidth / 2.f; }
	float top()    const { return shape.getPosition().y - brickHeight / 2.f; }
	float bottom() const { return shape.getPosition().y + brickHeight / 2.f; }
	void hit();
	bool isDestroyed() const { return destroyed; }
	int getHitPoints() const { return hitPoints; }
	void draw(RenderWindow& window) const { if (!destroyed) window.draw(shape); }
	const RectangleShape& getShape() const { return shape; }
	FloatRect getGlobalBounds() const { return shape.getGlobalBounds(); }

};

void Brick::hit() {
	hitPoints--;
	if (hitPoints <= 0) {
		destroyed = true;
	}
}

class RedBrick : public Brick {
public:
	RedBrick(float x, float y) : Brick(x, y, Color::Red, 1000) {}
};

class GreenBrick : public Brick {
public:
	GreenBrick(float x, float y) : Brick(x, y, Color::Green, 2) {}
};

class YellowBrick : public Brick {
public:
	YellowBrick(float x, float y) : Brick(x, y, Color::Yellow, 1) {}
};

class BlueBrick : public Brick {
public:
	BlueBrick(float x, float y) : Brick(x, y, Color::Blue, 1) {}
};

class Bonus {
private:
	Sprite sprite;
	shared_ptr<Texture> texture;
	Vector2f velocity{ 0.f, 2.f };
protected:
	bool isActive = false;
	Clock lifeClock;
public:
	Bonus(const string& texturePath, float fromX, float fromY) {
		texture = make_shared<Texture>();
		texture->loadFromFile(texturePath);
		sprite.setTexture(*texture);
		sprite.setPosition(fromX, fromY);
		float scaleX = 60.f / texture->getSize().x;
		float scaleY = 60.f / texture->getSize().y;
		sprite.setScale(scaleX, scaleY);
	}
	virtual void applyEffect(Paddle& paddle, Ball& ball) = 0;
	void draw(RenderWindow& window) { window.draw(sprite); }
	void activate();
	virtual void update();
	bool isExpired() const {return !isActive && lifeClock.getElapsedTime().asSeconds() > 7.0f;}
	FloatRect getBounds() const { return sprite.getGlobalBounds(); }
};

void Bonus::activate() {
	isActive = true;
	lifeClock.restart();
}

void Bonus::update() {
	if (isActive) {
		sprite.move(velocity);
		if (lifeClock.getElapsedTime().asSeconds() > 7.0f) {
			isActive = false;
		}
	}
}

class PaddleSizeIncrease : public Bonus {
private:
	bool applied;
	Clock boostTime;
	bool effectActive; 
public:
	PaddleSizeIncrease(float fromX, float fromY)
		: Bonus("assets/textures/paddle_size_increase.png", fromX, fromY),
		applied(false),
		effectActive(false)
	{}
	void applyEffect(Paddle& paddle, Ball& ball) override;
	void update(Paddle& paddle);

};

void PaddleSizeIncrease::applyEffect(Paddle& paddle, Ball& ball)  {
	if (!isActive || effectActive) return;
	effectActive = true;
	boostTime.restart();
	paddle.setWidth(300.f);
	isActive = false;
}

void PaddleSizeIncrease::update(Paddle& paddle) {
	if (effectActive) {
		if (boostTime.getElapsedTime().asSeconds() > 7.0f) {
			paddle.setWidth(150.f);
			effectActive = false;
		}
	}
}

class BallSpeedDown : public Bonus {
public:
	BallSpeedDown(float fromX, float fromY) : Bonus("assets/textures/ball_speed_down.png", fromX, fromY) {}
	void applyEffect(Paddle& paddle, Ball& ball) override {
		if (!isActive) return;
		ball.ballVelocity = 2.f;
		isActive = false;
	};
};

void Ball::increaseVelocity() {
	if (isBoosted) {
		if (boostClock.getElapsedTime().asSeconds() < 5.f) {
			ballVelocity = 10.f;
		}
		else {
			ballVelocity = 5.f;
			isBoosted = false;
		}
	}
}

class StickyPaddle : public Bonus {
public:
	StickyPaddle(float fromX, float fromY)
		: Bonus("assets/textures/sticky_paddle.png", fromX, fromY) {}

	void applyEffect(Paddle& paddle, Ball& ball) override {
		if (!isActive) return;
		paddle.setSticky(true);
		isActive = false;
	}
};

class OneTimeBottom : public Bonus {
public:
	OneTimeBottom(float fromX, float fromY)
		: Bonus("assets/textures/one_time_bottom.png", fromX, fromY) {}
	void applyEffect(Paddle& paddle, Ball& ball) override {
		if (!isActive) return;
		unique_ptr<Paddle> bottomPaddle = make_unique<Paddle>(windowWidth / 2, windowHeight - paddleHeight);
		bottomPaddle->setWidth(windowWidth);
		bottomPaddle->getShape().setFillColor(Color::White); 
		paddleList.push_back(move(bottomPaddle));
		isActive = false;
	}
};


void Ball::resolveCollision(const FloatRect& brickBounds) {
	float overlapLeft = right() - brickBounds.left;
	float overlapRight = brickBounds.left + brickBounds.width - left();
	float overlapTop = bottom() - brickBounds.top;
	float overlapBottom = brickBounds.top + brickBounds.height - top();

	bool fromLeft = overlapLeft < overlapRight;
	bool fromTop = overlapTop < overlapBottom;

	float minOverlapX = fromLeft ? overlapLeft : overlapRight;
	float minOverlapY = fromTop ? overlapTop : overlapBottom;

	if (minOverlapX < minOverlapY) {
		velocity.x = fromLeft ? -abs(velocity.x) : abs(velocity.x);
	}
	else {
		velocity.y = fromTop ? -abs(velocity.y) : abs(velocity.y);
	}
}

void Ball::update(const Paddle& paddle, vector<unique_ptr<Brick>>& bricks) {
	shape.move(velocity);
	increaseVelocity();

	if (left() < 0) velocity.x = ballVelocity;
	else if (right() > windowWidth) velocity.x = -ballVelocity;
	if (top() < 0) velocity.y = ballVelocity;

	//if (bottom() >= paddle.top() && right() <= paddle.right() && left() >= paddle.left()) { velocity.y = -ballVelocity; }
	if (bottom() >= windowHeight + 1) { velocity.y = -ballVelocity; }

	if (bottom() >= paddle.top() && top() <= paddle.bottom() &&
		right() >= paddle.left() && left() <= paddle.right()) {

		if (paddle.getSticky() && velocity.y > 0.f) {
			paddle.isSticky == true;
			velocity.y = 0.f;
			velocity.x = 0.f;

			shape.setPosition(paddle.getShape().getPosition().x, paddle.bottom() - ballRadius);
		}
		else if (hasBottomProtection) {
			if (Keyboard::isKeyPressed(Keyboard::Space)) {
				velocity.y = -ballVelocity;
				paddle.isSticky == false;
			}
			else {
				velocity.x = 0.0001f;
				velocity.y = 0.0f;
			}
		}
		else { velocity.y = -ballVelocity; }
	}
	//
	for (auto& p : paddleList) {
		if (getGlobalBounds().intersects(p->getShape().getGlobalBounds())) {
			velocity.y = -ballVelocity;
			break;
		}
	}

	for (auto it = paddleList.begin(); it != paddleList.end(); ) {
		if (getGlobalBounds().intersects((*it)->getShape().getGlobalBounds())) {
			velocity.y = -ballVelocity;
			it = paddleList.erase(it);
			break;
		}
		else {
			++it;
		}
	}

	for (auto& brick : bricks) {
		if (!brick->isDestroyed() && circleRectCollision(shape, brick->getGlobalBounds())) {
			resolveCollision(brick->getGlobalBounds());
			brick->hit();

			BlueBrick* blue = dynamic_cast<BlueBrick*>(brick.get());
			if (blue != nullptr) {
				if (!isBoosted) {
					isBoosted = true;
					boostClock.restart();
					ballVelocity = 10.f;
				}
			}
			RedBrick* red = dynamic_cast<RedBrick*>(brick.get());
			if (red == nullptr) {
				counter++;
			}
		}
	}
}

class ExtraBall : public Bonus {
private:
	Vector2f ballPosition;
	bool applied = false;
public:
	ExtraBall(float fromX, float fromY)
		: Bonus("assets/textures/extra_ball.png", fromX, fromY) {}

	void applyEffect(Paddle& paddle, Ball& ball) override {
		if (!isActive || applied) return;

		extraBalls.emplace_back(windowWidth / 2, windowHeight / 2);
		extraBalls.back().velocity = Vector2f(-ball.ballVelocity, -ball.ballVelocity);

		applied = true;
		isActive = false;
	}
};

int main() {
	RenderWindow window{ {windowWidth, windowHeight}, "Arkanoid" };
	window.setFramerateLimit(60);

	Text text;
	Font font;
	font.loadFromFile("assets/font/arial.ttf");
	text.setFont(font);
	text.setCharacterSize(48);
	text.setFillColor(Color::Red);
	text.setStyle(Text::Bold);

	bool DeductPoints{ true };
	bool gameStarted{ false };
	Ball ball{ windowWidth / 2, windowHeight / 2 };
	Paddle paddle(windowWidth / 2, windowHeight - 5 * ballRadius);
	vector<unique_ptr<Brick>> bricks;
	vector<unique_ptr<Bonus>> activeBonuses;

	for (int iY = 0; iY < countBricksY; ++iY) {
		for (int iX = 0; iX < countBricksX; ++iX) {
			int color = rand() % 4;
			float x = (windowWidth - (countBricksX * (brickWidth + brickPaddingX))) / 2 + iX * (brickWidth + brickPaddingX);
			float y = 100 + iY * (brickHeight + brickPaddingY);

			switch (color) {
			case 0: bricks.emplace_back(make_unique<RedBrick>(x, y)); break;
			case 1: bricks.emplace_back(make_unique<GreenBrick>(x, y)); break;
			case 2: bricks.emplace_back(make_unique<YellowBrick>(x, y)); break;
			case 3: bricks.emplace_back(make_unique<BlueBrick>(x, y)); break;
			}
		}
	}


	while (window.isOpen()) {

		while (window.isOpen() && counter >= 0) {
			Event event;
			while (window.pollEvent(event)) {
				if (event.type == Event::Closed || Keyboard::isKeyPressed(Keyboard::Key::Escape)) {
					window.close();
					return 0;
				}
				if (event.type == Event::KeyPressed && event.key.code == Keyboard::Space) {
					gameStarted = true;
				}
			}

			for (auto it = activeBonuses.begin(); it != activeBonuses.end(); ) {
				if ((*it)->isExpired()) {
					it = activeBonuses.erase(it);
				}
				else if ((*it)->getBounds().intersects(paddle.getShape().getGlobalBounds())) {
					(*it)->applyEffect(paddle, ball);
					it = activeBonuses.erase(it);
				}
				else {
					++it;
				}
			}

			for (auto& bonus : activeBonuses) {
				bonus->update();
			}

			window.clear(Color::Black);

			for (auto& brick : bricks) {
				brick->draw(window);
				YellowBrick* yellow = dynamic_cast<YellowBrick*>(brick.get());
				 
				if (brick->isDestroyed() && !brick->bonusSpawned && rand() % 50 == 0 && yellow != nullptr && rand()%2==0) {
					Vector2f pos = brick->getShape().getPosition();

					switch (rand() % 6) {
					case 0: activeBonuses.push_back(make_unique<PaddleSizeIncrease>(pos.x, pos.y)); break;
					case 1: activeBonuses.push_back(make_unique<BallSpeedDown>(pos.x, pos.y)); break;
					case 2: activeBonuses.push_back(make_unique<StickyPaddle>(pos.x, pos.y)); break;
					case 3: activeBonuses.push_back(make_unique<PaddleSizeIncrease>(pos.x, pos.y)); break;
					case 4:	activeBonuses.push_back(make_unique<OneTimeBottom>(pos.x, pos.y)); break;
					case 5:	activeBonuses.push_back(make_unique<ExtraBall>(pos.x, pos.y)); break;
					}
					brick->markBonusSpawned();
				
					if (!activeBonuses.empty()) {
						activeBonuses.back()->activate();
					}
					brick->markBonusSpawned();

				}
			}

			for (auto& bonus : activeBonuses) {
				bonus->draw(window);
			}

			if (ball.bottom() >= windowHeight) {
				counter -= 10;
			}

			if (counter < 0) {
				counter = 0;
				window.close();
				break;
			}
	
			text.setString(std::to_string(counter));
			window.draw(text);
			paddle.draw(window);
			paddle.update(ball, paddle);
			ball.update(paddle, bricks);
			ball.draw(window);

			for (auto& bonus : activeBonuses) { bonus->update(); }
			for (auto& p : paddleList) { p->draw(window); }
			for (auto& b : extraBalls) {
				b.update(paddle, bricks);
				b.draw(window);
			}
			window.display();

		}

	}

	return 0;
}

