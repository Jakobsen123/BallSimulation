#include "raylib.h"
#include <string>
#include <vector>
#include <algorithm>
#include <iostream>
#include <cmath>

Color BACKGROUND = Color{23, 23, 23};

class Circle
{
public:
    Circle(float varX, float varY, float varM, float varRad, float varRestitution)
    {
        x = varX;
        y = varY;
        m = varM;
        rad = varRad;
        restitution = varRestitution;
    }
    bool move = true;

    void Update(float dt, float gravity, Vector2 screenDimension)
    {
        if (move)
        {
            velocity.y += gravity * dt;
            y += velocity.y * dt;
            x += velocity.x * dt;

            if (y + rad >= screenDimension.y)
            {
                y = screenDimension.y - rad;
                velocity.y *= -restitution;
            }
            if (y - rad <= 0)
            {
                y = 0 + rad;
                velocity.y *= -restitution;
            }
            if (x + rad >= screenDimension.x)
            {
                x = screenDimension.x - rad;
                velocity.x *= -restitution;
            }
            if (x - rad <= 0)
            {
                x = 0 + rad;
                velocity.x *= -restitution;
            }
        }
    }
    void updVelocity(Vector2 v)
    {
        velocity.y *= v.y;
        velocity.x *= v.x;
    }
    void stopVelocity()
    {
        velocity.y = 0;
    }

    void Draw() const
    {
        DrawCircle(x, y, rad, color);
    }
    Vector2 getV() const
    {
        return velocity;
    }
    float getRad() const
    {
        return rad;
    }
    float getMass() const
    {
        return m;
    }
    Vector2 getPos() const
    {
        return {x, y};
    }
    void updateColor(Color varColor)
    {
        color = varColor;
    }
    void updatePos(Vector2 pos)
    {
        x = pos.x;
        y = pos.y;
    }
    void setV(Vector2 v)
    {
        velocity = v;
    }

private:
    float rad = 10;
    float x = 50.0f;
    float y = 50.0f;
    Vector2 velocity = {0.0f, 0.0f};
    float m = 15.0f;
    float restitution = 0.9f;
    Color color = RED;
};

void spawnCircle(std::vector<Circle> &circleVec, const int x, const int y, const bool runSim, const float rad, const float restitution)
{
    Circle object(x, y, rad, rad, restitution);
    object.move = runSim;
    circleVec.push_back(object);
};

void DrawVectorSize(const std::vector<Circle> &vec, int x, int y)
{
    DrawText(TextFormat("%d", vec.size()), x, y, 50, WHITE);
}

void resolveCollision(Circle &circle, Circle &otherCircle)
{
    Vector2 dir = {circle.getPos().x - otherCircle.getPos().x, circle.getPos().y - otherCircle.getPos().y};
    float distance = sqrtf(dir.x * dir.x + dir.y * dir.y);
    float radiusSum = circle.getRad() + otherCircle.getRad();

    if (distance <= radiusSum)
    {
        circle.updateColor(BLUE);
        otherCircle.updateColor(BLUE);
        if (distance == 0)
        {
            return;
        }
        dir = {dir.x / distance, dir.y / distance};

        float overlap = radiusSum - distance;
        float totalMass = circle.getMass() + otherCircle.getMass();
        float pushCircle = overlap * (otherCircle.getMass() / totalMass);
        float pushOther = overlap * (circle.getMass() / totalMass);

        Vector2 posA = circle.getPos();
        Vector2 posB = otherCircle.getPos();
        circle.updatePos({posA.x + dir.x * pushCircle, posA.y + dir.y * pushCircle});
        otherCircle.updatePos({posB.x - dir.x * pushOther, posB.y - dir.y * pushOther});

        float circlevdir = circle.getV().x * dir.x + circle.getV().y * dir.y;
        float circle2vdir = otherCircle.getV().x * dir.x + otherCircle.getV().y * dir.y;
        float diff = circle2vdir - circlevdir;
        Vector2 change = {dir.x * diff, dir.y * diff};
        circle.setV({circle.getV().x + change.x, circle.getV().y + change.y});
        otherCircle.setV({otherCircle.getV().x - change.x, otherCircle.getV().y - change.y});
    }
}

void freezeObjects(std::vector<Circle> &vec, const bool freeze)
{
    for (Circle &o : vec)
    {
        o.move = freeze;
    }
}

template <typename T>
void clearVector(T &vec)
{
    vec.clear();
    vec.shrink_to_fit();
}

void calculateMouseVelocity(Vector2 &mv, Vector2 &lastmpos, const float dt)
{
    Vector2 currentMousePos = GetMousePosition();
    Vector2 mouseDir = {currentMousePos.x - lastmpos.x, currentMousePos.y - lastmpos.y};
    mv = {mouseDir.x / dt, mouseDir.y / dt};
    lastmpos = GetMousePosition();
}

void handleScroll(const float scroll, float &radVar)
{
    if (scroll > 0.0f)
    {
        if (radVar < 25)
            radVar += 1;
    }
    else if (scroll < 0.0f)
    {

        if (radVar > 5)
            radVar -= 1;
    }
}

void restitutionHandler(float &restitution, const bool increase)
{
    if (increase)
    {
        if (restitution <= 2.5)
        {
            restitution += 0.1f;
        }
    }
    else
    {
        if (restitution >= 0.3)
        {
            restitution -= 0.1f;
        }
    }
}

int main(void)
{
    const int screenWidth = 960;
    const int screenHeight = 560;
    const float gravity = 1500.0f;

    const std::string andreas = "Fis";

    std::vector<Circle> objects;
    Circle *mvobject = nullptr;
    float spawnCooldown = 0.0f;
    bool runSim = true;
    float rad = 10;
    float restitution = 0.9f;

    Vector2 lastMousePos = GetMousePosition();
    Vector2 mouseVelocity;

    InitWindow(screenWidth, screenHeight, "Ball Simulation");
    SetTargetFPS(60);
    SetRandomSeed((unsigned int)time(NULL));
    while (!WindowShouldClose())

    {
        float dt = GetFrameTime();
        float scroll = GetMouseWheelMove();

        calculateMouseVelocity(mouseVelocity, lastMousePos, dt);
        if (IsKeyDown(KEY_UP))
        {
            restitutionHandler(restitution, true);
        }
        else if (IsKeyDown(KEY_DOWN))
        {
            restitutionHandler(restitution, false);
        }

        if (IsMouseButtonPressed(MOUSE_BUTTON_LEFT) && mvobject == nullptr)
        {
            spawnCircle(objects, GetMouseX(), GetMouseY(), runSim, rad, restitution);
        }
        if (spawnCooldown > 0.0f)
        {
            spawnCooldown -= dt;
        }
        if (scroll != 0.0f)
        {
            handleScroll(scroll, rad);
        }

        if (IsKeyDown(KEY_Q) && spawnCooldown <= 0.0f)
        {
            spawnCircle(objects, GetMouseX(), GetMouseY(), runSim, rad, restitution);
            spawnCooldown = 0.01f;
        }
        if (IsKeyDown(KEY_E))
        {
            if (mvobject == nullptr)
            {
                for (Circle &c : objects)
                {
                    Vector2 mpos = GetMousePosition();
                    Vector2 cpos = c.getPos();
                    int r = c.getRad();

                    float dx = mpos.x - cpos.x;
                    float dy = mpos.y - cpos.y;
                    float dist = sqrt(dx * dx + dy * dy);
                    if (dist <= r)
                    {
                        mvobject = &c;
                        c.stopVelocity();
                        break;
                    }
                }
            }
        }
        if (IsKeyPressed(KEY_F))
        {
            runSim = !runSim;
            freezeObjects(objects, runSim);
        }
        if (IsKeyPressed(KEY_C))
        {
            clearVector(objects);
        }
        if (IsKeyPressed(KEY_R))
        {
            if (!objects.empty())
            {
                objects.pop_back();
            }
        }
        if (IsKeyReleased(KEY_E) && mvobject != nullptr)
        {
            mvobject->move = runSim ? true : false;
            mvobject->setV(mouseVelocity);
            mvobject = nullptr;
        }
        if (mvobject != nullptr)
        {
            mvobject->move = false;
            mvobject->updatePos(GetMousePosition());
        }

        BeginDrawing();
        ClearBackground(BACKGROUND);
        if (objects.size() == 2)
        {
            DrawLine(objects[0].getPos().x, objects[0].getPos().y, objects[1].getPos().x, objects[1].getPos().y, RED);
        }
        for (Circle &c : objects)
        {
            c.updateColor(RED);
        }
        for (int i{0}; i < objects.size(); i++)
        {
            for (int j{i + 1}; j < objects.size(); j++)
            {
                resolveCollision(objects[i], objects[j]);
            }
        }
        for (Circle &i : objects)
        {
            i.Update(dt, gravity, {screenWidth, screenHeight});
            i.Draw();
        }
        DrawText(TextFormat("Radius: %f", rad), 0 + 50, screenHeight - 30, 25, WHITE);
        DrawText(TextFormat("Restitution: %f", restitution), 0 + 50, screenHeight - 50, 25, WHITE);
        DrawFPS(screenWidth - 150, 0 + 50);
        DrawVectorSize(objects, (screenWidth / 2) - 50, screenHeight - 50);
        EndDrawing();
    }
    CloseWindow();
    return 0;
}