#pragma once

#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QObject>
#include <array>

class QAction;

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

public slots:
    void setResizable(bool on);

protected:
    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    virtual void extendDerivedContextMenu(QMenu& menu);
    virtual void setupDeleteItemAction(QAction* deleteAction);

    bool isModifyMod() const;

    QRectF rectItem() const;

    void setInfoText(const QString& text);

private slots:
    void handleMoved(int handleIndex, const QPointF& scenePos);

private:
    void addDeleteItemAction(QMenu& menu);
    void initHandles();
    void updateHandles();

    bool m_resizable{};

    QGraphicsTextItem* infoItem{};

    std::array<ResizeHandle*, HandleCount> m_handles{};
};
