#include "menu.hpp"

void Menu::process_input() {
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

void Menu::draw() const {
    switch(state) {
    case STATE::INACTIVE:
        break; // if the menu is inactive do nothing
    case STATE::ACTIVE:
        for(size_t i = 0; i<entries.size(); ++i) {
            const auto [rect_x, rect_y, text_x, text_y] = entry_coords(i);
            const auto [rect_color, text_color] = entry_colors(i);

            DrawRectangle(rect_x, rect_y, rect_width, rect_height, rect_color);
            DrawText(entries[i].c_str(), text_x, text_y, font_size, text_color);
        }
    }
}

std::string Menu::current_pick() const {
    return entries[picked_index];
}

void Menu::next_pick() {
    picked_index++;
    picked_index%=entries.size();
}
void Menu::prev_pick() {
    picked_index--;
    picked_index+=entries.size();
    picked_index%=entries.size();
}

int Menu::text_width(const std::string& s) {
    return MeasureText(s.c_str(), font_size);
}


std::tuple<int, int, int, int> Menu::entry_coords(int entry_index) const {
    const int i = entry_index;

    int rect_x = initial_x;
    int rect_y = initial_y + (i*rect_height) + ((i-1)*inter_rect_vpad);

    int text_x = rect_x + text_hpad;
    int text_y = rect_y + text_vpad;

    return std::tuple<int, int, int, int>({rect_x, rect_y, text_x, text_y});
}

std::tuple<Color, Color> Menu::entry_colors(int entry_index) const {
    const bool is_current_pick = entry_index==picked_index;
    return std::tuple<Color, Color>({
            (is_current_pick?rect_picked_color:rect_unpicked_color),
                (is_current_pick?text_picked_color:text_unpicked_color),
                });
}
