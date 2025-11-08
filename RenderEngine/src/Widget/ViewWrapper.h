#ifndef VIEWWRAPPER_H
#define VIEWWRAPPER_H

#include <QWidget>
#include "MarchView.h"
#include "RenderWidget.h"

namespace GLRhi
{
    class ViewWrapper : public QWidget
    {
        Q_OBJECT
    public:
        explicit ViewWrapper(QWidget* parent = nullptr);
        ~ViewWrapper();

    private:
        MarchView* m_marchView = nullptr;
        RenderWidget* m_renderWidget = nullptr;
    };
}

#endif // VIEWWRAPPER_H