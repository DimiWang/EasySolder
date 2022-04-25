/*
EasySolder
Copyright (C) 2015  Dmitry Chikov

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef WORKAREA_H
#define WORKAREA_H

#include <QLabel>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QScrollArea>
#include <QScrollBar>

class WorkArea : public QLabel
{
    Q_OBJECT





    typedef enum  {NONE, CAL_TP1, CAL_TP2} Mode;

public:

    struct LayerSettings{
        LayerSettings(){
            m_reverse_x = false;
            m_reverse_y = false;
            calibrated = false;
        }

        QImage image;
        // test points
        QPointF tp1;
        QPointF tp2;
        QPointF tp1_original;
        QPointF tp2_original;
        QPointF scale;
        QPointF offset;
        bool m_reverse_x;
        bool m_reverse_y;
        bool calibrated;
    }m_layers_list[2];// Layers TOP+BOT

    typedef enum {TOP,BOTTOM} LayerEnum;
    LayerSettings *layer(LayerEnum l){return &m_layers_list[l];}

    explicit WorkArea(QWidget *parent = 0);
    void addPoint(const QPointF& point);
    void clearPoints();
    void setImage(LayerEnum currentLayer,const QImage &image);
    bool switchLayer(LayerEnum layer);
    bool hasTop() const;
    bool hasBottom() const;
    typedef QList<QPointF> PointList;
    PointList m_points;
    void setMarkerColor(Qt::GlobalColor color);
    void setMarkerSize(int marker_size);
    int markerSize() const;
    void enableCircleAround(int size);
    LayerSettings* currentLayer();
    void setReverseX(LayerEnum layer,bool on);
    void setReverseY(LayerEnum layer, bool on);
    QPointF mousePosition() const;
    QPointF mouseAreaPosition();
    QSize currentImageSize();
    QSize scale(double factor);

    QPointF toScreen(const QPointF &p);
    QPointF fromScreen(const QPointF &p);
    QPointF toOriginal(const QPointF &p);
    QPointF fromOriginal(const QPointF &p);
    void calibrate();

protected:
    Mode m_mouse_mode;
    void mousePressEvent(QMouseEvent *ev);
    void paintEvent(QPaintEvent *ev);
    void mouseMoveEvent(QMouseEvent *ev);

    LayerEnum m_current_layer;
    // current point
    QPointF m_current;    
    // test points in original
    Qt::GlobalColor m_marker_color;
    int m_marker_size;
    int m_circle_around;
    bool m_image_loaded;


signals:
    void signal_repainted();
    void signal_info(QString);

public slots:
    void slot_locateTP1_click();
    void slot_locateTP2_click();

};





class WorkScrolledArea: public QScrollArea
{
    Q_OBJECT
public:
    WorkScrolledArea(QWidget *parent = 0);

    void refresh();
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);
    void setNormalSize();
    void setImage(WorkArea::LayerEnum layer,const QImage &image);
    void switchImage(WorkArea::LayerEnum layer);
    void focusAllPoints();
    void focusPoint(qint32 n);
    bool imageAvailable();
    WorkArea* workArea();
    static qreal pointDistance(const QPointF &p1, const QPointF &p2);
    QRect findRect();

public slots:
    void slot_dragging(const QPointF &point);

protected:
    bool eventFilter(QObject *obj, QEvent *ev);
    void wheelEvent(QWheelEvent *);
    WorkArea *mp_work_area;
    bool m_dragging;
    QPoint m_mouse_pos;
};

#endif // WORKAREA_H
