#include "texture.h"
#include "color.h"

#include <assert.h>
#include <iostream>
#include <algorithm>

using namespace std;

namespace CMU462 {

inline void uint8_to_float( float dst[4], unsigned char* src ) {
  uint8_t* src_uint8 = (uint8_t *)src;
  dst[0] = src_uint8[0] / 255.f;
  dst[1] = src_uint8[1] / 255.f;
  dst[2] = src_uint8[2] / 255.f;
  dst[3] = src_uint8[3] / 255.f;
}

inline void float_to_uint8( unsigned char* dst, float src[4] ) {
  uint8_t* dst_uint8 = (uint8_t *)dst;
  dst_uint8[0] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[0])));
  dst_uint8[1] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[1])));
  dst_uint8[2] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[2])));
  dst_uint8[3] = (uint8_t) ( 255.f * max( 0.0f, min( 1.0f, src[3])));
}

void Sampler2DImp::generate_mips(Texture& tex, int startLevel) {

  // NOTE: 
  // This starter code allocates the mip levels and generates a level 
  // map by filling each level with placeholder data in the form of a 
  // color that differs from its neighbours'. You should instead fill
  // with the correct data!

  // Task 7: Implement this

  // check start level
  if ( startLevel >= tex.mipmap.size() ) {
    std::cerr << "Invalid start level"; 
  }

  // allocate sublevels
  int baseWidth  = tex.mipmap[startLevel].width;
  int baseHeight = tex.mipmap[startLevel].height;
  int numSubLevels = (int)(log2f( (float)max(baseWidth, baseHeight)));

  numSubLevels = min(numSubLevels, kMaxMipLevels - startLevel - 1);
  tex.mipmap.resize(startLevel + numSubLevels + 1);

  int width  = baseWidth;
  int height = baseHeight;
  for (int i = 1; i <= numSubLevels; i++) {//为每个level开辟空间，每次wh除2

    MipLevel& level = tex.mipmap[startLevel + i];

    // handle odd size texture by rounding down
    width  = max( 1, width  / 2); assert(width  > 0);
    height = max( 1, height / 2); assert(height > 0);

    level.width = width;
    level.height = height;
    level.texels = vector<unsigned char>(4 * width * height);

  }

  for(size_t i = 1; i < tex.mipmap.size(); ++i) {//遍历每个mipmap，填充像素
       
    
    MipLevel& mip = tex.mipmap[i];

    int stepx = baseWidth / mip.width;
    int stepy = baseHeight / mip.height;
    
    for (int i = 0; i < mip.height; i++) {
        for (int j = 0; j < mip.width; j++) {
            Color color;
            for (int y = 0; y < stepy; y++) {
                for (int x = 0; x < stepx; x++) {
                    color.r += tex.mipmap[startLevel].texels[((i * stepx + y) * baseWidth + j * stepy + x) * 4];
                    color.g += tex.mipmap[startLevel].texels[((i * stepx + y) * baseWidth + j * stepy + x) * 4 + 1];
                    color.b += tex.mipmap[startLevel].texels[((i * stepx + y) * baseWidth + j * stepy + x) * 4 + 2];
                    color.a += tex.mipmap[startLevel].texels[((i * stepx + y) * baseWidth + j * stepy + x) * 4 + 3];
                }
            }
            color.r /= stepx * stepy;
            color.g /= stepx * stepy;
            color.b /= stepx * stepy;
            color.a /= stepx * stepy;
            mip.texels[(i * mip.width + j) * 4] = color.r;
            mip.texels[(i * mip.width + j) * 4 + 1] = color.g;
            mip.texels[(i * mip.width + j) * 4 + 2] = color.b;
            mip.texels[(i * mip.width + j) * 4 + 3] = color.a;
        }
    }
  }

}

Color Sampler2DImp::sample_nearest(Texture& tex, 
                                   float u, float v, 
                                   int level) {

  // Task 6: Implement nearest neighbour interpolation
  
  // return magenta for invalid level
    if (level < 0 || level > kMaxMipLevels) return Color(255, 0, 255);

    MipLevel& miplevel = tex.mipmap[level];
    int U = round(u * miplevel.width);//四舍五入
    int V = round(v * miplevel.height);
    Color color;
    color.r = miplevel.texels[4 * (U + V * miplevel.width)];
    color.g = miplevel.texels[1 + 4 * (U + V * miplevel.width)];
    color.b = miplevel.texels[2 + 4 * (U + V * miplevel.width)];
    color.a = miplevel.texels[3 + 4 * (U + V * miplevel.width)];
    return color;

}

Color Sampler2DImp::sample_bilinear(Texture& tex, 
                                    float u, float v, 
                                    int level) {
  
  // Task 6: Implement bilinear filtering

  // return magenta for invalid level
    if (level < 0 || level > kMaxMipLevels) return Color(255, 0, 255);

    MipLevel& miplevel = tex.mipmap[level];
    float U = u * miplevel.width;
    float V = v * miplevel.height;
    int i = floor(U - 0.5);
    int j = floor(V - 0.5);
    float s = U - (i + 0.5);
    float t = V - (j + 0.5);
    Color f00(miplevel.texels[4 * (i + j * miplevel.width)],
        miplevel.texels[1 + 4 * (i + j * miplevel.width)],
        miplevel.texels[2 + 4 * (i + j * miplevel.width)],
        miplevel.texels[3 + 4 * (i + j * miplevel.width)]);
    Color f01(miplevel.texels[4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[1 + 4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[2 + 4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[3 + 4 * (i + (j + 1) * miplevel.width)]);
    Color f11(miplevel.texels[4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[1 + 4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[2 + 4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[3 + 4 * ((i + 1) + (j - 1) * miplevel.width)]);
    Color f10(miplevel.texels[4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[1 + 4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[2 + 4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[3 + 4 * ((i + 1) + j * miplevel.width)]);

    Color color;
    color = (1.0 - t) * ((1.0 - s) * f00 + s * f10) + t * ((1.0 - s) * f01 + s * f11);
    return color;

}

Color Sampler2DImp::sample_trilinear(Texture& tex, 
                                     float u, float v, 
                                     float u_scale, float v_scale) {
     
  // Task 7: Implement trilinear filtering

  // return magenta for invalid level
    int level1 = floor(u_scale);
    int level2 = ceil(u_scale);
    if (level1 < 0 || level2 > kMaxMipLevels) return Color(255, 0, 255);
    
    MipLevel& miplevel = tex.mipmap[level1];
    float U = u * miplevel.width;
    float V = v * miplevel.height;
    int i = floor(U - 0.5);
    int j = floor(V - 0.5);
    float s = U - (i + 0.5);
    float t = V - (j + 0.5);
    Color f000(miplevel.texels[4 * (i + j * miplevel.width)],
        miplevel.texels[1 + 4 * (i + j * miplevel.width)],
        miplevel.texels[2 + 4 * (i + j * miplevel.width)],
        miplevel.texels[3 + 4 * (i + j * miplevel.width)]);
    Color f010(miplevel.texels[4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[1 + 4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[2 + 4 * (i + (j + 1) * miplevel.width)],
        miplevel.texels[3 + 4 * (i + (j + 1) * miplevel.width)]);
    Color f110(miplevel.texels[4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[1 + 4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[2 + 4 * ((i + 1) + (j + 1) * miplevel.width)],
        miplevel.texels[3 + 4 * ((i + 1) + (j - 1) * miplevel.width)]);
    Color f100(miplevel.texels[4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[1 + 4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[2 + 4 * ((i + 1) + j * miplevel.width)],
        miplevel.texels[3 + 4 * ((i + 1) + j * miplevel.width)]);

    Color g00, g10, h0;
    g00 = s * f100 + (1.0 - s) * f000;
    g10 = s * f110 + (1.0 - s) * f010;
    h0 = t * g10 + (1.0 - t) * g00;

    MipLevel& miplevel1 = tex.mipmap[level2];
    U = u * miplevel1.width;
    V = v * miplevel1.height;
    i = floor(U - 0.5);
    j = floor(V - 0.5);
    s = U - (i + 0.5);
    t = V - (j + 0.5);
    Color f001(miplevel1.texels[4 * (i + j * miplevel1.width)],
        miplevel1.texels[1 + 4 * (i + j * miplevel1.width)],
        miplevel1.texels[2 + 4 * (i + j * miplevel1.width)],
        miplevel1.texels[3 + 4 * (i + j * miplevel1.width)]);
    Color f011(miplevel1.texels[4 * (i + (j + 1) * miplevel1.width)],
        miplevel1.texels[1 + 4 * (i + (j + 1) * miplevel1.width)],
        miplevel1.texels[2 + 4 * (i + (j + 1) * miplevel1.width)],
        miplevel1.texels[3 + 4 * (i + (j + 1) * miplevel1.width)]);
    Color f111(miplevel1.texels[4 * ((i + 1) + (j + 1) * miplevel1.width)],
        miplevel1.texels[1 + 4 * ((i + 1) + (j + 1) * miplevel1.width)],
        miplevel1.texels[2 + 4 * ((i + 1) + (j + 1) * miplevel1.width)],
        miplevel1.texels[3 + 4 * ((i + 1) + (j - 1) * miplevel1.width)]);
    Color f101(miplevel1.texels[4 * ((i + 1) + j * miplevel1.width)],
        miplevel1.texels[1 + 4 * ((i + 1) + j * miplevel1.width)],
        miplevel1.texels[2 + 4 * ((i + 1) + j * miplevel1.width)],
        miplevel1.texels[3 + 4 * ((i + 1) + j * miplevel1.width)]);

    Color g01, g11, h1;
    g01 = s * f101 + (1.0 - s) * f001;
    g11 = s * f111 + (1.0 - s) * f011;
    h1 = t * g11 + (1.0 - t) * g01;
    
    float w = u_scale - floor(u_scale);
    return w * h1 + (1.0 - w) * h0;
}

} // namespace CMU462
