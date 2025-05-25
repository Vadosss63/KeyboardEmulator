#pragma once

#include <QGraphicsRectItem>
#include <QObject>
#include <array>

class ResizeHandle;

class ResizableRectItem : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    enum HandlePos
    {
        TopLeft = 0,
        TopRight,
        BottomRight,
        BottomLeft,
        HandleCount
    };

    ResizableRectItem(qreal x, qreal y, qreal w, qreal h, QGraphicsItem* parent = nullptr);

    void setResizable(bool on);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

private slots:
    void handleMoved(int handleIndex, const QPointF& scenePos);

private:
    void initHandles();

    void updateHandles();

    bool m_resizable = false;

    std::array<ResizeHandle*, HandleCount> m_handles{};
};
