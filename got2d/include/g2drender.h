#pragma once

#include <g2dconfig.h>
#include <gmlvector.h>
#include <gmlmatrix.h>
#include <gmlcolor.h>
#include <gmlrect.h>

namespace g2d
{
	// 半透明混合种类
	enum class G2DAPI BlendMode
	{
		// 不混合
		None,

		// 半透明混合效果
		Normal,

		// 叠加效果
		Additve,

		// 屏幕混合效果
		Screen,
	};

	// Mesh中使用的内存布局
	struct GeometryVertex
	{
		gml::vec2 position;
		gml::vec2 texcoord;
		gml::color4 vtxcolor;
	};

	// 用户自定义模型组件
	// 数据保存在内存中，引擎在渲染过程中
	// 会根据材质使用情况，随机地把内存数据上传到显存里。
	class G2DAPI Mesh : public GObject
	{
	public:
		static Mesh* Create(uint32_t vertexCount, uint32_t indexCount);

		// 释放内存，只能调用一次
		// 用户需要手动调用
		virtual void Release() = 0;

		// 获得顶点数据指针，用户通过这个指针填充数据。
		// 用户需要自己保证数据不超过创建时指定的大小。
		// 否则会造成数组越界
		virtual const GeometryVertex* GetRawVertices() const = 0;
		virtual GeometryVertex* GetRawVertices() = 0;

		// 获得索引数据指针，用户通过这个指针填充数据。
		// 用户需要自己保证数据不超过创建时指定的大小。
		// 否则会造成数组越界
		virtual const uint32_t* GetRawIndices() const = 0;
		virtual uint32_t* GetRawIndices() = 0;

		// 获得顶点数据大小，为GeometryVertex数组的个数
		virtual uint32_t GetVertexCount() const = 0;

		// 获得索引数据数组的个数
		virtual uint32_t GetIndexCount() const = 0;

		// 重设顶点数据大小，原有数据会被自动迁移到新数组上
		virtual void ResizeVertexArray(uint32_t vertexCount) = 0;

		// 重设索引数据大小，原有数据会被自动迁移到新数组上
		virtual void ResizeIndexArray(uint32_t indexCount) = 0;
	};

	// 纹理对象，用来作为渲染资源，是一个共享资源
	// 每次获取对象的时候需要在手动调用 AddRef 增加引用计数
	// 每次不需要资源后需要显式调用 Release 接口释放引用
	class G2DAPI Texture : public GObject
	{
	public:
		// 从文件中读取构建纹理
		// 支持 bmp/png/tga，不支持的类型会返回nullptr
		// 支持透明和不透明的文件格式
		static Texture* LoadFromFile(const char* path);

		// 释放引用计数，每次不需要使用之后需要手动调用
		virtual void Release() = 0;

		// 增加引用计数，当共享纹理的时候需要显式调用。
		virtual void AddRef() = 0;

		// 纹理的标识符，现在纹理只能存文件里读取
		// 标识符等于文件路径
		virtual const char* Identifier() const = 0;

		// 判断两个纹理是否相同
		virtual bool IsSame(Texture* other) const = 0;
	};

	// 材质渲染Pass
	/// 此对象在之后的构建中会被拓展成多个子类
	/// 用户需要使用子类来填充材质数据
	// keep it simple and stupid.
	// there is no other more easy way to define param setting interface elegantly
	// TODO: make it more elegant.
	class G2DAPI Pass : public GObject
	{
	public:
		// 使用的Vertex Shader类型
		virtual const char* GetVertexShaderName() const = 0;

		// 使用的Pixel Shader类型
		virtual const char* GetPixelShaderName() const = 0;

		// 判断两个pass是否完全一致，包括其中的数据
		virtual bool IsSame(Pass* other) const = 0;

		// 设置半透明混合模式
		virtual void SetBlendMode(BlendMode mode) = 0;

		// 填充Vertex Shader常量数据
		virtual void SetVSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) = 0;

		// 填充Pixel Shader常量数据
		virtual void SetPSConstant(uint32_t index, float* data, uint32_t size, uint32_t count) = 0;

		// 设置材质信息，有些纹理不需要自动释放
		// 当Index被拓展的时候，中间没有被设置过的Index会自动填充nullptr
		// 并且TextureCount会被拉长
		// 如果索引中纹理为空，引擎不会设置渲染环境，会沿用之前的纹理状态。
		virtual void SetTexture(uint32_t index, Texture*, bool autoRelease) = 0;

		// 根据Inedx获取设置的纹理
		virtual Texture* GetTextureByIndex(uint32_t index) const = 0;

		// 获取最大的纹理索引，并不是每一个索引都有数据
		virtual uint32_t GetTextureCount() const = 0;

		// 获取 Vertex Shader 常量数据
		virtual const float* GetVSConstant() const = 0;

		// 获取 Vertex Shader 常量数据大小
		virtual uint32_t GetVSConstantLength() const = 0;

		// 获取 Pixel Shader 常量数据
		virtual const float* GetPSConstant() const = 0;

		// 获取 Pixel Shader 常量数据大小
		virtual uint32_t GetPSConstantLength() const = 0;

		// 当前的半透明混合状态
		virtual BlendMode GetBlendMode() const = 0;
	};

	// 物体材质，这个是用来抽象渲染资源的对象
	// 一个材质可能会分为多个不同的Pass（比如阴影会有shaodowmap pass 和 rendering pass）
	// 材质会保存物体数据，一般不能不同的物体通用一个mesh
	class G2DAPI Material : public GObject
	{
	public:
		// 渲染顶点颜色、纹理的材质
		static Material* CreateColorTexture();

		//只渲染纹理的材质
		static Material* CreateSimpleTexture();

		// 只渲染顶点颜色的材质
		static Material* CreateSimpleColor();

		// 释放内存，只能调用一次
		// 用户需要手动调用
		virtual void Release() = 0;

		// 根据索引获取Pass
		// 现在提供的(一般)只有一个Pass
		// 做Filter相关（柔化描边）会有多个Pass
		virtual Pass* GetPassByIndex(uint32_t index) const = 0;

		// 获取材质中Pass数量
		virtual uint32_t GetPassCount() const = 0;

		// 判断两个材质是否完全一致
		virtual bool IsSame(Material* other) const = 0;

		// 复制一个数据、类型完全一致材质
		virtual Material* Clone() const = 0;
	};

	// 渲染分层，2D渲染力很重要的一个环节，一般渲染是可以指定在某个层次之前或者在之后的。
	/// 这个其实可以通过SceneNode的关系去调整，引擎暂时没想好是否需要这个内容
	/// 根据以往的经验先加上
	class G2DAPI RenderLayer
	{
	public:
		constexpr static uint32_t PreZ = 0x1000;
		constexpr static uint32_t BackGround = 0x2000;
		constexpr static uint32_t Default = BackGround + 0x2000;
		constexpr static uint32_t ForeGround = BackGround + 0x4000;
		constexpr static uint32_t Overlay = 0x8000;
	};

	// 渲染系统接口
	class G2DAPI RenderSystem : public GObject
	{
	public:
		// 当nativeWindow发生改变的时候，需要显示调用这个接口
		// 更新渲染 系统相关的状态以修正投影矩阵
		// 过后会改成 引擎初始化的时候，提供一个接口，在引擎内部自己注册listener消息
		virtual bool OnResize(uint32_t width, uint32_t height) = 0;

		// 渲染开始的初始化工作
		// 所有的Render接口都需要在这个接口被调用之后使用
		virtual void BeginRender() = 0;

		// 渲染结束的收尾工作
		// 所有的Render接口都需要在这个接口被调用之前使用
		virtual void EndRender() = 0;

		// 提供确定的Material Mesh注册渲染请求。
		// 一般是用户自定义的Entity在OnRender事件中使用
		virtual void RenderMesh(uint32_t layer, Mesh*, Material*, const gml::mat32&) = 0;

		// nativeWindow 当前的大小
		// 过后会改成 引擎初始化的时候，提供一个接口
		virtual uint32_t GetWindowWidth() const = 0;
		virtual uint32_t GetWindowHeight() const = 0;

		// 从屏幕坐标转到摄像机坐标系
		virtual gml::vec2 ScreenToView(const gml::coord& screen) const = 0;

		// 从摄像机坐标系转换到屏幕坐标系
		virtual gml::coord ViewToScreen(const gml::vec2 & view) const = 0;
	};
}
