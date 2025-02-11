#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image.h"
#include "stb_image_write.h"

using namespace std;
using namespace glm;

const int WIDTH = 512;
const int HEIGHT = 512;

// Structure to hold texture data
struct Texture {
    unsigned char* data;
    int width, height, channels;
};

// Load image using stb_image
Texture loadTexture(const char* filename) {
    Texture tex;
    tex.data = stbi_load(filename, &tex.width, &tex.height, &tex.channels, 3); // Force 3 channels (RGB)
    if (!tex.data) {
        cerr << "Failed to load texture: " << filename << endl;
        exit(1);
    }
    return tex;
}

// Sample a texture (simulating GLSL `texture(iChannelX, uv)`)
vec3 texture(const Texture& tex, vec2 uv) {
    uv = clamp(uv, vec2(0.0f), vec2(1.0f)); // Keep UV in range [0,1]
    int x = int(uv.x * tex.width) % tex.width;
    int y = int(uv.y * tex.height) % tex.height;

    int index = (y * tex.width + x) * 3; // RGB channels
    return vec3(tex.data[index] / 255.0f, tex.data[index + 1] / 255.0f, tex.data[index + 2] / 255.0f);
}



// Simulated `mainImage`
vec4 mainImage(vec2 fragCoord, const Texture& tex1, const Texture& tex2) {
    vec2 uv = (fragCoord - vec2(WIDTH, HEIGHT) * 0.5f) / float(HEIGHT) * 2.0f;
    uv = (uv + 1.0f) * 0.5f; // Convert to [0,1] for texture lookup

    // Apply wave distortion to first texture
    float wave = sin(uv.y * 10.0f) * 0.05f;
    vec2 uv1 = uv;
    uv1.x += wave;

    // Apply ripple distortion to second texture
    float ripple = cos(uv.x * 40.0f) * 0.03f;
    vec2 uv2 = uv;
    uv2.y += ripple;

    // Fetch colors from both textures
    vec3 color1 = texture(tex1, uv1);
    vec3 color2 = texture(tex2, uv2);

    // Blend the two textures (mix function)
    vec3 finalColor = mix(color1, color2, 0.5f); // 50% blend

    return vec4(finalColor, 1.0f);
}

int main() {
    // Load two textures
    Texture tex1 = loadTexture("img/iouri.png");
    Texture tex2 = loadTexture("img/moon.png");

    vector<unsigned char> image(WIDTH * HEIGHT * 3);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            vec4 color = mainImage(vec2(x, y), tex1, tex2);

            int index = (y * WIDTH + x) * 3;
            image[index] = static_cast<unsigned char>(color.r * 255);
            image[index + 1] = static_cast<unsigned char>(color.g * 255);
            image[index + 2] = static_cast<unsigned char>(color.b * 255);
        }
    }

    // Save the output image
    stbi_write_png("shader_output-multitex.png", WIDTH, HEIGHT, 3, image.data(), WIDTH * 3);

    // Free texture memory
    stbi_image_free(tex1.data);
    stbi_image_free(tex2.data);

    cout << "Image generated: shader_output-multitex.png" << endl;
    return 0;
}