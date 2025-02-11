#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;
using namespace glm;

const int WIDTH = 512;
const int HEIGHT = 512;

// Simple hash function (pseudo-random)
float hash(float n) {
    return fract(sin(n * 0.1346f) * 43758.5453123f);
}

// Simulated `mainImage` function
vec4 mainImage(vec2 fragCoord) {
    // Normalize coordinates to the range [-1,1]
    vec2 uv = (fragCoord - vec2(WIDTH, HEIGHT) * 0.5f) / float(HEIGHT) * 2.0f;
    
    // Generate a simple sinusoidal gradient
    float colorVal = 0.5f + 0.5f * sin(uv.x * 10.0f + uv.y * 5.0f);
    
    return vec4(vec3(colorVal), 1.0f);
}

int main() {
    vector<unsigned char> image(WIDTH * HEIGHT * 3);

    // Iterate over each pixel and compute the color
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            vec4 color = mainImage(vec2(x, y));

            int index = (y * WIDTH + x) * 3;
            image[index] = static_cast<unsigned char>(color.r * 255);
            image[index + 1] = static_cast<unsigned char>(color.g * 255);
            image[index + 2] = static_cast<unsigned char>(color.b * 255);
        }
    }

    // Save the image as a PNG file
    stbi_write_png("img/shader_output.png", WIDTH, HEIGHT, 3, image.data(), WIDTH * 3);

    cout << "Image generated: shader_output.png" << endl;
    return 0;
}
