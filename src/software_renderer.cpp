#include "software_renderer.h"

#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>

#include "triangulation.h"

using namespace std;

namespace CMU462 {


// Implements SoftwareRenderer //

void SoftwareRendererImp::draw_svg( SVG& svg ) {

  // set top level transformation
  transformation = svg_2_screen;

  // draw all elements
  for ( size_t i = 0; i < svg.elements.size(); ++i ) {//绘制element之前应用transform，然后再取消
    transformation = transformation * svg.elements[i]->transform;
    draw_element(svg.elements[i]);
    transformation =  transformation * svg.elements[i]->transform.inv();
    //print(svg.elements[i]->transform);
  }

  // draw canvas outline
  Vector2D a = transform(Vector2D(    0    ,     0    )); a.x--; a.y--;
  Vector2D b = transform(Vector2D(svg.width,     0    )); b.x++; b.y--;
  Vector2D c = transform(Vector2D(    0    ,svg.height)); c.x--; c.y++;
  Vector2D d = transform(Vector2D(svg.width,svg.height)); d.x++; d.y++;

  rasterize_line(a.x, a.y, b.x, b.y, Color::Black);
  rasterize_line(a.x, a.y, c.x, c.y, Color::Black);
  rasterize_line(d.x, d.y, b.x, b.y, Color::Black);
  rasterize_line(d.x, d.y, c.x, c.y, Color::Black);

  // resolve and send to render target
  resolve();

}

void SoftwareRendererImp::set_sample_rate( size_t sample_rate ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  //if (target_h == 0 || target_w == 0) return;
  this->sample_rate = sample_rate;
  delete[] this->supersample_target;
  //cout << "set_sample_rate()\n";
  this->supersample_target = new unsigned char[4 * target_w * target_h * sample_rate * sample_rate];
}

void SoftwareRendererImp::set_render_target( unsigned char* render_target,
                                             size_t width, size_t height ) {

  // Task 4: 
  // You may want to modify this for supersampling support
  this->render_target = render_target;
  this->target_w = width;
  this->target_h = height;
  
  //delete[]supersample_target;
  //supersample_target = new unsigned char[4 * target_w * target_h * sample_rate * sample_rate];
  //cout << "set_render_target()\n";
}
/*用不到了
void ApplyTransformation(Polygon& polygon) {
    for (int i = 0; i < polygon.points.size(); i++) {//对每个polygon的point应用transformation
        Vector3D v(polygon.points[i].x, polygon.points[i].y, 1);
        v = polygon.transform * v;
        polygon.points[i].x = v.x / v.z;
        polygon.points[i].y = v.y / v.z;
    }
}*/

void SoftwareRendererImp::draw_element( SVGElement* element ) {

  // Task 5 (part 1):
  // Modify this to implement the transformation stack

  switch(element->type) {
    case POINT:
        //cout << "POINT\n";
      draw_point(static_cast<Point&>(*element));
      break;
    case LINE:
        //cout << "LINE\n";
      draw_line(static_cast<Line&>(*element));
      break;
    case POLYLINE:
        //cout << "POLYLINE\n";
      draw_polyline(static_cast<Polyline&>(*element));
      break;
    case RECT:
        //cout << "RECT\n";
      draw_rect(static_cast<Rect&>(*element));
      break;
    case POLYGON:
    {
        //cout << "POLYGON\n";
        Polygon& polygon = static_cast<Polygon&>(*element);
        //ApplyTransformation(polygon);
        draw_polygon(polygon);
        break;
    }
    case ELLIPSE:
      draw_ellipse(static_cast<Ellipse&>(*element));
      break;
    case IMAGE:
      draw_image(static_cast<Image&>(*element));
      break;
    case GROUP:
    {
        //cout << "GROUP\n";
        Group& group = static_cast<Group&>(*element);
        //
        for (size_t i = 0; i < group.elements.size(); ++i) {//对于group做额外的操作
            transformation = transformation * group.elements[i]->transform;
            draw_element(group.elements[i]);
            transformation = transformation * group.elements[i]->transform.inv();
        }
        //draw_group(static_cast<Group&>(*element));//不调用这个，直接在这里处理
        
    }
      break;
    default:
      break;
  }

}


// Primitive Drawing //

void SoftwareRendererImp::draw_point( Point& point ) {

  Vector2D p = transform(point.position);
  rasterize_point( p.x, p.y, point.style.fillColor );

}

void SoftwareRendererImp::draw_line( Line& line ) { 

  Vector2D p0 = transform(line.from);
  Vector2D p1 = transform(line.to);
  rasterize_line( p0.x, p0.y, p1.x, p1.y, line.style.strokeColor );

}

void SoftwareRendererImp::draw_polyline( Polyline& polyline ) {

  Color c = polyline.style.strokeColor;

  if( c.a != 0 ) {
    int nPoints = polyline.points.size();
    for( int i = 0; i < nPoints - 1; i++ ) {
      Vector2D p0 = transform(polyline.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polyline.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_rect( Rect& rect ) {

  Color c;
  
  // draw as two triangles
  float x = rect.position.x;
  float y = rect.position.y;
  float w = rect.dimension.x;
  float h = rect.dimension.y;

  Vector2D p0 = transform(Vector2D(   x   ,   y   ));
  Vector2D p1 = transform(Vector2D( x + w ,   y   ));
  Vector2D p2 = transform(Vector2D(   x   , y + h ));
  Vector2D p3 = transform(Vector2D( x + w , y + h ));
  
  // draw fill
  c = rect.style.fillColor;
  if (c.a != 0 ) {
    rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    rasterize_triangle( p2.x, p2.y, p1.x, p1.y, p3.x, p3.y, c );
  }

  // draw outline
  c = rect.style.strokeColor;
  if( c.a != 0 ) {
    rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    rasterize_line( p1.x, p1.y, p3.x, p3.y, c );
    rasterize_line( p3.x, p3.y, p2.x, p2.y, c );
    rasterize_line( p2.x, p2.y, p0.x, p0.y, c );
  }

}

void SoftwareRendererImp::draw_polygon( Polygon& polygon ) {

  Color c;

  // draw fill
  c = polygon.style.fillColor;
  if( c.a != 0 ) {

    // triangulate
    vector<Vector2D> triangles;
    triangulate( polygon, triangles );

    // draw as triangles
    for (size_t i = 0; i < triangles.size(); i += 3) {
      Vector2D p0 = transform(triangles[i + 0]);
      Vector2D p1 = transform(triangles[i + 1]);
      Vector2D p2 = transform(triangles[i + 2]);
      rasterize_triangle( p0.x, p0.y, p1.x, p1.y, p2.x, p2.y, c );
    }
  }

  // draw outline
  c = polygon.style.strokeColor;
  if( c.a != 0 ) {
    int nPoints = polygon.points.size();
    for( int i = 0; i < nPoints; i++ ) {
      Vector2D p0 = transform(polygon.points[(i+0) % nPoints]);
      Vector2D p1 = transform(polygon.points[(i+1) % nPoints]);
      rasterize_line( p0.x, p0.y, p1.x, p1.y, c );
    }
  }
}

void SoftwareRendererImp::draw_ellipse( Ellipse& ellipse ) {

  // Extra credit 

}

void SoftwareRendererImp::draw_image( Image& image ) {

  Vector2D p0 = transform(image.position);
  Vector2D p1 = transform(image.position + image.dimension);

  rasterize_image( p0.x, p0.y, p1.x, p1.y, image.tex );
}

void SoftwareRendererImp::draw_group( Group& group ) {

  for ( size_t i = 0; i < group.elements.size(); ++i ) {
    draw_element(group.elements[i]);
  }

}

// Rasterization //

// The input arguments in the rasterization functions 
// below are all defined in screen space coordinates

void SoftwareRendererImp::rasterize_point( float x, float y, Color color ) {

    if (sample_rate > 1) {
        rasterize_point_supersample(x, y, color);
        return;
    }
  // fill in the nearest pixel
  int sx = (int) floor(x);
  int sy = (int) floor(y);

  // check bounds
  if ( sx < 0 || sx >= target_w ) return;
  if ( sy < 0 || sy >= target_h ) return;

  // fill sample - NOT doing alpha blending!
  render_target[4 * (sx + sy * target_w)    ] = (uint8_t) (color.r * 255);
  render_target[4 * (sx + sy * target_w) + 1] = (uint8_t) (color.g * 255);
  render_target[4 * (sx + sy * target_w) + 2] = (uint8_t) (color.b * 255);
  render_target[4 * (sx + sy * target_w) + 3] = (uint8_t) (color.a * 255);

}

void SoftwareRendererImp::rasterize_point_supersample(float x, float y, Color color) {
    // fill in the nearest pixel
    int sx = (int)floor(x);
    int sy = (int)floor(y);

    // check bounds
    if (sx < 0 || sx >= target_w) return;
    if (sy < 0 || sy >= target_h) return;

    for (int s = 0; s < sample_rate; s++) {//行
        for (int t = 0; t < sample_rate; t++) {//列
            supersample_target[4 * (sx * sample_rate + t + (sy * sample_rate + s) * target_w * sample_rate)] = (uint8_t)(color.r * 255);
            supersample_target[4 * (sx * sample_rate + t + (sy * sample_rate + s) * target_w * sample_rate) + 1] = (uint8_t)(color.g * 255);
            supersample_target[4 * (sx * sample_rate + t + (sy * sample_rate + s) * target_w * sample_rate) + 2] = (uint8_t)(color.b * 255);
            supersample_target[4 * (sx * sample_rate + t + (sy * sample_rate + s) * target_w * sample_rate) + 3] = (uint8_t)(color.a * 255);
        }
    }
    // fill sample - NOT doing alpha blending!
    
}

void SoftwareRendererImp::rasterize_line( float x0, float y0,
                                          float x1, float y1,
                                          Color color) {

  // Task 2: 
  // Implement line rasterization
  // 
    if (sample_rate > 1) {
        rasterize_line_supersample(x0, y0, x1, y1, color);
        return;
    }
    //检查边界
    if (x0 < 0 || x0 >= target_w) return;
    if (y0 < 0 || y0 >= target_h) return;
    if (x1 < 0 || x1 >= target_w) return;
    if (y1 < 0 || y1 >= target_h) return;

    bool isSteep = abs(y1 - y0) > abs(x1 - x0);//斜率是否大于1/小于-1
    if (isSteep) {//通过交换xy坐标，将斜率变为[-1，1]之间
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {//分别交换x和y，统一绘制x递增的直线
        swap(x0, x1);
        swap(y0, y1);
    }
    // 至此，只有斜率[-1,1]之间的直线需要绘制
    
    //找到最近的像素点
    int sx0 = (int)floor(x0);
    int sy0 = (int)floor(y0);
    int sx1 = (int)floor(x1);
    int sy1 = (int)floor(y1);
    //cout << target_w << " " << target_h << "(" << sx0 << "," << sy0 << " (" << sx1 << "," << sy1 << ")" << endl;

    float dx = sx1 - sx0;
    float dy = abs(sy1 - sy0);
    float m = dy / dx;
    float error = 0;// 记录实际y和绘制的y之间的差距[-0.5, 0.5]

    //当前绘制的x，y坐标
    int curx = sx0;
    int cury = sy0;
    
    int ystep = y0 < y1 ? 1 : -1;//y增长的步长

    for (; curx <= sx1; curx++) {
        //绘制当前点
        if (!isSteep) {
            render_target[4 * (curx + cury * target_w)] = (uint8_t)(color.r * 255);
            render_target[4 * (curx + cury * target_w) + 1] = (uint8_t)(color.g * 255);
            render_target[4 * (curx + cury * target_w) + 2] = (uint8_t)(color.b * 255);
            render_target[4 * (curx + cury * target_w) + 3] = (uint8_t)(color.a * 255);
        }
        else {
            render_target[4 * (cury + curx * target_w)] = (uint8_t)(color.r * 255);
            render_target[4 * (cury + curx * target_w) + 1] = (uint8_t)(color.g * 255);
            render_target[4 * (cury + curx * target_w) + 2] = (uint8_t)(color.b * 255);
            render_target[4 * (cury + curx * target_w) + 3] = (uint8_t)(color.a * 255);
        }
        
        error = error + m;//x每多1，误差就多m
        if (error >= 0.5) {//误差大于一格的一半，y增加1，相应地误差减去1
            cury += ystep;
            error -= 1;
        }
    }
    
}

void SoftwareRendererImp::rasterize_line_supersample(float x0, float y0,
    float x1, float y1,
    Color color) {
    //检查边界
    if (x0 < 0 || x0 >= target_w) return;
    if (y0 < 0 || y0 >= target_h) return;
    if (x1 < 0 || x1 >= target_w) return;
    if (y1 < 0 || y1 >= target_h) return;

    bool isSteep = abs(y1 - y0) > abs(x1 - x0);//斜率是否大于1/小于-1
    if (isSteep) {//通过交换xy坐标，将斜率变为[-1，1]之间
        swap(x0, y0);
        swap(x1, y1);
    }

    if (x0 > x1) {//分别交换x和y，统一绘制x递增的直线
        swap(x0, x1);
        swap(y0, y1);
    }
    // 至此，只有斜率[-1,1]之间的直线需要绘制

    //找到最近的像素点
    int sx0 = (int)floor(x0);
    int sy0 = (int)floor(y0);
    int sx1 = (int)floor(x1);
    int sy1 = (int)floor(y1);
    //cout << target_w << " " << target_h << "(" << sx0 << "," << sy0 << " (" << sx1 << "," << sy1 << ")" << endl;

    float dx = sx1 - sx0;
    float dy = abs(sy1 - sy0);
    float m = dy / dx;
    float error = 0;// 记录实际y和绘制的y之间的差距[-0.5, 0.5]

    //当前绘制的x，y坐标
    int curx = sx0;
    int cury = sy0;

    int ystep = y0 < y1 ? 1 : -1;//y增长的步长

    for (; curx <= sx1; curx++) {
        //绘制当前点
        if (!isSteep) {
            for (int s = 0; s < sample_rate; s++) {//行
                for (int t = 0; t < sample_rate; t++) {//列
                    supersample_target[4 * (curx * sample_rate + t + (cury * sample_rate + s) * target_w * sample_rate)] = (uint8_t)(color.r * 255);
                    supersample_target[4 * (curx * sample_rate + t + (cury * sample_rate + s) * target_w * sample_rate) + 1] = (uint8_t)(color.g * 255);
                    supersample_target[4 * (curx * sample_rate + t + (cury * sample_rate + s) * target_w * sample_rate) + 2] = (uint8_t)(color.b * 255);
                    supersample_target[4 * (curx * sample_rate + t + (cury * sample_rate + s) * target_w * sample_rate) + 3] = (uint8_t)(color.a * 255);
                }
            }
        }
        else {
            for (int s = 0; s < sample_rate; s++) {//行
                for (int t = 0; t < sample_rate; t++) {//列
                    supersample_target[4 * (cury * sample_rate + t + (curx * sample_rate + s) * target_w * sample_rate)] = (uint8_t)(color.r * 255);
                    supersample_target[4 * (cury * sample_rate + t + (curx * sample_rate + s) * target_w * sample_rate) + 1] = (uint8_t)(color.g * 255);
                    supersample_target[4 * (cury * sample_rate + t + (curx * sample_rate + s) * target_w * sample_rate) + 2] = (uint8_t)(color.b * 255);
                    supersample_target[4 * (cury * sample_rate + t + (curx * sample_rate + s) * target_w * sample_rate) + 3] = (uint8_t)(color.a * 255);
                }
            }
        }

        error = error + m;//x每多1，误差就多m
        if (error >= 0.5) {//误差大于一格的一半，y增加1，相应地误差减去1
            cury += ystep;
            error -= 1;
        }
    }
}

void SoftwareRendererImp::rasterize_triangle( float x0, float y0,
                                              float x1, float y1,
                                              float x2, float y2,
                                              Color color ) {
  // Task 3: 
  // Implement triangle rasterization
    //超采样版本
    if (sample_rate > 1) {
        rasterize_triangle_supersample(x0, y0, x1, y1, x2, y2, color);
        return;
    }
    //先找到bbox边界，再遍历所有点并判断是否在三角形内
    int minx = floor(min(x0, min(x1, x2)));
    int miny = floor(min(y0, min(y1, y2)));
    int maxx = floor(max(x0, max(x1, x2)));
    int maxy = floor(max(y0, max(y1, y2)));

    for (int i = miny; i <= maxy; i += 1) {
        for (int j = minx; j <= maxx; j += 1) {
            //三角形三条边的向量
            Vector2D v01(x1 - x0, y1 - y0);
            Vector2D v12(x2 - x1, y2 - y1);
            Vector2D v20(x0 - x2, y0 - y2);
            //测试点到三角形顶点的向量
            Vector2D v0t(j + 0.5 - x0, i + 0.5 - y0);
            Vector2D v1t(j + 0.5 - x1, i + 0.5 - y1);
            Vector2D v2t(j + 0.5 - x2, i + 0.5 - y2);
            //三个叉积值
            double res1 = cross(v01, v0t);
            double res2 = cross(v12, v1t);
            double res3 = cross(v20, v2t);

            if (res1 >= 0 && res2 >= 0 && res3 >= 0 || res1 <= 0 && res2 <= 0 && res3 <= 0) {
                //cout << j <<","<<i << res1 << " " << res2 << " " << res3 << endl;
                render_target[4 * (j + i * target_w)] = (uint8_t)(color.r * 255);
                render_target[4 * (j + i * target_w) + 1] = (uint8_t)(color.g * 255);
                render_target[4 * (j + i * target_w) + 2] = (uint8_t)(color.b * 255);
                render_target[4 * (j + i * target_w) + 3] = (uint8_t)(color.a * 255);
            }
        }
    }

}

void SoftwareRendererImp::rasterize_triangle_supersample(float x0, float y0,
    float x1, float y1,
    float x2, float y2,
    Color color)
{
    //先找到bbox边界，再遍历所有点并判断是否在三角形内
    int minx = floor(min(x0, min(x1, x2)));
    int miny = floor(min(y0, min(y1, y2)));
    int maxx = ceil(max(x0, max(x1, x2)));
    int maxy = ceil(max(y0, max(y1, y2)));

    float step = 1.0 / sample_rate;//遍历的步长
    float sstep = step / 2.0;//得到采样点位置的距离，
    for (int si = miny*sample_rate; si < maxy * sample_rate; si += 1) {//遍历所有超采样点的超采样坐标
        int i = si / sample_rate + (si % sample_rate) * step;//得到左上角实际位置
        for (int sj = minx * sample_rate; sj < maxx * sample_rate; sj += 1) {
            int j = sj / sample_rate + (sj % sample_rate) * step;//得到左上角实际位置
            //三角形三条边的向量
            Vector2D v01(x1 - x0, y1 - y0);
            Vector2D v12(x2 - x1, y2 - y1);
            Vector2D v20(x0 - x2, y0 - y2);
            //测试点到三角形顶点的向量，这里取小方块中间为采样点
            Vector2D v0t(j + sstep - x0, i + sstep - y0);
            Vector2D v1t(j + sstep - x1, i + sstep - y1);
            Vector2D v2t(j + sstep - x2, i + sstep - y2);
            //三个叉积值
            double res1 = cross(v01, v0t);
            double res2 = cross(v12, v1t);
            double res3 = cross(v20, v2t);

            if (res1 >= 0 && res2 >= 0 && res3 >= 0 || res1 <= 0 && res2 <= 0 && res3 <= 0) {
                supersample_target[4 * (sj + si * target_w * sample_rate)] = (uint8_t)(color.r * 255);
                supersample_target[4 * (sj + si * target_w * sample_rate) + 1] = (uint8_t)(color.g * 255);
                supersample_target[4 * (sj + si * target_w * sample_rate) + 2] = (uint8_t)(color.b * 255);
                supersample_target[4 * (sj + si * target_w * sample_rate) + 3] = (uint8_t)(color.a * 255);
            }
        }
    }
}

void SoftwareRendererImp::rasterize_image( float x0, float y0,
                                           float x1, float y1,
                                           Texture& tex ) {
  // Task 6: 
  // Implement image rasterization
    cout << tex.mipmap[0].height << " " << tex.mipmap[0].width <<" " << tex.mipmap[0].texels.size() << endl;
    printf("坐标 %f, %f  %f, %f\n", x0, y0, x1, y1);
    int sx0 = (int)round(x0);
    int sy0 = (int)round(y0);
    int sx1 = (int)ceil(x1);
    int sy1 = (int)ceil(y1);

    // 检查范围
    if (sx0 < 0 || sx0 >= target_w) return;
    if (sy0 < 0 || sy0 >= target_h) return;
    if (sx1 < 0 || sx1 >= target_w) return;
    if (sy1 < 0 || sy1 >= target_h) return;
    
    int w = sx1 - sx0 + 1;
    int h = sy1 - sy0 + 1;

    Sampler2DImp* sampler2d = new Sampler2DImp;

    for (int i = sy0; i < sy1; i++) {
        for (int j = sx0; j < sx1; j++) {
            //映射到[0,1]
            float u = (j + 0.5 - sx0) / w; 
            float v = (i + 0.5 - sy0) / h;
            //Color color = sampler2d->sample_nearest(tex, u, v, 0);
            Color color = sampler2d->sample_bilinear(tex, u, v, 0);
            render_target[4 * (j + i * target_w)] = color.r;
            render_target[4 * (j + i * target_w) + 1] = color.g;
            render_target[4 * (j + i * target_w) + 2] = color.b;
            render_target[4 * (j + i * target_w) + 3] = color.a;
        }
    }

}

// resolve samples to render target
void SoftwareRendererImp::resolve( void ) {

  // Task 4: 
  // Implement supersampling执行重采样
  // You may also need to modify other functions marked with "Task 4".
    //return;
    if (sample_rate <= 1) return;
    int sample_num = sample_rate * sample_rate;//每个格子里有多少采样点
    for (int i = 0; i < target_h; i++) {
        for (int j = 0; j < target_w; j++) {//遍历原图所有像素点
            Color color;
            for (int s = 0; s < sample_rate; s++) {//行
                for (int t = 0; t < sample_rate; t++) {//列
                    color.r += supersample_target[4 * (j * sample_rate + t + i * target_w * sample_num + s * target_w * sample_rate)];
                    color.g += supersample_target[1 + 4 * (j * sample_rate + t + i * target_w * sample_num + s * target_w * sample_rate)];
                    color.b += supersample_target[2 + 4 * (j * sample_rate + t + i * target_w * sample_num + s * target_w * sample_rate)];
                    color.a += supersample_target[3 + 4 * (j * sample_rate + t + i * target_w * sample_num + s * target_w * sample_rate)];
                }
            }
            //做平均，unit-area box filter
            color.r /= sample_num;
            color.g /= sample_num;
            color.b /= sample_num;
            color.a /= sample_num;
            //赋值给原图
            render_target[4 * (j + i * target_w)] = (uint8_t)(color.r);
            render_target[4 * (j + i * target_w) + 1] = (uint8_t)(color.g);
            render_target[4 * (j + i * target_w) + 2] = (uint8_t)(color.b);
            render_target[4 * (j + i * target_w) + 3] = (uint8_t)(color.a);
        }
    }
    cout << "w&h " << target_w << " " << target_h << endl;

}


} // namespace CMU462
