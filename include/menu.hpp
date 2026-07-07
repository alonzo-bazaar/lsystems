#pragma once
#ifndef LSYSTEMS_MENU_HPP_
#define LSYSTEMS_MENU_HPP_

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

    void process_input();
    void draw() const;
    std::string current_pick() const;

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

    void next_pick();
    void prev_pick();
    static int text_width(const std::string& s);

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

    std::tuple<int, int, int, int> entry_coords(int entry_index) const;
    std::tuple<Color, Color> entry_colors(int entry_index) const;
};

#endif // LSYSTEMS_MENU_HPP_
