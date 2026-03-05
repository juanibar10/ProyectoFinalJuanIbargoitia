#include "Collision.hpp"

bool Collision::circleRectHitBasic(sf::Vector2f center, float radius, const sf::FloatRect& rb, sf::Vector2f& pushOut)
{
    float closestX = center.x;
    if (closestX < rb.left) closestX = rb.left;
    if (closestX > rb.left + rb.width) closestX = rb.left + rb.width;

    float closestY = center.y;
    if (closestY < rb.top) closestY = rb.top;
    if (closestY > rb.top + rb.height) closestY = rb.top + rb.height;

    const float dx = center.x - closestX;
    const float dy = center.y - closestY;
    const float dist2 = dx * dx + dy * dy;
    if (dist2 > radius * radius) return false;

    const float adx = (dx < 0.f) ? -dx : dx;
    const float ady = (dy < 0.f) ? -dy : dy;

    // Caso especial: centro dentro del rectángulo (dx=dy=0)
    if (adx < 0.0001f && ady < 0.0001f)
    {
        const float leftPen   = (center.x - rb.left);
        const float rightPen  = ((rb.left + rb.width) - center.x);
        const float topPen    = (center.y - rb.top);
        const float bottomPen = ((rb.top + rb.height) - center.y);

        float best = leftPen;
        pushOut = sf::Vector2f(-1.f, 0.f);

        if (rightPen < best)  { best = rightPen;  pushOut = sf::Vector2f( 1.f, 0.f); }
        if (topPen < best)    { best = topPen;    pushOut = sf::Vector2f( 0.f,-1.f); }
        if (bottomPen < best) {                   pushOut = sf::Vector2f( 0.f, 1.f); }

        return true;
    }

    if (adx > ady)
    {
        pushOut = sf::Vector2f((dx > 0.f) ? 1.f : -1.f, 0.f);
    }
    else
    {
        pushOut = sf::Vector2f(0.f, (dy > 0.f) ? 1.f : -1.f);
    }

    return true;
}

bool Collision::rectRectHit(const sf::RectangleShape& a, const sf::RectangleShape& b)
{
    return a.getGlobalBounds().intersects(b.getGlobalBounds());
}

sf::Uint8 Collision::clampByte(int v)
{
    if (v < 0) v = 0;
    if (v > 255) v = 255;
    return static_cast<sf::Uint8>(v);
}

sf::Color Collision::darker(sf::Color c, float factor)
{
    if (factor < 0.f) factor = 0.f;
    if (factor > 1.f) factor = 1.f;

    int r = static_cast<int>(static_cast<float>(c.r) * factor);
    int g = static_cast<int>(static_cast<float>(c.g) * factor);
    int b = static_cast<int>(static_cast<float>(c.b) * factor);

    c.r = clampByte(r);
    c.g = clampByte(g);
    c.b = clampByte(b);
    return c;
}


void Collision::removeDeadBricks(std::vector<Entities::Brick>& bricks)
{
    std::vector<Entities::Brick> alive;

    for (std::size_t i = 0; i < bricks.size(); ++i)
    {
        if (bricks[i].alive)
            alive.push_back(bricks[i]);
    }

    bricks = alive;
}

void Collision::keepPaddleInBounds(Entities::Paddle& paddle, float windowWidth)
{
    sf::FloatRect b = paddle.shape.getGlobalBounds();

    if (b.left < 0.f)
        paddle.shape.setPosition(b.width / 2.f, paddle.shape.getPosition().y);

    if (b.left + b.width > windowWidth)
        paddle.shape.setPosition(windowWidth - b.width / 2.f, paddle.shape.getPosition().y);
}

void Collision::stepBalls(std::vector<Entities::Ball>& balls, float dt, float windowWidth, float windowHeight, float hudHeight, const Entities::Paddle& paddle, std::vector<Entities::Brick>& bricks, StepResult& out)
{
    for (std::size_t i = 0; i < balls.size(); )
    {
        Entities::Ball& ball = balls[i];
        ball.shape.move(ball.velocity * dt);

        const float r = ball.shape.getRadius();
        sf::Vector2f pos = ball.shape.getPosition();

        // Walls
        if (pos.x - r < 0.f)
        {
            pos.x = r;
            if (ball.velocity.x < 0.f) ball.velocity.x = -ball.velocity.x;
        }
        if (pos.x + r > windowWidth)
        {
            pos.x = windowWidth - r;
            if (ball.velocity.x > 0.f) ball.velocity.x = -ball.velocity.x;
        }
        if (pos.y - r < hudHeight)
        {
            pos.y = hudHeight + r;
            if (ball.velocity.y < 0.f) ball.velocity.y = -ball.velocity.y;
        }

        if (pos.y - r > windowHeight)
        {
            out.ballLost = true;
            balls[i] = balls.back();
            balls.pop_back();
            continue;
        }

        ball.shape.setPosition(pos);

        // Paddle
        const sf::FloatRect pb = paddle.shape.getGlobalBounds();
        sf::Vector2f push;

        if (circleRectHitBasic(ball.shape.getPosition(), r, pb, push) && ball.velocity.y > 0.f)
        {
            if (push.x != 0.f) ball.velocity.x = -ball.velocity.x;
            if (push.y != 0.f) ball.velocity.y = -ball.velocity.y;

            if (ball.velocity.y > 0.f) ball.velocity.y = -ball.velocity.y;

            const float centerX = pb.left + pb.width / 2.f;
            float rel = (ball.shape.getPosition().x - centerX);
            rel = rel / (pb.width / 2.f);
            ball.velocity.x += rel * 60.f;

            ball.shape.move(ball.velocity * dt);
        }

        // Bricks
        for (std::size_t b = 0; b < bricks.size(); ++b)
        {
            Entities::Brick& br = bricks[b];
            if (!br.alive) continue;

            sf::Vector2f pushBrick;
            if (!circleRectHitBasic(ball.shape.getPosition(), r, br.shape.getGlobalBounds(), pushBrick))
                continue;

            br.hp = br.hp - 1;
            if (br.hp < 0) br.hp = 0;

            if (br.hp > 0)
            {
                br.shape.setFillColor(darker(br.shape.getFillColor(), 0.85f));
            }

            if (br.hp <= 0)
            {
                const sf::Vector2f breakPos = br.shape.getPosition();
                const sf::Color breakColor = br.shape.getFillColor();

                br.alive = false;
                out.bricksDestroyed++;

                out.brokeBrick = true;
                out.brokePos = breakPos;
                out.brokeColor = breakColor;
            }

            if (pushBrick.x != 0.f) ball.velocity.x = -ball.velocity.x;
            if (pushBrick.y != 0.f) ball.velocity.y = -ball.velocity.y;

            ball.shape.move(ball.velocity * dt);
            break;
        }

        ++i;
    }

    removeDeadBricks(bricks);
}

void Collision::stepPowerUps(std::vector<Entities::PowerUp>& powerUps, float dt, float windowHeight, const Entities::Paddle& paddle, StepResult& out)
{
    for (std::size_t i = 0; i < powerUps.size(); )
    {
        Entities::PowerUp& p = powerUps[i];
        p.shape.move(p.velocity * dt);

        const sf::FloatRect b = p.shape.getGlobalBounds();

        if (b.top > windowHeight)
        {
            powerUps[i] = powerUps.back();
            powerUps.pop_back();
            continue;
        }

        if (rectRectHit(p.shape, paddle.shape))
        {
            out.collectedPowerUp = true;
            out.collectedPos = p.shape.getPosition();

            powerUps[i] = powerUps.back();
            powerUps.pop_back();
            continue;
        }

        ++i;
    }
}