#pragma once

#include "globals.h"

#include "Render/RenderQueue.h"
#include "Math.h"
#include "R.h"

namespace fun {
    class WindowManager {
    public:

        static const sf::Vector2u INIT_SCREEN_SIZE;
        static const sf::Vector2f INIT_VIEW_SIZE;
        static const sf::Vector2f INIT_VIEW_ORIGIN;

        struct WindowData {
            explicit WindowData(const std::string&);

            sf::RenderWindow window;

            // render queues
            RenderQueue world_queue;
            RenderQueue ui_queue;

            // render views
            sf::View world_view;
            sf::View ui_view;
            sf::View final_view;

            // render textures
            sf::RenderTexture world_buffer;
            sf::RenderTexture ui_buffer;
            sf::RenderTexture final_buffer;

            // render sprites
            sf::Sprite world_render;
            sf::Sprite ui_render;
            sf::Sprite final_render;

            bool is_focused;
            float zoom;
            sf::Vector2u resolution;

            void AddWorld(const sf::Drawable&, int);
            void AddUI(const sf::Drawable&, int);

            void PollEvents();
            void Display(const sf::Color&);
        };

        static WindowData* main_window;

        static void Init(const std::string&);

    private:
    };
}