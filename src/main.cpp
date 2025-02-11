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

// Simulate GLSL `texture(iChannel0, uv)`
vec3 texture(const Texture& tex, vec2 uv) {
    uv = clamp(uv, vec2(0.0f), vec2(1.0f)); // Ensure UV is in range [0,1]

    int x = int(uv.x * tex.width) % tex.width;
    int y = int(uv.y * tex.height) % tex.height;

    int index = (y * tex.width + x) * 3; // RGB channels
    return vec3(tex.data[index] / 255.0f, tex.data[index + 1] / 255.0f, tex.data[index + 2] / 255.0f);
}

// Simulated `mainImage`
vec4 mainImage(vec2 fragCoord, const Texture& tex) {
    vec2 uv = (fragCoord - vec2(WIDTH, HEIGHT) * 0.5f) / float(HEIGHT) * 2.0f;
    uv = (uv + 1.0f) * 0.5f; // Convert to [0,1] for texture lookup

    // Apply sinusoidal distortion
    float wave = sin(uv.y * 15.0f) * 0.05f;
    uv.x += wave;
    
    float ripple = cos(uv.x * 20.0f) * 0.03f;
    uv.y += ripple;

    vec3 color = texture(tex, uv); // Fetch distorted texture
    return vec4(color, 1.0f);
}

int main() {
    // Load texture
    Texture tex = loadTexture("img/iouri.png");

    vector<unsigned char> image(WIDTH * HEIGHT * 3);

    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            vec4 color = mainImage(vec2(x, y), tex);

            int index = (y * WIDTH + x) * 3;
            image[index] = static_cast<unsigned char>(color.r * 255);
            image[index + 1] = static_cast<unsigned char>(color.g * 255);
            image[index + 2] = static_cast<unsigned char>(color.b * 255);
        }
    }

    // Save the output image
    stbi_write_png("shader_output-2.png", WIDTH, HEIGHT, 3, image.data(), WIDTH * 3);

    // Free texture memory
    stbi_image_free(tex.data);

    cout << "Image generated: shader_output-2.png" << endl;
    return 0;
}
