#pragma once

#include <QBrush>
#include <QGraphicsRectItem>
#include <QGraphicsTextItem>
#include <QObject>
#include <QPainter>
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

    bool isActive() const;
    void setActive(bool active);

    void setCircularShape(bool circular);

    bool isCircular() const;

    virtual ResizableRectItem* clone() const = 0;

signals:
    void itemCopied(ResizableRectItem* item);

public slots:
    void setResizable(bool on);

    void makeRectShape();

protected:
    void paint(QPainter* painter, const QStyleOptionGraphicsItem* option, QWidget* widget) override;

    QPainterPath shape() const override;

    QVariant itemChange(GraphicsItemChange change, const QVariant& value) override;

    void contextMenuEvent(QGraphicsSceneContextMenuEvent* event) override;

    virtual void extendDerivedContextMenu(QMenu& menu);
    virtual void setupDeleteItemAction(QAction* deleteAction);

    bool isModifyMod() const;

    QRectF rectItem() const;

    void setInfoText(const QString& text);

    void   setColor(const QColor& color);
    QColor color() const;

    void updateAppearance();

private slots:
    void handleMoved(int handleIndex, const QPointF& scenePos);

    void changeColor();

private:
    void addDeleteItemAction(QMenu& menu);
    void initHandles();
    void updateHandles();

    void updateRect(const QRectF& rect);

    bool m_circular = false;

    bool m_active = false;

    QBrush m_normalBrush;
    QBrush m_activeBrush;

    QColor m_color{Qt::green};

    bool m_resizable{};

    QGraphicsTextItem* infoItem{};

    std::array<ResizeHandle*, HandleCount> m_handles{};
};
