#pragma once

// logic around the intializing and the string picker
#include<initializer_list>
#include<string>
#include<vector>
#include<tuple>
#include<cassert>

// logic around drawing the menu on the screen
#include "raylib.h"

#include "utils.hpp"

class Menu {
public:
    template<typename T>
    Menu(const T& entries)
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
            else if(IsKeyPressed(KEY_ENTER) || IsKeyPressed(KEY_M))
                state = STATE::INACTIVE;
            break;
        }
    }
    void draw() const {
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
    std::string current_pick() const {
        return entries[picked_index];
    }

private:
    enum STATE {INACTIVE, ACTIVE};
    STATE state;
    std::vector<std::string> entries;
    size_t picked_index;

    // buncha hardcoded hyperparameters
    static constexpr int initial_x = 10;
    static constexpr int initial_y = 10;
    static constexpr int inter_rect_vpad = 5;
    static constexpr int rect_height = 30;
    // rect_width depends on entries
    // (it is the maximum width of any string in entries, plus padding)
    // since entries is variable so must rect_width be
    int rect_width;
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
    template<typename T>
    static int max_text_width(const T& sl)
        requires goddamn_container<T, std::string>
    {
        int res = 0;
        for(const auto& s : sl) {
            int curr = text_width(s);
            res = curr>res?curr:res;
        }
        return res;
    }

    std::tuple<int, int, int, int> entry_coords(int entry_index) const {
        const int i = entry_index;

        int rect_x = initial_x;
        int rect_y = initial_y + (i*rect_height) + ((i-1)*inter_rect_vpad);

        int text_x = rect_x + text_hpad;
        int text_y = rect_y + text_vpad;

        return std::tuple<int, int, int, int>({rect_x, rect_y, text_x, text_y});
    }

    std::tuple<Color, Color> entry_colors(int entry_index) const {
        const bool is_current_pick = entry_index==picked_index;
        return std::tuple<Color, Color>({
                (is_current_pick?rect_picked_color:rect_unpicked_color),
                (is_current_pick?text_picked_color:text_unpicked_color),
            });
    }
};
