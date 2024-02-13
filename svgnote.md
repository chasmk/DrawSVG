# 预备知识

- `software_renderer.cpp`

  - 大多数工作在这个文件里实施类`SoftwareRendererImp`
  - `draw_svg( SVG& svg )`方法接受一个`SVG`对象并绘制
    - 一个**SVG**文件定义了canvas(它定义了2D坐标空间，**width和height**)并指定了一系列的shape elements(如点，线，三角形，图像)需要绘制在canvas上
    - 每个**shape elements** `SVGElement`都有一些**style**参数（如color），还有modeling **transform**用来确定element在canvas上的位置

  - `set_render_target()`
    - 为代码提供了一个与输出图像相对应的**buffer**（它还以像素为单位提供了缓冲区的宽度和高度，这两个值在本地存储为 **target_w** 和 **target_h**）
    - 在许多应用程序中，这个缓冲区通常被称为 "render target"，因为它是渲染命令的 "target"。
    - 我们之所以在这里使用像素一词，是因为该buffer中的**值**就是将**显示在屏幕上的值**。像素值以**行**主格式存储，每个像素为 8 位 RGBA 值（共 32 位）。当需要绘制 SVG 文件时，您的程序需要填写该缓冲区的内容。
    - 每当用户**resize**应用程序窗口时，**都会调用** `set_render_target()`

- 绘制点的例子
  - `draw_svg()`->`draw_element()`->`draw_point()`->`rasterize_point()`
  - 在`rasterize_point()`中将点的color填充到render_target相应的位置上

# Task 1: Hardware Renderer

- 调用opengl api即可
- 参考https://nehe.gamedev.net/tutorial/lessons_01__05/22004/

# Task 2 : Drawing Lines

- 参考 
  - http://www.cs.helsinki.fi/group/goa/mallinnus/lines/bresenh.html
  - https://zh.wikipedia.org/zh-cn/%E5%B8%83%E9%9B%B7%E6%A3%AE%E6%BC%A2%E5%A7%86%E7%9B%B4%E7%B7%9A%E6%BC%94%E7%AE%97%E6%B3%95
- 使用 Bresenham's algorithm
  - 原始算法支持斜率0-1之间的直线绘制，每次x+1，然后根据error和m判断y是否+1（斜率[-1, 0] 时y是-1）
  - 然后通过交换x y值 将x递减和斜率大于1和小于-1的情况统一到原始算法中

# Task 3 :Draw Triangles

- 采样点需要在**0.5-0.5**的位置
- 假定三角形**边上的样本点**被三角形覆盖
- 至少应将覆盖率测试限制在三角形的屏幕空间**边界框**内的样本

# Task 4 : 使用超采样来反走样

- 分别实现`rasterize_point(), rasterize_line(), rasterize_triangle()`的超采样版本，其中点和线之间迁移即可，三角形需要在超采样点上重新判断
- `resolve()`函数做重采样，恢复到原始`render_target`大小
- 注意
  - 这个task里主要是超采样的位置比较绕，需要仔细
  - 超采样数组`supersample_target`需要**放在Imp类**里，不能放在父类，否则另一个子类Ref类会报内存越界**错误**。

# Task 5 : modeling变换和viewing变换

## modeling transform

- 每个SVG Element里都有一个transform矩阵，代表这个2D element的变换
- 每次绘制前在全局的transformation矩阵 右乘 element矩阵，绘制完后再乘其逆矩阵撤销操作
  - 绘制group类型的elements时需要额外应用transformation，因为它是嵌套操作

## viewing transform

- 这个任务是实现大小 缩放**scale** 和 平移**translation**

  - 三个参数`centerX`，`centerY`，`vspan`。前两个和平移有关代表中心位置，第三个和缩放有关代表显示的范围

  - 需要调用`set_svg_2_norm()`来更新`svg_2_norm`矩阵

  - 这个transform包括translation和scale，需要将SVG canvas坐标空间映射到标准化的设备坐标空间

    - 左上角映射到(0, 0)，右下角映射到(1, 1)

  - ```c++
    Matrix3x3 m = Matrix3x3::identity();
    
      //设置平移(向右下)，先做缩放，再平移到中心
      m(0, 2) = (vspan - centerX) / (2.0 * vspan);
      m(1, 2) = (vspan - centerY) / (2.0 * vspan);
       
      //设置缩放，缩小 vspan*2 倍，即以vspan为边界值
      m(0, 0) = 1.0 / (2.0 * vspan);
      m(1, 1) = 1.0 / (2.0 * vspan);
    
      set_svg_2_norm(m);//先缩放，再平移
    ```

  - 

- ★最终对svg里坐标的transformation矩阵如下
  - `transformation` = `norm_screen` * `svg_norm` * `element_transform`
  - 上面三个分别是projection变换，view视口变换，model模型变换
  - 对svg中坐标u应用： `u` = `tansformation` * `u`
  - 所以变换矩阵是从**最右边开始应用**的

# Task 6 : Drawing Scaled Images

- 这个任务是应用纹理映射，把texture插值显示到屏幕上
- 步骤
  - 首先遍历绘制区域的所有像素点，计算每个点的uv值 [0, 1]
  - 然后取texure里第0级的mipmap，把uv值映射到纹理坐标下
  - 使用插值算法 最近邻/双线性 得到texture上的color
    - 最近邻：取里UV点最近的纹理像素
    - 双线性，取四周的四个纹理像素做双线性插值
  - 把color应用到像素上





































































































