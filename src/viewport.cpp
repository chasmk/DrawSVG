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

  /*��svg����ռ��׼��Ϊ�豸����ռ�
    ����Ĳ����Ǳ�׼����SVG canvas����
    span���Ǵ��ڵ���Y��ֵ
    m�� (centerX, centerY) ��� (0.5, 0.5)
    m�� (centerX, centerY + vspan) ��� (0.5, 1.0)
    */

  std::cout << "set_viewbox() " << centerX << " " << centerY << " " << vspan << "\n";

  Matrix3x3 m = Matrix3x3::identity();

  //����ƽ��(������)��0.5���ƶ�һ�룬��centerλ������
  m(0, 2) = 0.5 * (vspan - centerX) / vspan;
  m(1, 2) = 0.5 * (vspan - centerY) / vspan;

  //�������ţ���С vspan*2 ����[0,0.5]֮��
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
