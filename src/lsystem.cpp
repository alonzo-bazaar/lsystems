#include "lsystem.hpp"

Texture Lsystem::stripes_texture(const std::vector<Color>& colors,
                                 const unsigned int stripe) {
    // code partially readapted from GenImageChecked in rtextures.c
    auto nsteps = colors.size();
    const int width = nsteps*stripe;
    const int height = nsteps*stripe;

    Color *pixels = (Color *)RL_MALLOC(width*height*sizeof(Color));

    for (int y = 0; y < nsteps; y++) {
        for (int x = 0; x < nsteps; x++) {
            // color stripexstripe area starting at y*stripe + x*stripe
            // (we're generating a vertical gradient)
            int scaled_y = y*stripe;
            int scaled_x = x*stripe;
            for(unsigned int i = 0; i<stripe; i++)
                for(unsigned int j = 0; j<stripe; j++)
                    pixels[(scaled_y+i)*width + (scaled_x+j)] = colors[y];
        }
    }
    Image image = {
        .data = pixels,
        .width = width,
        .height = height,
        .mipmaps = 1,
        .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8
    };
    Texture texture = LoadTextureFromImage(image);
    UnloadImage(image);
    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
    return texture;
}

Texture Lsystem::gradient_texture(const Color& start,
                                  const Color& end,
                                  const int sidelen) {
    return vertical_gradient(sidelen, start, end);
}


Lsystem Lsystem::from_parsed_tree(const ParsedTree& parsed_tree,
                                  const Color& start,
                                  const Color& end) {
    std::cout<<parsed_tree.rewrite_times<<std::endl;
    Image image = GenImageGradientLinear(parsed_tree.rewrite_times*2,
                                         parsed_tree.rewrite_times*2,
                                         0,
                                         start, end);
    Texture texture = LoadTextureFromImage(image);
    GenTextureMipmaps(&texture);
    SetTextureFilter(texture, TEXTURE_FILTER_TRILINEAR);
    UnloadImage(image);
    return Lsystem(parsed_tree.thickness_table,
                   parsed_tree.texcoords_table,
                   texture,
                   parsed_tree.rewrite_times,
                   parsed_tree.axiom,
                   parsed_tree.rewrite_rules);
}
Lsystem Lsystem::from_parsed_tree(const ParsedTree& parsed_tree,
                                  const std::vector<Color>& colors) {
    // unlikely to work
    assert(0 && "TODO");
    return Lsystem(parsed_tree.thickness_table,
                   parsed_tree.texcoords_table,
                   stripes_texture(colors, 4),
                   parsed_tree.rewrite_times,
                   parsed_tree.axiom,
                   parsed_tree.rewrite_rules);
}

TreeModel Lsystem::gen_model(const unsigned int seed) const {
    const std::vector<instruction> turtle_instructions =
        rewrite_times(times_to_rewrite, axiom, rules_for_rewriting);
    Turtle turtle (thickness_table, texcoords_table);	
    for(auto ttc : texcoords_table)
        std::cout<<ttc[0] << ", " << ttc[1] << "\n";
    TreeModel tmodel = turtle.follow_instruction_vector(turtle_instructions);

    tmodel.model.materials[0].maps[MATERIAL_MAP_ALBEDO].texture = texture;

    return tmodel;
}

TreeModel Lsystem::gen_model(const unsigned int seed, const Shader shader) const {
    TreeModel tmodel = gen_model(seed);
    tmodel.model.materials[0].shader = shader;
    return tmodel;
}

Lsystem basic_tree_lsystem() {
    constexpr float stride = 0.3f;
    constexpr float angle = deg_to_rad(22.5);
    
    const std::map<char, RewriteTarget> rewrite_rules = {
        RWP('A', [stride, angle](const std::vector<float>& ignored) {
            (void)ignored;
            return std::vector<instruction>
                {{'[',{}},
                 {'&',{angle}},
                 {'F',{stride}},
                 {'L',{}},
                 {'!',{}},
                 {'A',{}},
                 {']',{}},

                 {'/',{angle*5}},
                 {'\'',{}},

                 {'[',{}},
                 {'&',{angle}},
                 {'F',{stride}},
                 {'L',{}},
                 {'!',{}},
                 {'A',{}},
                 {']',{}},

                 {'/',{7*angle}},
                 {'\'',{}},

                 {'[',{}},
                 {'&',{angle}},
                 {'F',{stride}},
                 {'L',{}},
                 {'!',{}},
                 {'A',{}},
                 {']',{}}};
        }),
        RWP('F',
            {{0.9,
              [stride, angle](const std::vector<float>&ignored) {
                  (void)ignored;
                  return std::vector<instruction>
                      {{'S', {}},
                       {'/', {4*angle}},
                       {'F', {2*stride}}};
              }},
             {0.1,
              [stride, angle](const std::vector<float>&ignored) {
                  (void)ignored;
                  return std::vector<instruction>
                      {{'S', {}},
                       {'/', {5*angle}},
                       {'F', {stride}}};
              }}}),
        RWP('S',
            [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                    {{'F', {stride}},
                     {'L', {}}};
            }),
        RWP('L',
            [stride, angle](const std::vector<float>&ignored) {
                (void)ignored;
                return std::vector<instruction>
                    {{'[',{}},
                     {'\'',{}},
                     {'\'',{}},
                     {'\'',{}},
                     {'^', {2*angle}},
                     {'{',{}},
                     {'-',{angle}},
                     {'f',{stride}},
                     {'+',{angle}},
                     {'f',{stride}},
                     {'+',{angle}},
                     {'f',{stride}},
                     {'-',{angle}},
                     {'|',{}},
                     {'-',{angle}},
                     {'f',{stride}},
                     {'+',{angle}},
                     {'f',{stride}},
                     {'+',{angle}},
                     {'f',{stride}},
                     {'}',{}},
                     {']',{}}};
            })};

    return Lsystem(map_range<float>(0.06f, 0.015f, 7),
                   map_range(std::array{0.05f, 0.05f}, std::array{0.95f, 0.95f}, 7),
                   Lsystem::gradient_texture(BROWN, LIME, 20),
                   7,
                   {{'A', {}}},
                   rewrite_rules);
}
