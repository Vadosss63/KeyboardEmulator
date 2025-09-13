#include "ImageZoomWidget.h"

#include <QHBoxLayout>
#include <QPainter>
#include <QPushButton>
#include <QVBoxLayout>
#include <QtMath>

ImageZoomWidget::ImageZoomWidget(QWidget* parent) : QWidget(parent)
{
    m_btnReady = new QPushButton("Готово", this);

    m_btnMinus = new QPushButton("−", this);
    m_btnPlus  = new QPushButton("+", this);
    m_btnMinus->setFixedWidth(36);
    m_btnPlus->setFixedWidth(36);

    auto* buttons = new QHBoxLayout;
    buttons->setContentsMargins(0, 0, 0, 0);
    buttons->setSpacing(8);
    buttons->addStretch();
    buttons->addWidget(m_btnReady);
    buttons->addSpacing(16);
    buttons->addWidget(m_btnMinus);
    buttons->addWidget(m_btnPlus);

    auto* root = new QVBoxLayout(this);
    root->setContentsMargins(8, 8, 8, 8);
    root->setSpacing(8);
    root->addLayout(buttons);
    root->addStretch();

    connect(m_btnReady, &QPushButton::clicked, this, &ImageZoomWidget::zoomReady);

    connect(m_btnPlus, &QPushButton::clicked, this, &ImageZoomWidget::onZoomIn);
    connect(m_btnMinus, &QPushButton::clicked, this, &ImageZoomWidget::onZoomOut);
}

void ImageZoomWidget::setImage(const QPixmap& pixmap)
{
    m_original = pixmap;
    m_scale    = 1.0;
    updateScaled();
}

QPixmap ImageZoomWidget::getResultPixmap() const
{
    return m_scaled;
}

void ImageZoomWidget::setZoomStep(double step)
{
    if (step > 1.0)
    {
        m_step = step;
    }
}

void ImageZoomWidget::setZoomLimits(double minK, double maxK)
{
    if (minK > 0.0 && maxK > minK)
    {
        m_minZoom = minK;
        m_maxZoom = maxK;
        m_scale   = qBound(m_minZoom, m_scale, m_maxZoom);
        const_cast<ImageZoomWidget*>(this)->updateScaled();
    }
}

void ImageZoomWidget::onZoomIn()
{
    if (m_original.isNull())
    {
        return;
    }
    m_scale = qMin(m_scale * m_step, m_maxZoom);
    updateScaled();
}

void ImageZoomWidget::onZoomOut()
{
    if (m_original.isNull())
    {
        return;
    }
    m_scale = qMax(m_scale / m_step, m_minZoom);
    updateScaled();
}

void ImageZoomWidget::updateScaled()
{
    if (m_original.isNull())
    {
        m_scaled = QPixmap();
        update();
        return;
    }
    const QSizeF newSize = m_original.size() * m_scale;
    m_scaled = m_original.scaled(newSize.height(), newSize.width(), Qt::KeepAspectRatio, Qt::SmoothTransformation);
    update();
}

void ImageZoomWidget::paintEvent(QPaintEvent*)
{
    QPainter p(this);
    p.fillRect(rect(), palette().brush(QPalette::Base));

    if (m_scaled.isNull())
    {
        return;
    }

    const int x = (width() - m_scaled.width()) / 2;
    const int y = (height() - m_scaled.height()) / 2;
    p.drawPixmap(x, y, m_scaled);
}
