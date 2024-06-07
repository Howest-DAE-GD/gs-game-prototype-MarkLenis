#include "pch.h"
#include "Game.h"
#include "utils.h"
#include "Texture.h"
#include <cstdlib>
#include <ctime>
#include <cmath>
#include <iostream>

using namespace utils;

Game::Game(const Window& window)
    : BaseGame{ window }, m_IsGameOver(false)
{
    Initialize();
}

Game::~Game()
{
    Cleanup();
}

void Game::Initialize()
{
    std::srand(static_cast<unsigned int>(std::time(nullptr)));
    m_SpawnTimer = 0.0f;
    m_SpawnInterval = 1.0f;
    m_BackgroundColor = Color4f{ 1.0f, 1.0f, 1.0f, 1.0f };
    m_Score = 0;
    m_BalloonsOffScreen = 0;
    m_GameTime = 0.0f;
    m_BalloonSpeed = 100.0f;
    m_Lives = 5;

    if (TTF_Init() == -1)
    {
        std::cerr << "TTF_Init: " << TTF_GetError() << std::endl;
        exit(1);
    }

    m_Font = TTF_OpenFont("arial.ttf", 24); 
    if (!m_Font)
    {
        std::cerr << "TTF_OpenFont: " << TTF_GetError() << std::endl;
        exit(1);
    }

    m_ScoreTexture = new Texture("Score: 0", "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
    m_LivesTexture = new Texture("Lives: 5", "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
}

void Game::Cleanup()
{
    TTF_CloseFont(m_Font);
    TTF_Quit();
    delete m_ScoreTexture;
    delete m_LivesTexture;
    m_Balloons.clear();
}

void Game::Update(float elapsedSec)
{
    if (m_IsGameOver)
    {
        return;
    }

    if (m_BalloonsOffScreen >= 5 || m_Lives <= 0)
    {
        m_IsGameOver = true;
        return;
    }

    m_GameTime += elapsedSec;
    m_SpawnInterval = std::max(0.2f, 1.0f - m_GameTime * 0.01f);
    m_BalloonSpeed = 100.0f + m_GameTime * 10.0f;

    m_SpawnTimer += elapsedSec;
    while (m_SpawnTimer >= m_SpawnInterval)
    {
        m_SpawnTimer -= m_SpawnInterval;

        if (std::rand() % 20 == 0)
        {
            SpawnMultipleBalloons(4);
        }
        else
        {
            SpawnBalloon();
        }
    }

    MoveBalloons(elapsedSec);

    std::string scoreText = "Score: " + std::to_string(m_Score);
    delete m_ScoreTexture;
    m_ScoreTexture = new Texture(scoreText, "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });

    std::string livesText = "Lives: " + std::to_string(m_Lives);
    delete m_LivesTexture;
    m_LivesTexture = new Texture(livesText, "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
}

void Game::Draw() const
{
    ClearBackground();

    for (const Balloon& balloon : m_Balloons)
    {
        if (!balloon.isPopped)
        {
            DrawBalloon(balloon);
        }
    }

    DrawScore();
    DrawLives();

    if (m_IsGameOver)
    {
        ShowGameOver();
    }
}

void Game::ProcessKeyDownEvent(const SDL_KeyboardEvent& e)
{
    if (e.keysym.sym == SDLK_r)
    {
        ResetGame();
    }
}

void Game::ProcessKeyUpEvent(const SDL_KeyboardEvent& e)
{
}

void Game::ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e)
{
}

void Game::ProcessMouseDownEvent(const SDL_MouseButtonEvent& e)
{
    if (e.button == SDL_BUTTON_LEFT)
    {
        PopBalloon(static_cast<float>(e.x), static_cast<float>(e.y));
    }
}

void Game::ProcessMouseUpEvent(const SDL_MouseButtonEvent& e)
{
}

void Game::ClearBackground() const
{
    glClearColor(m_BackgroundColor.r, m_BackgroundColor.g, m_BackgroundColor.b, m_BackgroundColor.a);
    glClear(GL_COLOR_BUFFER_BIT);
}

void Game::SpawnBalloon()
{
    Balloon balloon;
    balloon.position = Point2f{ static_cast<float>(std::rand() % static_cast<int>(GetViewPort().width)), 0.0f };
    balloon.radius = 15.0f;
    balloon.color = Color4f{ static_cast<float>(std::rand() % 256) / 255.0f,
                             static_cast<float>(std::rand() % 256) / 255.0f,
                             static_cast<float>(std::rand() % 256) / 255.0f,
                             1.0f };
    balloon.isPopped = false;
    balloon.isBomb = false;
    balloon.randGenerated = false;
    m_Balloons.push_back(balloon);
}

void Game::SpawnMultipleBalloons(int count)
{
    for (int i = 0; i < count; ++i)
    {
        SpawnBalloon();
    }
}

void Game::MoveBalloons(float elapsedSec)
{
    for (Balloon& balloon : m_Balloons)
    {
        if (!balloon.isPopped)
        {
            balloon.position.y += m_BalloonSpeed * elapsedSec;
            balloon.radius += m_BalloonSpeed * elapsedSec * 0.05f;

            if (balloon.position.y - balloon.radius > GetViewPort().height)
            {
                balloon.isPopped = true;
                m_Lives--;
                m_BalloonsOffScreen++;
            }
            else
            {
                if (!balloon.isBomb && balloon.position.y > GetViewPort().height * 3 / 4 && !balloon.randGenerated)
                {
                    int randNumber = std::rand() % 100 + 1;
                    balloon.randGenerated = true;
                    std::cout << randNumber << std::endl;

                    if (randNumber <= 8)
                    {
                        balloon.isBomb = true;
                        balloon.color = Color4f{ 0.0f, 0.0f, 0.0f, 1.0f };
                    }
                }
            }
        }
    }

    m_Balloons.erase(std::remove_if(m_Balloons.begin(), m_Balloons.end(),
        [](const Balloon& balloon) { return balloon.isPopped; }),
        m_Balloons.end());
}

void Game::PopBalloon(float x, float y)
{
    for (Balloon& balloon : m_Balloons)
    {
        if (!balloon.isPopped)
        {
            float dx = balloon.position.x - x;
            float dy = balloon.position.y - y;
            if (std::sqrt(dx * dx + dy * dy) < balloon.radius)
            {
                if (balloon.isBomb)
                {
                    m_Lives--;
                    std::cout << "Bomb balloon popped! Lives left: " << m_Lives << std::endl;
                }
                balloon.isPopped = true;

                float height = balloon.position.y;
                float screenHeight = GetViewPort().height;

                if (height >= screenHeight * 3 / 4)
                {
                    m_Score += 4;
                    std::cout << "Score: " << m_Score << " (4 points)\n";
                }
                else if (height >= screenHeight / 2)
                {
                    m_Score += 3;
                    std::cout << "Score: " << m_Score << " (3 points)\n";
                }
                else if (height >= screenHeight / 4)
                {
                    m_Score += 2;
                    std::cout << "Score: " << m_Score << " (2 points)\n";
                }
                else
                {
                    m_Score += 1;
                    std::cout << "Score: " << m_Score << " (1 point)\n";
                }

                break;
            }
        }
    }
}

void Game::DrawWavyString(const Point2f& start, float length) const
{
    SetColor(Color4f{ 0.5f, 0.5f, 0.5f, 1.0f });

    const int segments = 10;
    const float waveHeight = 2.0f;
    const float segmentLength = length / segments;

    std::vector<Point2f> points;
    for (int i = 0; i <= segments; ++i)
    {
        float x = start.x + (i % 2 == 0 ? waveHeight : -waveHeight);
        float y = start.y - i * segmentLength;
        points.emplace_back(x, y);
    }

    for (size_t i = 1; i < points.size(); ++i)
    {
        DrawLine(points[i - 1], points[i], 2.0f);
    }
}

void Game::DrawBalloon(const Balloon& balloon) const
{
    SetColor(balloon.color);
    FillEllipse(balloon.position, balloon.radius, balloon.radius);

    if (!balloon.isBomb)
    {
        Point2f bottomCenter = Point2f{ balloon.position.x, balloon.position.y - balloon.radius };
        DrawWavyString(bottomCenter, 50.0f);
        Point2f triangleLeft = Point2f{ balloon.position.x - 5, balloon.position.y - balloon.radius };
        Point2f triangleRight = Point2f{ balloon.position.x + 5, balloon.position.y - balloon.radius };
        Point2f triangleBottom = Point2f{ balloon.position.x, balloon.position.y - balloon.radius - 10 };
        SetColor(balloon.color);
        FillTriangle(triangleLeft, triangleRight, triangleBottom);
    }
    else
    {
        SetColor(Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
        FillRect(Rectf{ balloon.position.x - 2, balloon.position.y + balloon.radius, 4, 10 });
        FillEllipse(Point2f{ balloon.position.x, balloon.position.y + balloon.radius + 15 }, 5, 5);
    }
}

void Game::DrawScore() const
{
    m_ScoreTexture->Draw(Point2f{ 10, GetViewPort().height - 30 });
}

void Game::DrawLives() const
{
    m_LivesTexture->Draw(Point2f{ GetViewPort().width - 90, GetViewPort().height - 30 }); 
}

void Game::ShowGameOver() const
{
    Color4f overlayColor = Color4f{ 0.0f, 0.0f, 0.0f, 0.5f };
    SetColor(overlayColor);
    FillRect(0, 0, GetViewPort().width, GetViewPort().height);

    std::string gameOverText = "Game Over! Final Score: " + std::to_string(m_Score) + "    Time Survived: " + std::to_string(static_cast<int>(m_GameTime)) + "s";
    Texture gameOverTexture(gameOverText, "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
    gameOverTexture.Draw(Point2f{ GetViewPort().width / 2 - 260, GetViewPort().height / 2 });
}

void Game::ResetGame()
{
    m_Balloons.clear();
    m_SpawnTimer = 0.0f;
    m_SpawnInterval = 1.0f;
    m_Score = 0;
    m_BalloonsOffScreen = 0;
    m_GameTime = 0.0f;
    m_BalloonSpeed = 100.0f;
    m_Lives = 5;
    m_IsGameOver = false;

    delete m_ScoreTexture;
    m_ScoreTexture = new Texture("Score: 0", "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });

    delete m_LivesTexture;
    m_LivesTexture = new Texture("Lives: 5", "arial.ttf", 24, Color4f{ 1.0f, 0.0f, 0.0f, 1.0f });
}
