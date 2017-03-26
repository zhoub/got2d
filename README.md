# got2d

got2d是一个“兴趣使然”的2d游戏渲染框架，框架使用c++编写，使用了部分c++ 11的语法。

框架的初衷是用于2d游戏的制作，并有可能会随着莫须有的游戏开发而演变成为一个2d游戏引擎。

框架将向[ubi某引擎](http://www.bilibili.com/video/av1505190/)看齐，与之有区别的是，got2d应该不会打开深度缓存，而单纯的使用画家算法进行2d渲染。

## 现状

got2d在vs2017下编写测试，项目设置保持之前使用vs2015的设置。所有的项目都在 got2d.sln 解决方案里。

现阶段整个框架被分为框架结构、渲染结构、场景结构三个部分，现阶段已经完成的功能如下：

### 场景结构

* 场景节点正确设置局部变换：平移、缩放、旋转、节点中心偏移。
* 场景节点表达父子空间关系：世界坐标、局部坐标，并提供了空间之间的变换接口。
* 场景节点表达父子之间的逻辑关系，提供了父节点、子节点、兄弟节点的获取接口。
* 场景节点和逻辑代码分离：抽象了实体(Entity)、组件（Component）逻辑节点。
* 用户能够自定义Entity，Component节点，支持响应多种不同的键盘鼠标事件。

### 渲染结构

* 具有简单的材质概念，并拥有简单实现。
* 能够通过判断材质信息，合并多个drawcall。
* 根据场景节点的动态/静态设置，使用四叉树的可见性剔除。
* 支持自定义的渲染顺序，可以进行分层渲染。
* 支持简单的半透明渲染（待更新：只实现了功能，还没有明确接口。）
* 具有摄像机的抽象，并支持多摄像机的操作和渲染

### 框架结构

* 定义了鼠标、键盘的事件（待更新：只支持Win32）。并具有完整的世间派发逻辑结构。
* 支持多摄像机条件下的鼠标选取：屏幕到世界、世界到屏幕的坐标变换映射。
* 数学库进行了大量的单元测试，趋于稳定。
* 编写了自定义Entity/Component，响应键盘鼠标消息的测试用例。（笑）

主题框架已经搭建成功，接下来开始根据 milestone 对各个 issues 进行开发。

其中当前的开发现状在 Board里可以看到。接下来要进行的工程可以在Issue里查询到。

## 文件内容

got2d 为了配置简单，原则是尽量不使用第三方库和其他(如cmake)依赖。可以直接通过vs2015打开Solution，直接编译运行。文件内容分为三部分：

* got2d 这个是框架实现
* testbed 用来测试的win32程序
* extern 这个是第三方依赖库的目录，你需要通过submodule初始化并下载它们。现在包括两步分：
  * gml 一个简单的数学运算库。
  * res 一个简单的图像读取库，支持24/32位未压缩的TGA/BMP/PNG三种格式。

 
## 特别鸣谢：

感谢 `???`、`婉君`、`MacKong` 对代码与文档的贡献，和平日的大力支持。

第一次弄开源软件经验有限，欢迎各位来搞，多提宝贵意见。

