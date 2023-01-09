#include <iostream>
#include <cmath>
#include <algorithm>
#include "tgaimage.h"
using namespace std;

// dimenzije slike
const int width  = 512;
const int height = 512;

// definirajmo boje
const TGAColor white = TGAColor(255, 255, 255, 255);
const TGAColor red   = TGAColor(255, 0, 0, 255);
const TGAColor blue  = TGAColor(0, 0, 255, 255);
const TGAColor green  = TGAColor(0, 255, 0, 255);

typedef float Vec3[3];
typedef float Vec2[2];

//vj9
float line(float x0, float y0, float x1, float y1, float x, float y)
{
    return (y0 - y1) * x + (x1 - x0) * y + x0 * y1 - x1 * y0;
}

void set_color(int x, int y, TGAImage &image, TGAColor color, bool invert = false)
{
    image.set(x, y, color);
}

float help(const Vec3 &v1, const Vec3 &v2, const Vec3 &v3)
{
    return (v3[0] - v1[0]) * (v2[1] - v1[1]) - (v3[1] - v1[1]) * (v2[0] - v1[0]);
}

float pov(float x1, float y1, float x2, float y2, float x3, float y3)
{
    return abs(( x1 * (y2 - y3) + x2 * (y3 - y1) + x3 * (y1 - y2)) / 2.0 );
}

bool in_triangle(float x1, float y1, float x2, float y2, float x3, float y3, float x, float y)
{
    float P = pov(x1, y1, x2, y2, x3, y3);
    float P1 = pov(x, y, x2, y2, x3, y3);
    float P2 = pov(x1, y1, x, y, x3, y3);
    float P3 = pov(x1, y1, x2, y2, x, y);
    return (P == P1 + P2 + P3);
}

void draw_triangle_2d(TGAImage& image, float x0, float y0, float x1, float y1, float x2, float y2, TGAColor color)
{
    long max_x = ceil  (max (max(x0, x1), x2));
    long min_x = floor (min (min(x0, x1), x2));
    long max_y = ceil  (max (max(y0, y1), y2));
    long min_y = floor (min (min(y0, y1), y2));

    for (int j = min_y; j <= max_y ; ++j)
        for (int i = min_x; i <= max_x; ++i)
            if (in_triangle(x0, y0, x1, y1, x2, y2, i, j))
                image.set(i, j, color);
}

void draw_triangle_2d_gouraurd(TGAImage& image, float x0, float y0, const TGAColor& c0, float x1, float y1, const TGAColor& c1, float x2, float y2, const TGAColor& c2)
{
    int max_x = ceil  (max (max(x0, x1), x2));
    int min_x = floor (min (min(x0, x1), x2));
    int max_y = ceil  (max (max(y0, y1), y2));
    int min_y = floor (min (min(y0, y1), y2));

    TGAColor color;

    for (int j = min_y; j <= max_y; ++j)
    {
        for (int i = min_x; i <= max_x; ++i)
        {
            float first = line(x1, y1, x2, y2, i, j) / line(x1, y1, x2, y2, x0, y0);
            float second = line(x2, y2, x0, y0, i, j) / line(x2, y2, x0, y0, x1, y1);
            float third = line(x0, y0, x1, y1, i, j) / line(x0, y0, x1, y1, x2, y2);

            if (in_triangle(x0, y0, x1, y1, x2, y2, i, j))
            {
                color.r = first*c0.r + second*c1.r + third*c2.r;
                color.g = first*c0.g + second*c1.g + third*c2.g;
                color.b = first*c0.b + second*c1.b + third*c2.b;
                image.set(i, j, color);
            }
        }
    }
}

void draw_triangle(float x0, float y0,float z0, float x1, float y1,float z1, float x2, float y2,float z2 ,TGAImage &image,TGAColor color)
{
    Vec3 a = {x0,y0,z0};
    Vec3 b = {x1,y1,z1};
    Vec3 c = {x2, y2,z2};

    // podijeli kord sa z
    a[0]/=a[2]; a[1]/=a[2];
    b[0]/=b[2]; b[1]/=b[2];
    c[0]/=c[2]; c[1]/=c[2];

    //tranformacija

    a[0] = (1 + a[0]) * 0.5 * width; a[1]= (1 + a[1]) * 0.5 * height;
    b[0] = (1 + b[0]) * 0.5 * width; b[1]= (1 + b[1]) * 0.5 * height;
    c[0] = (1 + c[0]) * 0.5 * width; c[1]= (1 + c[1]) * 0.5 * height;

    // 1/z
    a[2] = 1 /a[2];  b[2]= 1 /b[2]; c[2] = 1 /c[2];

    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            Vec3 p = {((float)i+0.5f),(height-(float)j+0.5f), 0};
            float first = help(c, b, p);
            float second  = help(a, c, p);
            float third = help(b, a, p);
            if((first >= 0 && second >= 0 && third >= 0))
                image.set(i, j, color);
        }
    }
}

void draw_triangle_tex(TGAImage& image, float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float u0, float v0, float u1, float v1, float u2, float v2, const char* path)
{
    TGAColor color;
    TGAImage texture;
    texture.read_tga_file(path);

    Vec3 a {x0,y0,z0};
    Vec3 b {x1,y1,z1};
    Vec3 c {x2, y2,z2};

    Vec2 uv0 {u0, v0};
    Vec2 uv1 {u1, v1};
    Vec2 uv2 {u2,v2};

    a[0]/=a[2]; a[1]/=a[2];
    b[0]/=b[2]; b[1]/=b[2];
    c[0]/=c[2]; c[1]/=c[2];

    //tranformacija
    a[0] = (1 + a[0]) * 0.5 * width; a[1]= (1 + a[1]) * 0.5 * height;
    b[0] = (1 + b[0]) * 0.5 * width; b[1]= (1 + b[1]) * 0.5 * height;
    c[0] = (1 + c[0]) * 0.5 * width; c[1]= (1 + c[1]) * 0.5 * height;

    // 1/z
    a[2] = 1 /a[2];  b[2]= 1 /b[2]; c[2] = 1 /c[2];

    uv0[0] /= a[2]; uv0[1] /= a[2];
    uv1[0] /= b[2]; uv1[1] /= b[2];
    uv2[0] /= c[2]; uv2[1] /= c[2];

    for (int j = 0; j < height; ++j)
    {
        for (int i = 0; i < width; ++i)
        {
            Vec3 p = {((float)i+0.5f),(height-(float)j+0.5f),0};
            float first = help(c, b, p);
            float second  = help(a, c, p);
            float third = help(b, a, p);
            if((first >= 0 && second >= 0 && third >= 0))
            {
                color = texture.get(i, j);
                image.set(i, j, color);
            }
        }
    }

}

void draw_triangle_tex_corrected(TGAImage& image, float x0, float y0, float z0, float x1, float y1, float z1, float x2, float y2, float z2, float u0, float v0, float u1, float v1, float u2, float v2, const char* path)
{
    TGAColor color;
    TGAImage texture;
    texture.read_tga_file(path);

    Vec3 a {x0,y0,z0};
    Vec3 b {x1,y1,z1};
    Vec3 c {x2, y2,z2};

    Vec2 uv0 {u0, v0};
    Vec2 uv1 {u1, v1};
    Vec2 uv2 {u2,v2};

    // podijeli kord sa z
    a[0]/=a[2]; a[1]/=a[2];
    b[0]/=b[2]; b[1]/=b[2];
    c[0]/=c[2]; c[1]/=c[2];

    //tranformacija
    a[0] = (1 + a[0]) * 0.5 * width; a[1]= (1 + a[1]) * 0.5 * height;
    b[0] = (1 + b[0]) * 0.5 * width; b[1]= (1 + b[1]) * 0.5 * height;
    c[0] = (1 + c[0]) * 0.5 * width; c[1]= (1 + c[1]) * 0.5 * height;

    uv0[0] /= a[2]; uv0[1] /= a[2];
    uv1[0] /= b[2]; uv1[1] /= b[2];
    uv2[0] /= c[2]; uv2[1] /= c[2];

    for (int j = 0; j <= width; ++j)
    {
        for (int i = 0; i <= height; ++i)
        {
            float first = line(b[0], b[1], c[0], c[1], i, j) / line(b[0], b[1], c[0], c[1], a[0], a[1]);
            float second = line(c[0], c[1], a[0], a[1], i, j) / line(c[0], c[1], a[0], a[1], b[0], b[1]);
            float third = line(a[0], a[1], b[0], b[1], i, j) / line(a[0], a[1], b[0], b[1], c[0], c[1]);

            if (first > 0 && first < 1 && second > 0 && second < 1 && third > 0 && third < 1)
            {
                float x = (first * uv0[0] + second * uv1[0] + third * uv2[0]) * texture.get_width();
                float y = (first * uv0[1] + second * uv1[1] + third * uv2[1]) * texture.get_height();
                float z  = 1 / (first * a[2] + second * b[2] + third * c[2]);
                x *= z; y *= z;
                set_color(i, j, image, texture.get(round(x), round(y)));
            }
        }
    }
}


int main()
{
    // definiraj sliku
    TGAImage image(width, height, TGAImage::RGB);
    
    //draw_triangle_2d(image, 15, 20, 200, 90, 200, 250, red);
    //draw_triangle_2d(image, 410, 66, 350, 100, 400, 300, green);
    //draw_triangle_2d_gouraurd(image, 18, 245, blue, 250, 260, red, 18, 500, green);

    //draw_triangle(-48,-10, 82, 29, -15, 44, 13, 34, 114,image, white);

    //draw_triangle_tex(image, -48, -10,  82, 29, -15,  44, 13,  34, 114, 0, 0, 0, 1, 1, 0, "./brick3.tga");
    draw_triangle_tex_corrected(image, -48, -10,  82, 29, -15,  44, 13,  34, 114, 0, 0, 0, 1, 1, 0, "./brick3.tga");

    // spremi sliku
    image.flip_vertically();
    image.write_tga_file("brick_tex_corr.tga");
}