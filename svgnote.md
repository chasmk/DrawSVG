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

















































