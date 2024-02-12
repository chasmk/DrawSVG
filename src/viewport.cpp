#include "viewport.h"
#include "CMU462.h"

namespace CMU462 {

void ViewportImp::set_viewbox( float centerX, float centerY, float vspan ) {

  // Task 5 (part 2): 
  // Set svg coordinate to normalized device coordinate transformation. Your input
  // arguments are defined as normalized SVG canvas coordinates.
  this->centerX = centerX;
  this->centerY = centerY;
  this->vspan = vspan; 

  /*将svg坐标空间标准化为设备坐标空间
    输入的参数是标准化的SVG canvas坐标
    span总是大于等于Y的值
    m将 (centerX, centerY) 变成 (0.5, 0.5)
    m将 (centerX, centerY + vspan) 变成 (0.5, 1.0)
    */

  std::cout << "set_viewbox() " << centerX << " " << centerY << " " << vspan << "\n";

  Matrix3x3 m = Matrix3x3::identity();

  //设置平移(向右下)，0.5是移动一半，让center位于中心
  m(0, 2) = 0.5 * (vspan - centerX) / vspan;
  m(1, 2) = 0.5 * (vspan - centerY) / vspan;

  //设置缩放，缩小 vspan*2 倍，[0,0.5]之间
  m(0, 0) = 0.5 / vspan;
  m(1, 1) = 0.5 / vspan;

  set_svg_2_norm(m);
}

void ViewportImp::update_viewbox( float dx, float dy, float scale ) { 
  
  this->centerX -= dx;
  this->centerY -= dy;
  this->vspan *= scale;
  set_viewbox( centerX, centerY, vspan );
}

} // namespace CMU462
