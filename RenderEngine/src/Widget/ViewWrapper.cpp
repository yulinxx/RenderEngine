#include "ViewWrapper.h"
#include <QGridLayout>

namespace GLRhi
{
    ViewWrapper::ViewWrapper(QWidget* parent) : QWidget(parent)
    {
        //m_marchView = new MarchView(this);
        m_renderWidget = new RenderWidget(this);

        // 设置焦点策略，确保子部件可以接收键盘事件
        m_renderWidget->setFocusPolicy(Qt::StrongFocus);
        //m_marchView->setFocusPolicy(Qt::StrongFocus);

        // 默认让renderWidget获得焦点
        m_renderWidget->setFocus();

        QGridLayout* layout = new QGridLayout(this);
        //layout->addWidget(m_marchView, 0, 0);
        layout->addWidget(m_renderWidget, 0, 0);
        setLayout(layout);
    }

    ViewWrapper::~ViewWrapper()
    {
    }
}