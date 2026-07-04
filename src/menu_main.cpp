#include<initializer_list>
#include<string>
#include<vector>
#include<tuple>
#include<cassert>

#include "raylib.h"

// menu is keyboard driven for ease of programming
// (although please note: this message was written by a vim user)
class Menu {
public:
    Menu(const std::initializer_list<std::string>& entries)
        :entries(entries),
         picked_index(0),
         state(STATE::INACTIVE),
         rect_width(max_text_width(entries) + 2*text_hpad)
    {
        assert(entries.size() > 0);
    }

    void process_input() {
        switch(state) {
        case STATE::INACTIVE:
            if(IsKeyPressed(KEY_M))
                state = STATE::ACTIVE;
            break;
        case STATE::ACTIVE:
            if(IsKeyPressed(KEY_J))
                next_pick();
            else if(IsKeyPressed(KEY_K))
                prev_pick();
            else if(IsKeyPressed(KEY_ENTER))
                state = STATE::INACTIVE;
            break;
        }
    }
    void draw() {
        switch(state) {
        case STATE::INACTIVE:
            break; // if the menu is inactive do nothing
        case STATE::ACTIVE: {
            for(size_t i = 0; i<entries.size(); ++i) {
                const auto [rect_x, rect_y, text_x, text_y] = entry_coords(i);
                const auto [rect_color, text_color] = entry_colors(i);

                DrawRectangle(rect_x, rect_y, rect_width, rect_height, rect_color);
                DrawText(entries[i].c_str(), text_x, text_y, font_size, text_color);
            }
        }
        }
    }
    std::string current_pick() {
        return entries[picked_index];
    }

private:
    enum STATE {INACTIVE, ACTIVE};
    STATE state;
    const std::vector<std::string> entries;
    size_t picked_index;

    // buncha hardcoded hyperparameters
    static constexpr int initial_x = 10;
    static constexpr int initial_y = 10;
    static constexpr int inter_rect_vpad = 5;
    static constexpr int rect_height = 30;
    const int rect_width;
    // computed at construction time,
    // since it needs the string intializer list
    // cannot therefore be constexpr (constexpr = computable at compile time);
    static constexpr int font_size = 20;
    static constexpr int text_vpad = (rect_height - font_size)/2;
    static constexpr int text_hpad = 15;

    static constexpr Color rect_unpicked_color = LIGHTGRAY;
    static constexpr Color rect_picked_color = DARKGRAY;

    static constexpr Color text_unpicked_color = BLACK;
    static constexpr Color text_picked_color = WHITE;

    void next_pick() {
        picked_index++;
        picked_index%=entries.size();
    }
    void prev_pick() {
        picked_index--;
        picked_index+=entries.size();
        picked_index%=entries.size();
    }

    static int text_width(const std::string& s) {
        return MeasureText(s.c_str(), font_size);
    }
    static int max_text_width(const std::initializer_list<std::string>& sl) {
        int res = 0;
        for(const auto& s : sl) {
            int curr = text_width(s);
            res = curr>res?curr:res;
        }
        return res;
    }

    std::tuple<int, int, int, int> entry_coords(int entry_index) {
        const int i = entry_index;

        int rect_x = initial_x;
        int rect_y = initial_y + (i*rect_height) + ((i-1)*inter_rect_vpad);

        int text_x = rect_x + text_hpad;
        int text_y = rect_y + text_vpad;

        return std::tuple<int, int, int, int>({rect_x, rect_y, text_x, text_y});
    }

    std::tuple<Color, Color> entry_colors(int entry_index) {
        const bool is_current_pick = entry_index==picked_index;
        return std::tuple<Color, Color>({
                (is_current_pick?rect_picked_color:rect_unpicked_color),
                (is_current_pick?text_picked_color:text_unpicked_color),
            });
    }
};

int main() {
    SetConfigFlags(FLAG_MSAA_4X_HINT);
    InitWindow(1280, 960, "L Systems");
    DisableCursor();
    int target_fps = 60;

    Menu menu = Menu({
            "porcoddio",
            "diocane",
            "dio bestia",
            "puttana la madonna di diocane",
            "madonna è mobile, qual piuma al vento",
        });

    while (!WindowShouldClose()) {
        BeginDrawing(); {
            ClearBackground(WHITE);
            menu.process_input();
            menu.draw();

            DrawText(menu.current_pick().c_str(), 300, 300, 30, BLACK);
        } EndDrawing();
    }
    
    CloseWindow();
    return 0;
}
