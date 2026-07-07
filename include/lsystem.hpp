#pragma once
#ifndef LSYSTEMS_LSYSTEM_HPP_
#define LSYSTEMS_LSYSTEM_HPP_

#include "lsystem_json.hpp" // for ParsedTree struct
#include "turtle.hpp"       // for Turtle class
#include "rewrite.hpp"      // for rewrite functions
#include "raylib.h"         // for textures and colors and models the like

struct Lsystem {
    typedef std::vector<instruction> iv;
    const std::vector<float> thickness_table;
    // color table made out of texcoords table and a texture
    const std::vector<std::array<float ,2>> texcoords_table;
    Texture texture;
    // had to use a different names than rewrite_times since it clashed with the
    // global function rewrite_times
    // rewrite_rules was renamed to rules_for_rewriting to better fit with the
    // renamed times_to_rewrite
    const unsigned int times_to_rewrite;
    const iv axiom;
    const std::map<char, RewriteTarget> rules_for_rewriting;

    Lsystem(const std::vector<float>& thickness_table,
            const std::vector<std::array<float, 2>>& texcoords_table,
            const Texture texture,
            const unsigned int times_to_rewrite,
            const iv& axiom,
            const std::map<char, RewriteTarget>& rules_for_rewriting)
        :thickness_table(thickness_table),
         texcoords_table(texcoords_table),
         texture(texture),
         times_to_rewrite(times_to_rewrite),
         axiom(axiom),
         rules_for_rewriting(rules_for_rewriting){};

    static Texture stripes_texture(const std::vector<Color>& colors,
                                   const unsigned int stripe);
    static Texture gradient_texture(const Color& start,
                                    const Color& end,
                                    const int sidelen);

    static Lsystem from_parsed_tree(const ParsedTree& parsed_tree,
                                    const Color& start,
                                    const Color& end);
    static Lsystem from_parsed_tree(const ParsedTree& parsed_tree,
                                    const std::vector<Color>& colors);

    Model gen_model(const unsigned int seed) const;
    Model gen_model(const unsigned int seed, const Shader shader) const;
};

Lsystem basic_tree_lsystem();

#endif // LSYSTEMS_LSYSTEM_HPP_
