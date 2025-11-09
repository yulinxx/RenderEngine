#include "RenderManager.h"
#include <QDebug>
#include <cassert>
#include <memory>

#include "FakeData/FakePolyLineData.h"
#include "FakeData/FakeTriangleData.h"
#include "FakeData/FakeTextureData.h"
#include "Common/TextureLoader.h"

#include "Common/Tools.h"

namespace GLRhi
{
    RenderManager::RenderManager()
    {
        // 初始化默认背景色
        m_bgColor = { 1.0f, 1.0f, 0.0f, 1.0f };

        // 使用std::unique_ptr替代原始指针，实现自动内存管理
        m_boardRenderer = std::make_unique<CheckerboardRenderer>();
        m_lineRenderer = std::make_unique<LineRenderer>();
        //m_lineRenderer = std::make_unique<LineRendererUbo>();
        m_lineBRenderer = std::make_unique<LineBRenderer>();
        m_triRenderer = std::make_unique<TriangleRenderer>();
        m_imageRenderer = std::make_unique<ImageRenderer>();
        m_texRenderer = std::make_unique<TextureRenderer>();
        m_instancedTexRenderer = std::make_unique<InstancedTextureRenderer>();
    }

    RenderManager::~RenderManager()
    {
        cleanup();
        // std::unique_ptr会自动管理内存，无需手动释放
    }

    bool RenderManager::initialize(QOpenGLFunctions_3_3_Core* gl)
    {
        m_gl = gl;
        if (!m_gl)
        {
            qWarning() << "RenderManager::initialize: OpenGL functions not available";
            assert(!"RenderManager initialize: OpenGL functions not available");

            return false;
        }

        // 初始化所有渲染器
        bool success = true;
        success &= m_boardRenderer->initialize(m_gl);
        success &= m_lineRenderer->initialize(m_gl);
        success &= m_lineBRenderer->initialize(m_gl);
        success &= m_triRenderer->initialize(m_gl);
        success &= m_imageRenderer->initialize(m_gl);
        success &= m_texRenderer->initialize(m_gl);
        success &= m_instancedTexRenderer->initialize(m_gl);

        if (!success)
        {
            qWarning() << "RenderManager::initialize: Failed to initialize one or more renderers";
            return false;
        }

        genFakeData();

        return true;
    }

    void RenderManager::render(const float* cameraMat)
    {
        if (!m_gl)
            return;

        // 设置背景色
        m_gl->glClearColor(m_bgColor.r(), m_bgColor.g(), m_bgColor.b(), m_bgColor.a());
        m_gl->glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        //float mat[9] = { 1.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 1.0f };
        constexpr float mat[9] = {
        2.0f / 879.0f, 0.0f, 0.0f,
        0.0f, 2.0f / 1368.0f, 0.0f,
        0.0f, 0.0f, 1.0f
        };

        //m_boardRenderer->render(mat);
        //m_triRenderer->render(mat);
        //m_lineRenderer->render(mat);
        //m_lineBRenderer->render(mat);
        //m_imageRenderer->render(mat);
        m_texRenderer->render(mat);
        m_instancedTexRenderer->render(mat);
    }

    void RenderManager::cleanup()
    {
        m_boardRenderer->cleanup();
        m_lineRenderer->cleanup();
        m_lineBRenderer->cleanup();
        m_triRenderer->cleanup();
        m_imageRenderer->cleanup();
        m_texRenderer->cleanup();
        m_instancedTexRenderer->cleanup();

        m_gl = nullptr;
    }

    void RenderManager::genFakeData()
    {
        // Line
        if (0)
        {
            std::vector<PolylineData> vPLineDatas;

            FakePolyLineData fakePolyLineData;
            for (size_t i = 0; i < 20; ++i) // Group
                //for (size_t i = 0; i < 2; ++i) // Group
            {
                // fakePolyLineData.setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);

                fakePolyLineData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
                fakePolyLineData.generateLines(100000, 2, 15);

                std::vector<float> datas = fakePolyLineData.getVertices();
                std::vector<size_t> lineVertexCounts = fakePolyLineData.getLineInfos();

                float r = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float g = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float b = fakePolyLineData.getRandomFloat(0.0f, 1.0f);
                float d = fakePolyLineData.getRandomFloat(-1.0f, 1.0f);

                PolylineData polyLineData{ datas, lineVertexCounts, {r, g, b, 1.0, d} };
                //PolylineData polyLineData{ datas, lineVertexCounts, {r, 0.0, 0.0, 1.0, d} };

                vPLineDatas.emplace_back(polyLineData);
            }

            //static_cast<LineRendererUbo*>(m_lineRenderer.get())->updateData(vPLineDatas);
            static_cast<LineRenderer*>(m_lineRenderer.get())->updateData(vPLineDatas);
        }

        // Triangle
        if (0)
        {
            // 准备数据
            std::vector<GLRhi::TriangleData> vTriDatas(2);

            // 第一个三角形批次
            vTriDatas[0].vertices = { 0.0f, 0.0f, 0.0f,  1.0f, 0.0f, 0.0f,  0.0f, 1.0f, 0.0f };
            vTriDatas[0].indices = { 0, 1, 2 };
            vTriDatas[0].brush = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0 };

            // 第二个三角形批次
            vTriDatas[1].vertices = { 2.0f, 0.0f, 0.0f,  3.0f, 0.0f, 0.0f,  2.0f, 1.0f, 0.0f };
            vTriDatas[1].indices = { 0, 1, 2 };
            vTriDatas[1].brush = { 0.0f, 1.0f, 0.0f, 1.0f, 0.0f, 0 };

            static_cast<TriangleRenderer*>(m_triRenderer.get())->updateData(vTriDatas);
        }

        if (0)
        {
            std::vector<TriangleData> vTriDatas;
            vTriDatas.reserve(10);

            for (size_t i = 0; i < 100; ++i) // Group
            {
                FakeTriangleData fakeTriangleData;
                // fakeTriangleData.setRange(-1000.0f, 1000.0f, -1000.0f, 1000.0f);
                fakeTriangleData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
                fakeTriangleData.generateTriangles(10000);

                TriangleData triData{};
                triData.vertices = fakeTriangleData.getVertices();
                triData.indices = fakeTriangleData.getIndices();

                float r = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float g = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float b = fakeTriangleData.getRandomFloat(0.0f, 1.0f);
                float a = fakeTriangleData.getRandomFloat(0.3f, 1.0f);
                float d = fakeTriangleData.getRandomFloat(-1.0f, 1.0f);
                triData.brush = { r, g, b, a, 0.0f, 0 };

                vTriDatas.push_back(triData);
            }
            static_cast<TriangleRenderer*>(m_triRenderer.get())->updateData(vTriDatas);
        }

        if (0)
        {
            GLuint tex1, tex2;

            std::vector<TextureData> vTexDatas(2);

            vTexDatas[0].vertices = {
                -0.8f, -0.2f, 0.0f, 0.0f,  // x,y,u,v
                -0.4f, -0.2f, 1.0f, 0.0f,
                -0.4f, 0.2f, 1.0f, 1.0f,
                -0.8f, 0.2f, 0.0f, 1.0f
            };
            vTexDatas[0].indices = { 0, 1, 2, 0, 2, 3 };
            vTexDatas[0].textureId = tex1;
            vTexDatas[0].brush = { 1.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0 };

            vTexDatas[1].vertices = {
                0.4f, -0.2f, 0.0f, 0.0f,  // x,y,u,v
                0.8f, -0.2f, 1.0f, 0.0f,
                0.8f, 0.2f, 1.0f, 1.0f,
                0.4f, 0.2f, 0.0f, 1.0f
            };

            vTexDatas[1].indices = { 0, 1, 2, 0, 2, 3 };
            vTexDatas[1].textureId = tex2;

            // m_texRenderer->updateData(vTexDatas);
        }

        if (0)
        {
            FakeTextureData fakeTextureData;
            fakeTextureData.setRange(-1.0f, 1.0f, -1.0f, 1.0f);
            fakeTextureData.setTextureSizeRange(32, 256, 32, 256);
            fakeTextureData.generateTextures(10, m_gl);

            //fakeTextureData.clear(m_gl); // 清理内存

            std::vector<TextureData> vTexDatas = fakeTextureData.getTextureDatas();
            static_cast<TextureRenderer*>(m_texRenderer.get())->updateData(vTexDatas);
        }

        if (1)
        {
            // 使用TextureLoader加载多张图片到纹理数组
            QStringList imagePaths = {
                "D:/xx/Pictures/34.png",
                "D:/xx/Pictures/34.png",
                "D:/xx/Pictures/34.png",
                "D:/xx/Pictures/34.png",
                "D:/xx/Pictures/34.png",
                "D:/xx/Pictures/34.png",
            };

            // 转换QStringList为std::vector<QString>
            std::vector<QString> imagePathVec = imagePaths.toVector().toStdVector();

            if (!imagePathVec.empty())
            {
                // 调用正确的createTextureArray方法，添加width和height参数
                GLuint textureArrayId = GLRhi::TextureLoader::createTextureArray(
                    imagePathVec,
                    m_gl,
                    256,  // width
                    256,  // height
                    GL_CLAMP_TO_EDGE,
                    GL_LINEAR
                );

                if (textureArrayId > 0)
                {
                    qDebug() << "成功创建纹理数组，层数:" << imagePathVec.size();

                    // 设置纹理数组
                    static_cast<InstancedTextureRenderer*>(m_instancedTexRenderer.get())->setTextureArray(textureArrayId, imagePathVec.size());

                    // 创建实例数据
                    std::vector<InstancedTextureData> instances;
                    float spacing = 0.3f; // 每个实例之间的间距
                    float size = 0.2f;    // 实例大小
                    int cols = 5;         // 列数

                    // 生成网格布局的实例
                    for (int i = 0; i < 20; ++i)
                    { // 生成20个实例
                        InstancedTextureData instance;

                        // 计算网格位置
                        int col = i % cols;
                        int row = i / cols;

                        // 计算实际位置（居中显示）
                        instance.x = -0.7f + col * spacing;
                        instance.y = 0.7f - row * spacing;

                        // 设置大小
                        instance.width = size;
                        instance.height = size;

                        // 循环使用纹理层
                        instance.textureLayer = i % imagePathVec.size();

                        // 设置透明度
                        instance.alpha = 1.0f;

                        instances.push_back(instance);
                    }

                    // 更新实例数据
                    static_cast<InstancedTextureRenderer*>(m_instancedTexRenderer.get())->updateInstances(instances);
                }
                else
                {
                    qWarning() << "Failed to create texture array.";
                }
            }
            else
            {
                qWarning() << "Error: No valid image paths provided.";
            }
        }

        if (0)
        {
            // 使用TextureLoader加载指定图片
            QString imagePath = "D:/xx/Pictures/34.png";
            GLuint textureId = GLRhi::TextureLoader::loadTextureFromFile(
                imagePath, m_gl, true, GL_CLAMP_TO_EDGE, GL_LINEAR);

            if (textureId > 0)
            {
                qDebug() << "成功加载图片:" << imagePath;

                // 创建纹理数据
                std::vector<TextureData> vTexDatas;
                TextureData texData;

                // 设置顶点数据（-0.5到0.5范围，居中显示）
                texData.vertices = {
                    -0.5f, -0.5f, 0.0f, 0.0f,  // 左下角
                    0.5f, -0.5f, 1.0f, 0.0f,   // 右下角
                    0.5f, 0.5f, 1.0f, 1.0f,    // 右上角
                    -0.5f, 0.5f, 0.0f, 1.0f    // 左上角
                };

                // 设置索引数据（两个三角形组成矩形）
                texData.indices = { 0, 1, 2, 0, 2, 3 };

                // 设置纹理ID和画笔颜色
                texData.textureId = textureId;
                texData.brush = { 1.0f, 1.0f, 1.0f, 1.0f, 0.0f, 0 }; // 白色，不透明

                vTexDatas.push_back(texData);

                // 更新到渲染器
                static_cast<TextureRenderer*>(m_texRenderer.get())->updateData(vTexDatas);
            }
            else
            {
                qWarning() << "加载图片失败:" << imagePath;
            }
        }
    }
}