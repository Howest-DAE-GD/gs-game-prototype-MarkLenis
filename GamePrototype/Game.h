#pragma once
#include "BaseGame.h"
#include "Texture.h"
#include <SDL_ttf.h>
#include <vector>

struct Balloon
{
    Point2f position;
    float radius;
    Color4f color;
    bool isPopped;
    bool isBomb;
    bool randGenerated;
    bool isSpecial; 
};

class Game : public BaseGame
{
public:
    explicit Game(const Window& window);
    Game(const Game& other) = delete;
    Game& operator=(const Game& other) = delete;
    Game(Game&& other) = delete;
    Game& operator=(Game&& other) = delete;
    ~Game();

    void Update(float elapsedSec) override;
    void Draw() const override;

    // Event handling
    void ProcessKeyDownEvent(const SDL_KeyboardEvent& e) override;
    void ProcessKeyUpEvent(const SDL_KeyboardEvent& e) override;
    void ProcessMouseMotionEvent(const SDL_MouseMotionEvent& e) override;
    void ProcessMouseDownEvent(const SDL_MouseButtonEvent& e) override;
    void ProcessMouseUpEvent(const SDL_MouseButtonEvent& e) override;

private:
    // FUNCTIONS
    void Initialize();
    void Cleanup();
    void ClearBackground() const;
    void SpawnBalloon();
    void SpawnMultipleBalloons(int count);
    void MoveBalloons(float elapsedSec);
    void PopBalloon(float x, float y);
    void DrawWavyString(const Point2f& start, float length) const;
    void DrawBalloon(const Balloon& balloon) const;
    void DrawScore() const;
    void DrawLives() const;
    void ShowGameOver() const;
    void PopAllBalloons(); 

    // MEMBER VARIABLES
    std::vector<Balloon> m_Balloons;
    float m_SpawnTimer;
    float m_SpawnInterval;
    Color4f m_BackgroundColor;
    int m_Score;
    int m_BalloonsOffScreen;
    float m_GameTime;
    float m_BalloonSpeed;
    int m_Lives;
    TTF_Font* m_Font;
    Texture* m_ScoreTexture;
    Texture* m_LivesTexture;
    bool m_IsGameOver;
};
