#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <random>
#include <algorithm>

// Константы
const int WIDTH = 800;
const int HEIGHT = 600;

// Структуры
struct Projectile {
    float x, y;
    float speed, gravity;
};

struct Cloud {
    float x, y;
    float width, height;
};

struct Enemy {
    float x, y;
    float width, height;
};

// Глобальные переменные
float planeY = 0.0f;
float planeSpeedY = 0.0f;
const float acceleration = 0.001f;
const float friction = 0.02f;

std::vector<Projectile> projectiles;
std::vector<Cloud> clouds;
std::vector<Enemy> enemies;

bool spacePressedLastFrame = false;
float enemySpawnTimer = 0.0f;

// Инициализация окна
GLFWwindow* initWindow() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return nullptr;
    }

    GLFWwindow* window = glfwCreateWindow(WIDTH, HEIGHT, "Plane Game", nullptr, nullptr);
    if (!window) {
        std::cerr << "Failed to create GLFW window\n";
        glfwTerminate();
        return nullptr;
    }

    glfwMakeContextCurrent(window);
    glewInit();
    return window;
}

// Обработка ввода
void processInput(GLFWwindow* window) {
    if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
        planeSpeedY += acceleration;
    }
    if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
        planeSpeedY -= acceleration;
    }
    if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) {
        if (!spacePressedLastFrame) {
            projectiles.push_back({ -0.66f + 0.1f, planeY, 0.02f, -0.001f });
            spacePressedLastFrame = true;
        }
    }
    else {
        spacePressedLastFrame = false;
    }
}

// Обновление позиции самолёта
void updatePlane() {
    planeY += planeSpeedY;
    planeSpeedY *= (1.0f - friction);

    // Ограничение движения по вертикали
    if (planeY > 1.0f) planeY = 1.0f;
    if (planeY < -0.9f) planeY = -0.9f;
}

// Обновление снарядов
void updateProjectiles() {
    for (auto& projectile : projectiles) {
        projectile.x += projectile.speed;
        projectile.y += projectile.gravity;
    }

    // Удаление снарядов, вышедших за границы экрана
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile& p) { return p.x > 1.0f; }),
        projectiles.end());
}

// Инициализация облаков
void initClouds() {
    std::mt19937 rng(static_cast<unsigned>(time(0)));
    std::uniform_real_distribution<float> distX(-1.0f, 2.0f);
    std::uniform_real_distribution<float> distY(-0.8f, 1.0f);
    std::uniform_real_distribution<float> distW(0.1f, 0.3f);
    std::uniform_real_distribution<float> distH(0.05f, 0.15f);

    clouds.clear();
    for (int i = 0; i < 20; ++i) {
        clouds.push_back({ distX(rng), distY(rng), distW(rng), distH(rng) });
    }
}

// Обновление позиции облаков
void updateClouds(float offset) {
    for (auto& cloud : clouds) {
        cloud.x -= offset;
        if (cloud.x < -1.0f - cloud.width / 2.0f) {
            std::mt19937 rng(static_cast<unsigned>(time(0)));
            std::uniform_real_distribution<float> distY(-0.8f, 1.0f);
            cloud.x = 1.0f + cloud.width / 2.0f;
            cloud.y = distY(rng);
        }
    }
}

// Инициализация врагов
void initEnemies() {
    enemies.clear();
}

// Обновление позиции врагов
void updateEnemies() {
    const float enemySpeed = 0.01f;
    for (auto& enemy : enemies) {
        enemy.x -= enemySpeed;
    }

    // Удаление врагов, вышедших за границы экрана
    enemies.erase(std::remove_if(enemies.begin(), enemies.end(),
        [](const Enemy& e) { return e.x < -1.0f - e.width; }),
        enemies.end());

    // Добавление новых врагов
    std::mt19937 rng(static_cast<unsigned>(time(0)));
    std::uniform_real_distribution<float> distY(-0.8f, 1.0f);

    enemySpawnTimer += 0.01f; // Увеличиваем таймер

    if (enemySpawnTimer >= 1.0f) {
        enemies.push_back({ 1.0f, distY(rng), 0.4f, 0.2f });
        enemySpawnTimer = 0.0f;
    }
}

// Отрисовка самолёта
void drawPlane() {
    float planeX = -0.66f; // 1/3 экрана по горизонтали

    glColor3f(1.0f, 0.0f, 0.0f); // Красный цвет для самолёта

    // Фюзеляж
    glBegin(GL_QUADS);
    glVertex2f(planeX - 0.05f, planeY - 0.025f);
    glVertex2f(planeX + 0.1f, planeY - 0.025f);
    glVertex2f(planeX + 0.1f, planeY + 0.025f);
    glVertex2f(planeX - 0.05f, planeY + 0.025f);
    glEnd();

    // Верхнее крыло
    glBegin(GL_QUADS);
    glVertex2f(planeX + 0.02f, planeY + 0.025f);
    glVertex2f(planeX + 0.06f, planeY + 0.025f);
    glVertex2f(planeX + 0.06f, planeY + 0.075f);
    glVertex2f(planeX + 0.02f, planeY + 0.075f);
    glEnd();

    // Нижнее крыло
    glBegin(GL_QUADS);
    glVertex2f(planeX + 0.02f, planeY - 0.025f);
    glVertex2f(planeX + 0.06f, planeY - 0.025f);
    glVertex2f(planeX + 0.06f, planeY - 0.075f);
    glVertex2f(planeX + 0.02f, planeY - 0.075f);
    glEnd();

    // Хвост
    glBegin(GL_QUADS);
    glVertex2f(planeX - 0.05f, planeY + 0.025f);
    glVertex2f(planeX - 0.1f, planeY + 0.025f);
    glVertex2f(planeX - 0.1f, planeY + 0.075f);
    glVertex2f(planeX - 0.05f, planeY + 0.075f);
    glEnd();
}

// Отрисовка снарядов
void drawProjectiles() {
    glColor3f(1.0f, 0.64f, 0.0f); // Оранжевый цвет для снарядов
    for (const auto& projectile : projectiles) {
        glBegin(GL_QUADS);
        glVertex2f(projectile.x - 0.01f, projectile.y - 0.01f);
        glVertex2f(projectile.x + 0.01f, projectile.y - 0.01f);
        glVertex2f(projectile.x + 0.01f, projectile.y + 0.01f);
        glVertex2f(projectile.x - 0.01f, projectile.y + 0.01f);
        glEnd();
    }
}

// Отрисовка земли
void drawGround() {
    glColor3f(0.545f, 0.271f, 0.075f); // Коричневый цвет для земли
    glBegin(GL_QUADS);
    glVertex2f(-1.0f, -0.9f);
    glVertex2f(1.0f, -0.9f);
    glVertex2f(1.0f, -1.0f);
    glVertex2f(-1.0f, -1.0f);
    glEnd();
}

// Отрисовка облаков
void drawClouds() {
    glColor3f(1.0f, 1.0f, 1.0f); // Белый цвет для облаков
    for (const auto& cloud : clouds) {
        glBegin(GL_QUADS);
        glVertex2f(cloud.x - cloud.width / 2.0f, cloud.y - cloud.height / 2.0f);
        glVertex2f(cloud.x + cloud.width / 2.0f, cloud.y - cloud.height / 2.0f);
        glVertex2f(cloud.x + cloud.width / 2.0f, cloud.y + cloud.height / 2.0f);
        glVertex2f(cloud.x - cloud.width / 2.0f, cloud.y + cloud.height / 2.0f);
        glEnd();
    }
}

// Отрисовка врагов
void drawEnemies() {
    glColor3f(0.3f, 0.3f, 0.3f); // Темно-серый цвет для врагов
    for (const auto& enemy : enemies) {
        // Фюзеляж
        glBegin(GL_QUADS);
        glVertex2f(enemy.x + 0.1f, enemy.y - 0.05f);
        glVertex2f(enemy.x - 0.2f, enemy.y - 0.05f);
        glVertex2f(enemy.x - 0.2f, enemy.y + 0.05f);
        glVertex2f(enemy.x + 0.1f, enemy.y + 0.05f);
        glEnd();

        // Верхнее крыло
        glBegin(GL_QUADS);
        glVertex2f(enemy.x - 0.04f, enemy.y + 0.05f);
        glVertex2f(enemy.x - 0.12f, enemy.y + 0.05f);
        glVertex2f(enemy.x - 0.12f, enemy.y + 0.15f);
        glVertex2f(enemy.x - 0.04f, enemy.y + 0.15f);
        glEnd();

        // Нижнее крыло
        glBegin(GL_QUADS);
        glVertex2f(enemy.x - 0.04f, enemy.y - 0.05f);
        glVertex2f(enemy.x - 0.12f, enemy.y - 0.05f);
        glVertex2f(enemy.x - 0.12f, enemy.y - 0.15f);
        glVertex2f(enemy.x - 0.04f, enemy.y - 0.15f);
        glEnd();

        // Хвост
        glBegin(GL_QUADS);
        glVertex2f(enemy.x + 0.1f, enemy.y + 0.05f);
        glVertex2f(enemy.x + 0.2f, enemy.y + 0.05f);
        glVertex2f(enemy.x + 0.2f, enemy.y + 0.15f);
        glVertex2f(enemy.x + 0.1f, enemy.y + 0.15f);
        glEnd();
    }
}

// Проверка на столкновение
bool checkCollision(float x1, float y1, float w1, float h1, float x2, float y2, float w2, float h2) {
    return (x1 < x2 + w2 && x1 + w1 > x2 && y1 < y2 + h2 && y1 + h1 > y2);
}

// Проверка всех столкновений
bool checkCollisions() {
    // Проверка столкновения самолета с землей
    if (planeY - 0.025f <= -0.9f) {
        return true; // Столкновение с землей
    }

    // Проверка столкновения самолета с врагами
    float planeX = -0.66f; // 1/3 экрана по горизонтали
    for (const auto& enemy : enemies) {
        if (checkCollision(planeX - 0.05f, planeY - 0.025f, 0.1f, 0.05f,
            enemy.x - enemy.width / 2.0f, enemy.y - enemy.height / 2.0f, enemy.width, enemy.height)) {
            return true; // Столкновение с врагом
        }
    }

    return false; // Нет столкновений
}

// Проверка на попадание снарядов во врагов
void checkProjectileCollisions() {
    projectiles.erase(std::remove_if(projectiles.begin(), projectiles.end(),
        [](const Projectile& p) {
            for (auto& enemy : enemies) {
                if (checkCollision(p.x - 0.01f, p.y - 0.01f, 0.02f, 0.02f,
                    enemy.x - enemy.width / 2.0f, enemy.y - enemy.height / 2.0f,
                    enemy.width, enemy.height)) {
                    enemy.x = -2.0f; // Убираем врага
                    return true; // Убираем снаряд
                }
            }
            return false;
        }),
        projectiles.end());
}

int main() {
    GLFWwindow* window = initWindow();
    if (!window) return -1;

    initClouds();
    initEnemies();

    // Установка голубого фона
    glClearColor(0.53f, 0.81f, 0.98f, 1.0f);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT);

        // Обработка ввода
        processInput(window);

        // Обновление игры
        updatePlane();
        updateProjectiles();
        updateClouds(0.01f); // скорость движения облаков
        updateEnemies();

        // Проверка столкновений
        if (checkCollisions()) {
            std::cout << "Game Over!" << std::endl;
            break; // Завершение игры
        }

        checkProjectileCollisions();

        // Отрисовка
        drawGround();
        drawClouds();
        drawPlane();
        drawProjectiles();
        drawEnemies();

        // Переключение кадров
        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glfwDestroyWindow(window);
    glfwTerminate();
    return 0;
}
