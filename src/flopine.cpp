// Original GLSL code by Flopine, see here : https://www.shadertoy.com/view/tttBRX

// Thanks to wsmind, leon, XT95, lsdlive, lamogui, 
// Coyhot, Alkama,YX, NuSan, slerpy and wwrighter for teaching me

// Thanks LJ for giving me the spark :3

// Thanks to the Cookie Collective, which build a cozy and safe environment for me 
// and other to sprout :)  https://twitter.com/CookieDemoparty

// C++ experimental translation

#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb_image_write.h"

using namespace std;
using namespace glm;

const int WIDTH = 1280;
const int HEIGHT = 720;
const int ITER = 80;
const float EPSILON = 0.00001f;

#define PI 3.14159265359f
#define TAU 6.28318530718f

float iTime = 1.f;

// Rotation matrices
mat2 rot(float a) {
    float c = cos(a), s = sin(a);
    return mat2(c, s, -s, c);
}

// crep for a single float value (emulates the GLSL swizzle)
void crepf(float& p, float c, float l) {
    p -= c * glm::clamp(glm::round(p / c), -l, l);
}

// crep for a vec2 (emulates the GLSL swizzle)
void crep2(vec2& p, float c, float l) {
    p -= c * glm::clamp(glm::round(p / c), -l, l);
}

float dt(float sp, float off, float time) {
    return glm::fract((time + off) * sp);
}

float bouncy(float sp, float off, float time) {
    return sqrt(sin(dt(sp, off, time) * PI));
}

// "Object" definition
struct Object {
    float d;     // SDF value
    vec3 cs;     // Dark color
    vec3 cl;     // Lighting color
};

Object minObj(Object a, Object b) {
    return (a.d < b.d) ? a : b;
}

float stmin(float a, float b, float k, float n) {
    float st = k / n;
    float u = b - k;
    return glm::min(glm::min(a, b), 0.5f * (u + a + glm::abs(glm::mod(u - a + st, 2.0f * st) - st)));
}

void mo(vec2& p, vec2 d) {
    p = glm::abs(p) - d;
    if (p.y > p.x) {
        std::swap(p.x, p.y);
    }
}

float boxSDF(vec3 p, vec3 c) {
    vec3 q = glm::abs(p) - c;
    return glm::min(0.0f, glm::max(q.x, glm::max(q.y, q.z))) + length(glm::max(q, vec3(0.0f)));
}

float scSDF(vec3 p, float d) {
    p = glm::abs(p);
    p = glm::max(p, vec3(p.y, p.z, p.x)); // p = max(p, p.yzx);
    return glm::min(p.x, glm::min(p.y, p.z)) - d;
}

Object prim1(vec3 p, float time) {
    p.x = glm::abs(p.x) - 3.0f;
    
    float per = 0.9f;
    float id = round(p.y / per);
    
    vec2 rotatedXZ = rot(sin(fract(time * 0.8f + id * 1.2f) * TAU)) * vec2(p.x, p.z);
    p.x = rotatedXZ.x;
    p.z = rotatedXZ.y;
    
    crepf(p.y, per, 4.0f);
    vec2 p_xz = vec2(p.x, p.z);
    mo(p_xz, vec2(0.3f, 0.3f));
    p.x = p_xz.x;
    p.z = p_xz.y;
    
    p.x += bouncy(2.0f, 0.0f, time) * 0.8f;
    
    float pd = boxSDF(p, vec3(1.5f, 0.2f, 0.2f));
    
    return {pd, vec3(0.5f, 0.0f, 0.0f), vec3(1.0f, 0.5f, 0.9f)};
}

Object prim2(vec3 p, float time) {
    p.y = glm::abs(p.y) - 6.0f;
    p.z = glm::abs(p.z) - 4.0f;

    vec2 p_xz = vec2(p.x, p.z);
    mo(p_xz, vec2(1.0f, 1.0f));
    p.x = p_xz.x;
    p.z = p_xz.y;
    
    vec3 pp = p;
    vec2 p_yz = vec2(p.y, p.z);
    mo(p_yz, vec2(0.5f, 0.5f));
    p.y = p_yz.x;
    p.z = p_yz.y;
    p.y -= 0.5f;

    float p2d = glm::max(-scSDF(p, 0.7f), boxSDF(p, vec3(1.0f)));

    p = pp;
    p2d = glm::min(p2d, glm::max(boxSDF(p, vec3(bouncy(2.0f, 0.0f, time) * 4.0f)), scSDF(p, 0.2f)));

    return {p2d, vec3(0.2f, 0.2f, 0.2f), vec3(1.0f, 1.0f, 1.0f)};
}

Object prim3(vec3 p, float time) {
    p.z = glm::abs(p.z) - 9.0f;

    float per = 0.8f;
    vec2 p_xy = vec2(p.x, p.y);
    vec2 id = glm::round(p_xy / per) - 0.5f;
    p.x = p_xy.x;
    p.y = p_xy.y;

    float height = 1.0f * bouncy(2.0f, glm::sin(glm::length(id * 0.05f)), time);

    float p3d = boxSDF(p, vec3(2.0f, 2.0f, 0.2f));

    p_xy = vec2(p.x, p.y);
    crep2(p_xy, per, 2.0f);
    p.x = p_xy.x;
    p.y = p_xy.y;

    p3d = stmin(p3d, boxSDF(p + vec3(0.0f, 0.0f, height * 0.9f), vec3(0.15f, 0.15f, height)), 0.2f, 3.0f);

    return {p3d, vec3(0.1f, 0.7f, 0.0f), vec3(1.0f, 0.9f, 0.0f)};
}

Object prim4(vec3 p, float time) {
    p.y = glm::abs(p.y) - 5.0f;

    vec2 p_xz = vec2(p.x, p.z);
    mo(p_xz, vec2(1.0f, 1.0f));
    p.x = p_xz.x;
    p.z = p_xz.y;

    float scale = 1.5f;
    p *= scale;

    float per = 2.0f * bouncy(0.5f, 0.0f, time);
    p_xz = vec2(p.x, p.z);
    crep2(p_xz, per, 2.0f);
    p.x = p_xz.x;
    p.z = p_xz.y;

    float p4d = glm::max(boxSDF(p, vec3(0.9f, 0.9f, 0.9f)), scSDF(p, 0.25f));

    return {p4d / scale, vec3(0.1f, 0.2f, 0.4f), vec3(0.1f, 0.8f, 0.9f)};
}

float squared(vec3 p, float s) {
    vec2 p_zy = vec2(p.z, p.y);
    mo(p_zy, vec2(s, s));
    p.z = p_zy.x;
    p.y = p_zy.y;
    return boxSDF(p, vec3(0.2f, 10.0f, 0.2f));
}

Object prim5(vec3 p, float time) {
    p.x = glm::abs(p.x) - 8.0f;
    
    float id = glm::round(p.z / 7.0f);
    crepf(p.z, 7.0f, 2.0f);

    float scarce = 3.0f;
    float p5d = 1e10f;

    for (int i = 0; i < 4; i++) {
        p.x += bouncy(1.0f, id * 0.9f, time) * 0.6f;
        p5d = glm::min(p5d, squared(p, scarce));
        vec2 rotatedYZ = rot(PI / 4.0f) * vec2(p.y, p.z);
        p.y = rotatedYZ.x;
        p.z = rotatedYZ.y;
        scarce -= 1.0f;
    }

    return {p5d, vec3(0.5f, 0.2f, 0.1f), vec3(1.0f, 0.9f, 0.1f)};
}

Object scene(vec3 p, float time) {
    // Apply rotation to yz-plane
    vec2 rotatedYZ = rot(-atan(1.0f / sqrt(2.0f))) * vec2(p.y, p.z);
    p.y = rotatedYZ.x;
    p.z = rotatedYZ.y;

    // Apply rotation to xz-plane
    vec2 rotatedXZ = rot(PI / 4.0f) * vec2(p.x, p.z);
    p.x = rotatedXZ.x;
    p.z = rotatedXZ.y;

    // Combine all primitives using minObj()
    Object scene = prim1(p, time);
    scene = minObj(scene, prim2(p, time));
    scene = minObj(scene, prim3(p, time));
    scene = minObj(scene, prim4(p, time));
    scene = minObj(scene, prim5(p, time));

    return scene;
}

vec3 getNormal(vec3 p, float time) {
    vec3 eps_x = vec3(0.001f, 0.0f, 0.0f);
    vec3 eps_y = vec3(0.0f, 0.001f, 0.0f);
    vec3 eps_z = vec3(0.0f, 0.0f, 0.001f);

    return normalize(vec3(
        scene(p + eps_x, time).d - scene(p - eps_x, time).d,
        scene(p + eps_y, time).d - scene(p - eps_y, time).d,
        scene(p + eps_z, time).d - scene(p - eps_z, time).d
    ));
}

// Generates the final image
void renderImage(vector<unsigned char>& image, float time) {
    for (int y = 0; y < HEIGHT; y++) {
        for (int x = 0; x < WIDTH; x++) {
            // Original mainImage() stars here
            vec2 uv = ((vec2(x, y) - vec2(WIDTH, HEIGHT) * 0.5f) / float(HEIGHT)) * vec2(-2.0f, 2.0f);
            // vec2 uv = (vec2(x, y) - vec2(WIDTH, HEIGHT) * 0.5f) / float(glm::min(WIDTH, HEIGHT));
            // vec2 uv = (vec2(x / float(WIDTH), y / float(HEIGHT)) - 0.5f) * 2.0f;

            vec3 ro = vec3(uv * 5.0f, -30.0f);
            vec3 rd = vec3(0.0f, 0.0f, 1.0f);
            vec3 p = ro;

            // vec3 col = vec3(smoothstep(0.3f, 0.65f, length(uv * 0.5f)));
            vec3 col = vec3(0.0f); // Bg color

            vec3 l = normalize(vec3(1.0f, 1.4f, -2.0f));

            Object O;
            bool hit = false;

            for (int i = 0; i < ITER; i++) {
                O = scene(p, time);
                if (O.d < EPSILON) {
                    hit = true;
                    break;
                }
                p += O.d * rd;
            }

            if (hit) {
                vec3 n = getNormal(p, time);
                float light = glm::max(dot(n, l), 0.0f);
                col = glm::mix(O.cs, O.cl, light);
            }

            // Store pixel color
            int index = (y * WIDTH + x) * 3;
            image[index] = static_cast<unsigned char>(sqrt(col.r) * 255);
            image[index + 1] = static_cast<unsigned char>(sqrt(col.g) * 255);
            image[index + 2] = static_cast<unsigned char>(sqrt(col.b) * 255);
        }
    }
}

// int main() {
//     vector<unsigned char> image(WIDTH * HEIGHT * 3);
//     renderImage(image, 1.0);

//     stbi_write_png("shader_flopine.png", WIDTH, HEIGHT, 3, image.data(), WIDTH * 3);
//     cout << "Image generated: shader_flopine.png" << endl;

//     return 0;
// }

int main() {
    vector<unsigned char> image(WIDTH * HEIGHT * 3);

    for (int frame = 0; frame < 120; frame++) {
        float time = frame / 60.0f;

        renderImage(image, time);

        char filename[50];
        snprintf(filename, sizeof(filename), "img/render/frame_%04d.png", frame);

        stbi_write_png(filename, WIDTH, HEIGHT, 3, image.data(), WIDTH * 3);
        cout << "Image generated: " << filename << endl;
    }

    return 0;
}

