#include <random>

#include "globals.h"

#include "Render/WindowManager.h"
#include "Input.h"
#include "Time.h"
#include "Math.h"
#include "R.h"

class Point;
class Stick;

typedef std::vector <Point*> PointVector;
typedef std::vector <Stick> StickVector;

struct Point : sf::Drawable {
    static constexpr float RADIUS = 3;

    Point(sf::Vector2f p, bool i_l, float m) : shape(RADIUS, 20), pos(p), prev_pos(p), mass(m) {
        SetLockState(i_l);
    }

    sf::Vector2f pos;
    sf::Vector2f prev_pos;

    float mass;

    bool is_locked;

    mutable sf::CircleShape shape;

    void SetLockState(bool i_l) {
        is_locked = i_l;

        shape.setFillColor(is_locked ? sf::Color::Red : sf::Color::White);
    }

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        shape.setPosition(pos - RADIUS);

        target.draw(shape, states);
    }
};

float prev_delta = 1;

struct Stick : sf::Drawable {
    Stick(Point* a, Point* b) : Stick(a, b, fun::Math::Distance(a->pos, b->pos)) {}
    Stick(Point* a, Point* b, float l) : length(l) {
        vertices = { sf::Vertex(), sf::Vertex() };

        point_a = a;
        point_b = b;
    }

    Point* point_a;
    Point* point_b;

    mutable std::array <sf::Vertex, 2> vertices;

    float length;

    void draw(sf::RenderTarget& target, sf::RenderStates states) const override {
        vertices[0].position = point_a->pos;
        vertices[1].position = point_b->pos;

        target.draw(vertices.begin(), 2, sf::Lines, states);
    }
};

void simulate(PointVector& points, StickVector& sticks) {
    static const float gravity = 2.0681f;
    static const int iterations = 2;

    for (auto point : points) {
        if (point->is_locked) continue;

        const sf::Vector2f curr_pos = point->pos;

        point->pos += (point->pos - point->prev_pos) * fun::Time::DeltaTime() / prev_delta;
        point->pos += sf::Vector2f(0, gravity * point->mass * fun::Time::DeltaTime());

        point->prev_pos = curr_pos;
    }

    for (int i = 0; i < iterations; i++) {
        for (auto& stick : sticks) {
            //std::shuffle(sticks.begin(), sticks.end(), std::mt19937(std::random_device()()));

            const sf::Vector2f stick_center = (stick.point_a->pos + stick.point_b->pos) * .5f;
            const sf::Vector2f stick_dir = fun::Math::Normalize(stick.point_a->pos - stick.point_b->pos);
            const sf::Vector2f stick_delta = stick_dir * stick.length * .5f;

            if (!stick.point_a->is_locked) stick.point_a->pos = stick_center + stick_delta;
            if (!stick.point_b->is_locked) stick.point_b->pos = stick_center - stick_delta;
        }
    }

    prev_delta = fun::Time::DeltaTime();
}

int main () {
    glob_init();

    fun::R::LoadResources();

    fun::WindowManager::Init("Cloth Simulation");
    fun::WindowManager::WindowData* window_data = fun::WindowManager::main_window;

    PointVector points;
    StickVector sticks;

    int width = 101;
    int height = 101;

    for (int i = 0; i < width; i++) {
        for (int j = 0; j < height; j++) {
            bool is_locked = !(i % 20) && j == 0;

            float l = 30;
            int x = i * l;
            int y = j * l;

            points.emplace_back(new Point(sf::Vector2f(x, y), is_locked, 1));
        }
    }

    auto coord_to_index = [&](int i, int j) {
        return width * j + i;
    };

    for (int i = 0; i < width - 1; i++) {
        for (int j = 0; j < height - 1; j++) {
            float length = 30;

            if (i < width - 1) sticks.emplace_back(points[coord_to_index(i, j)], points[coord_to_index(i + 1, j)], length);
            if (j < height - 1) sticks.emplace_back(points[coord_to_index(i, j)], points[coord_to_index(i, j + 1)], length);
        }
    }

    while (window_data->window.isOpen()) {
        fun::Input::Listen();
        fun::Time::Recalculate();

        window_data->PollEvents();
        window_data->world_view.move(fun::Input::K2D() * sf::Vector2f(1, -1) * 30.f);

        if (sticks.size() && fun::Input::Hold(sf::Keyboard::Space)) {
            for (int i = 0; i < 30; i++) {
                sticks.erase(sticks.begin() + rand() % sticks.size());
            }
        }

        if (fun::Input::Hold(sf::Keyboard::E)) {
            for (int i = 0; i < 100; i++) {
                Point &point = *points[rand() % points.size()];

                point.SetLockState(!point.is_locked);
            }
        }

        simulate(points, sticks);

        for (auto& stick : sticks) {
            window_data->AddWorld(stick, 0);
        }

        for (auto point : points) {
            window_data->AddWorld(*point, 1);
        }

        window_data->Display(sf::Color::Black);
    }

    return 0;
};