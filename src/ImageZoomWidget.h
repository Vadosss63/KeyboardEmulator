#pragma once

#include <QPixmap>
#include <QWidget>

class QPushButton;

class ImageZoomWidget : public QWidget
{
    Q_OBJECT
public:
    explicit ImageZoomWidget(QWidget* parent = nullptr);

    void    setImage(const QPixmap& pixmap);
    QPixmap getResultPixmap() const;

    void setZoomStep(double step);
    void setZoomLimits(double minK, double maxK);

signals:
    void zoomReady();

private slots:
    void onZoomIn();
    void onZoomOut();

protected:
    void  paintEvent(QPaintEvent*) override;
    QSize sizeHint() const override { return {400, 300}; }

private:
    void updateScaled();

    QPixmap m_original;
    QPixmap m_scaled;
    double  m_scale   = 1.0;
    double  m_minZoom = 0.1;
    double  m_maxZoom = 10.0;
    double  m_step    = 1.1;

    QPushButton* m_btnReady = nullptr;

    QPushButton* m_btnPlus  = nullptr;
    QPushButton* m_btnMinus = nullptr;
};
