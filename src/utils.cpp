#include "utils.hpp"

def_range_mat(float, 2)
def_range_mat(float, 3)
def_range_mat(unsigned char, 4)

#ifndef UTILS_STANDALONE
template<>
std::vector<Color> map_range<Color>(Color start, Color end, size_t n) {
    auto floats = map_range(std::array{start.r, start.g, start.b, start.a},
                            std::array{end.r, end.g, end.b, end.a},
                            n);

    std::vector<Color> res(n);
    for(size_t i = 0; i<n; ++i) {
        res[i].r = floats[i][0];
        res[i].g = floats[i][1];
        res[i].b = floats[i][2];
        res[i].a = floats[i][3];
    }
    return res;
}
#endif

std::vector<size_t>iota(size_t to, size_t from=0) {
    assert(from <= to);
    std::vector<size_t> res;
    res.reserve(to-from);
    for(size_t i = from; i<to; ++i)
        res.push_back(i);
    return res;
}

#ifndef UTILS_STANDALONE
Texture load_and_free_image(Image image) {
    Texture t = LoadTextureFromImage(image);
    UnloadImage(image);
    return t;
}

Texture vertical_gradient(const int length, const Color& start, const Color& end) {
    return load_and_free_image(GenImageGradientLinear(10, 10, 0, start, end));
}

Texture flat_color(const int sidelen, const Color& color) {
    return load_and_free_image(GenImageColor(10, 10, color));
}
#endif
