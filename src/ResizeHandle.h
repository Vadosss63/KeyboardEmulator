#pragma once

#include <QGraphicsRectItem>
#include <QObject>

class ResizableRectItem;

class ResizeHandle : public QObject, public QGraphicsRectItem
{
    Q_OBJECT
public:
    ResizeHandle(ResizableRectItem* parent, int idx);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

signals:
    void moved(int handleIndex, const QPointF& scenePos);

private:
    ResizableRectItem* m_parent{nullptr};

    int m_index;
};
