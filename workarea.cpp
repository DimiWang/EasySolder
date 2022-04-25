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
#include "workarea.h"
#include <QDebug>
#include <QAction>
#include <QApplication>
#include <QInputDialog>

QMenu *menu ;
double screenScaleFactor;

/** @brief
 *
 *
 *  @param
 *  @return
 */
WorkArea::WorkArea(QWidget *parent) :
    QLabel(parent)
{
    QAction *action = new QAction("RefPoint1", this);
    connect(action, SIGNAL(triggered()), this, SLOT(slot_locateTP1_click()));
    menu = new QMenu();
    menu->addAction(action);
    action = new QAction("RefPoint2", this);
    connect(action, SIGNAL(triggered()), this, SLOT(slot_locateTP2_click()));
    menu->addAction(action);
    setSizePolicy(QSizePolicy::Ignored, QSizePolicy::Ignored);
    setScaledContents(true);
    setBackgroundRole(QPalette::Base);
    setMouseTracking(true);
    m_mouse_mode = NONE;
    m_marker_color = Qt::red;
    m_image_loaded = false;
    m_current_layer = TOP;
    m_layers_list[TOP].calibrated =false;
    m_layers_list[BOTTOM].calibrated =false;
    m_circle_around = 50; //off
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::addPoint(const QPointF &point)
{
    m_points.append(point);
    repaint();
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::clearPoints()
{
    m_points.clear();
    repaint();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::setImage(LayerEnum layer,const QImage &image)
{
    m_layers_list[layer].image = image;

    if(hasTop()) switchLayer(TOP);
    else
    {
        switchLayer(BOTTOM);
    }
    m_image_loaded  = hasTop() || hasBottom();
    adjustSize();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
bool WorkArea::switchLayer(WorkArea::LayerEnum layer)
{
    if(!m_layers_list[layer].image.isNull())
    {
        setPixmap(QPixmap::fromImage(m_layers_list[layer].image));
        m_current_layer = layer;
        clearPoints();
        adjustSize();
        return true;
    }
    return false;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
bool WorkArea::hasTop() const
{
    return !m_layers_list[TOP].image.isNull();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
bool WorkArea::hasBottom() const
{
    return !m_layers_list[BOTTOM].image.isNull();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::setMarkerColor(Qt::GlobalColor color)
{
    m_marker_color = color;
}

void WorkArea::setMarkerSize(int marker_size)
{
    m_marker_size = marker_size;
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::mousePressEvent(QMouseEvent *ev)
{
    if (ev->button() & Qt::LeftButton)
    {
        if (m_mouse_mode == NONE)
        {

        } else if (m_mouse_mode == CAL_TP1)
        {
            QApplication::restoreOverrideCursor();
            LayerSettings *layer = currentLayer();
            layer->tp1 = fromScreen(QPointF(ev->pos()));            
            calibrate();
            repaint();
            m_mouse_mode = NONE;
            qDebug()<<"TP1"<<layer->tp1<<layer->tp1_original;
        }
        else if (m_mouse_mode == CAL_TP2)
        {
            QApplication::restoreOverrideCursor();
            LayerSettings *layer = currentLayer();
            layer->tp2 = fromScreen(QPointF(ev->pos()));            
            calibrate();
            repaint();
            m_mouse_mode = NONE;
            qDebug()<<"TP2"<<layer->tp2<<layer->tp1_original;
        }
    }
    if (ev->button() & Qt::RightButton)
    {
        menu->popup(mapToGlobal(ev->pos()));
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
QPointF WorkArea::toOriginal(const QPointF &p)
{
    QPointF result;
    LayerSettings *layer = currentLayer();
    result.setX( (p.x()) / layer->scale.x() - layer->offset.x());
    result.setY( (p.y()) / layer->scale.y() - layer->offset.y());
    return result;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
QPointF WorkArea::fromOriginal(const QPointF &p)
{
    QPointF result;
    LayerSettings *layer = currentLayer();
    result.setX( (p.x() + layer->offset.x()) * layer->scale.x());
    result.setY( (p.y() + layer->offset.y()) * layer->scale.y());
    return result;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
WorkArea::LayerSettings *WorkArea::currentLayer()
{
    LayerSettings *result = NULL;
    if(m_current_layer == TOP || m_current_layer == BOTTOM)
    {
        result = &m_layers_list[m_current_layer];
    }
    Q_ASSERT(result);
    return result;
}

void WorkArea::setReverseX(LayerEnum layer, bool on)
{
    LayerSettings *player = &m_layers_list[layer];
    player->m_reverse_x = on;
    calibrate();
    this->repaint();
}

void WorkArea::setReverseY(LayerEnum layer,bool on)
{
    LayerSettings *player = &m_layers_list[layer];
    player->m_reverse_y = on;
    calibrate();
    this->repaint();
}

QPointF WorkArea::mousePosition() const
{
    return mapFromGlobal(QCursor::pos());
}

QPointF WorkArea::mouseAreaPosition()
{
    return fromScreen(mousePosition() );
}

QSize WorkArea::currentImageSize()
{
    LayerSettings *layer = currentLayer();
    return layer->image.size();
}

QSize WorkArea::scale(double factor)
{
    resize(currentImageSize()*factor);
    return size();
}


/** @brief
 *
 *
 *  @param
 *  @return
 */
QPointF WorkArea::toScreen(const QPointF &p)
{
    QPointF result = p;
     LayerSettings *player = currentLayer();
    if(!player->m_reverse_y) result.setY(pixmap()->height() - p.y());
    if(player->m_reverse_x) result.setX(pixmap()->width() - p.x());
    result = result * screenScaleFactor;
    return result;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
QPointF WorkArea::fromScreen(const QPointF &p)
{
    QPointF result = p;
     LayerSettings *player = currentLayer();
    result = result / screenScaleFactor;
    if(!player->m_reverse_y) result.setY(pixmap()->height() - result.y());
    if(player->m_reverse_x)  result.setX(pixmap()->width() - result.x());
    return result;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::paintEvent(QPaintEvent *ev)
{
    if(m_image_loaded)
    {
        QLabel::paintEvent(ev);
        LayerSettings *layer = currentLayer();
        QPainter painter(this);
        if( m_points.size() )
        {
            QPen pen( m_marker_color);
            QBrush brush(m_marker_color);
            painter.setPen(pen);
            painter.setBrush(brush);
            QList<QPointF>::const_iterator i = m_points.constBegin();

            // points
            while ( i != m_points.constEnd() )
            {
                QPointF point = *i;
                point = point + layer->offset ;
                point.setX(point.x() *layer->scale.x());
                point.setY(point.y() *layer->scale.y());
                QPoint screen_point = toScreen(point).toPoint();
                if(!screen_point.isNull() && rect().contains(screen_point))
                    painter.drawEllipse(screen_point, m_marker_size, m_marker_size);
                ++i;
            }
            // circle around            
            if(m_circle_around >m_marker_size)
            {
                i = m_points.constBegin();
                while ( i != m_points.constEnd() )
                {
                    QPointF point = *i;
                    point = point + layer->offset ;
                    point.setX(point.x() *layer->scale.x());
                    point.setY(point.y() *layer->scale.y());
                    QColor transparent_color(m_marker_color);
                    transparent_color.setAlpha(25);
                    QPen pen( transparent_color);
                    QBrush brush(transparent_color);
                    painter.setBrush(brush);
                    painter.setPen(pen);
                    QPoint screen_point = toScreen(point).toPoint();
                    if(!screen_point.isNull() && rect().contains(screen_point))
                        painter.drawEllipse(toScreen(point), m_circle_around, m_circle_around);
                    ++i;
                }
            }
        }        
        // reference points
        if(layer->tp1.isNull() ||layer->tp2.isNull() )
        {
            painter.setPen(QPen(Qt::red));
            QFont font = painter.font();
            font.setPixelSize(60);
            painter.setFont(font);
            QString text = "! SET";
            if(layer->tp1.isNull()) text+="  REF1";
            if(layer->tp2.isNull()) text+="  REF2";
            painter.drawText(width()/2,height()/2,text);
        }
        else {
            QPen pen(Qt::darkCyan);
            pen.setWidth(2);
            painter.setPen(pen);

            painter.drawLine(toScreen(QPointF(layer->tp1.x() - 15, layer->tp1.y()))
                             , toScreen(QPointF(layer->tp1.x() + 15, layer->tp1.y())));
            painter.drawLine(toScreen(QPointF(layer->tp1.x(), layer->tp1.y() - 15))
                             , toScreen(QPointF(layer->tp1.x(), layer->tp1.y() + 15)));
            painter.drawLine(toScreen(QPointF(layer->tp2.x() - 15, layer->tp2.y()))
                             , toScreen(QPointF(layer->tp2.x() + 15, layer->tp2.y())));
            painter.drawLine(toScreen(QPointF(layer->tp2.x(), layer->tp2.y() - 15))
                             , toScreen(QPointF(layer->tp2.x(), layer->tp2.y() + 15)));
        }
        painter.end();
        emit signal_repainted();
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::mouseMoveEvent(QMouseEvent *ev)
{    
    // milimeters
    m_current = toOriginal(fromScreen(mousePosition()));
    char tmp[100];
    if(m_layers_list[m_current_layer].calibrated )
    {
        sprintf(tmp, "x:%2.2f\ty:%2.2f", m_current.x(), m_current.y());
        emit signal_info(QString(tmp));
    }
    else
    {
        emit signal_info(QString("NOT CALIBRATED"));
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::calibrate()
{
    //calculate scale to original
    LayerSettings *layer = currentLayer();
    qreal sx = qAbs((layer->tp1.x() - layer->tp2.x())
                    / (layer->tp1_original.x() - layer->tp2_original.x()));
    qreal sy = qAbs((layer->tp1.y() - layer->tp2.y())
                    / (layer->tp1_original.y() - layer->tp2_original.y()));
    layer->scale.setX(sx);
    layer->scale.setY(sy);
    layer->offset.setX(layer->tp1.x() / sx - layer->tp1_original.x());
    layer->offset.setY(layer->tp1.y() / sx - layer->tp1_original.y());
    layer->calibrated = true;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::slot_locateTP1_click()
{
    QApplication::setOverrideCursor(Qt::CrossCursor);
    m_mouse_mode = CAL_TP1;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkArea::slot_locateTP2_click()
{
    QApplication::setOverrideCursor(Qt::CrossCursor);
    m_mouse_mode = CAL_TP2;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
WorkScrolledArea::WorkScrolledArea(QWidget *parent)
    : QScrollArea(parent)
{
    setBackgroundRole(QPalette::Dark);
    mp_work_area = new WorkArea;
    setWidget( mp_work_area);
    screenScaleFactor = 1.0;
    qApp->installEventFilter(this);
    m_dragging = false;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::refresh()
{
    mp_work_area->repaint();
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::scaleImage(double factor)
{
    QPointF prev_pos = mp_work_area->mouseAreaPosition();
    screenScaleFactor = factor;
    mp_work_area->scale(screenScaleFactor);
    horizontalScrollBar()->setMaximum(mp_work_area->width() - width());
    verticalScrollBar()->setMaximum(mp_work_area->height() - height());
    QPointF new_pos = mp_work_area->mouseAreaPosition();
    QPointF screen_new = mp_work_area->toScreen(new_pos);
    QPointF screen_prev = mp_work_area->toScreen(prev_pos);
     horizontalScrollBar()->setValue(horizontalScrollBar()->value() + screen_prev.x()-screen_new.x());
     verticalScrollBar()->setValue(verticalScrollBar()->value() +screen_prev.y()-screen_new.y());
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * (qreal)scrollBar->value()
                            + ((factor - 1) * (qreal)scrollBar->pageStep() / 2)));

}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::wheelEvent(QWheelEvent *pev)
{
    static qint32 delta = 0;
    double factor=0;
    m_mouse_pos = pev->pos();
    delta += pev->delta();

    if (delta >= 120 || delta <= -120)
    {
        factor = (double)delta / 1200;
        if (screenScaleFactor < 0.1)
        {
            screenScaleFactor = 0.2;
            factor=0;
        }
        else if (screenScaleFactor > 5)
        {
            screenScaleFactor = 5;
            factor=0;
        }
        scaleImage(screenScaleFactor+factor);
        delta %= 120;
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::setNormalSize()
{    
    scaleImage(1.0);
}


/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::setImage(WorkArea::LayerEnum layer, const QImage &image)
{
    if (mp_work_area)
    {
        mp_work_area->setImage(layer,image);
    }
}


/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::switchImage(WorkArea::LayerEnum layer)
{    
    if(mp_work_area->switchLayer(layer))
        scaleImage(screenScaleFactor);
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::focusAllPoints()
{
    // QRect r = findRect();

    //scaleImage();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
WorkArea *WorkScrolledArea::workArea()
{
    return mp_work_area;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
qreal WorkScrolledArea::pointDistance(const QPointF &p1, const QPointF &p2)
{
    qreal result = (p1.x() - p2.x()) * (p1.x() - p2.x()) + (p1.y() - p2.y()) * (p1.y() - p2.y());
    return result;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
QRect WorkScrolledArea::findRect()
{
    qreal  maximum = -999999;
    qreal  minimum = 999999;
    QPointF max;
    QPointF min;
    QList<QPointF>::const_iterator i = mp_work_area->m_points.constBegin();
    while (i != mp_work_area->m_points.constEnd())
    {
        QPointF point = *i;
        qreal dist = pointDistance(QPointF(0, 0), point);
        if (dist < minimum ) {minimum = dist; min = *i;}
        ++i;
    }
    i = mp_work_area->m_points.constBegin();
    while (i != mp_work_area->m_points.constEnd())
    {
        QPointF point = *i;
        qreal dist = pointDistance(min, point);
        if (dist > maximum ) {maximum = dist; max = *i;}
        ++i;
    }
    return QRect(min.toPoint(), max.toPoint());
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void WorkScrolledArea::slot_dragging(const QPointF &point)
{
    verticalScrollBar()->setValue(verticalScrollBar()->value() + point.y());
    horizontalScrollBar()->setValue(horizontalScrollBar()->value() + point.x());
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
bool WorkScrolledArea::eventFilter(QObject *obj, QEvent *ev)
{
    static QPoint prev_point;
    if (obj == mp_work_area)
    {
        QMouseEvent * mouseEvent = static_cast <QMouseEvent *> (ev);

        {
            if (ev->type() == QEvent::MouseMove)
            {
                if (m_dragging)
                {
                    slot_dragging(prev_point - QCursor::pos());
                    prev_point = QCursor::pos();
                }
            }
            if (mouseEvent->button() & Qt::MiddleButton)
            {
                if (ev->type() == QEvent::MouseButtonPress)
                {
                    m_dragging = true;
                    QApplication::setOverrideCursor(Qt::OpenHandCursor);
                    prev_point = QCursor::pos();
                }

                if (ev->type() == QEvent::MouseButtonRelease)
                {
                    m_dragging = false;
                    QApplication::restoreOverrideCursor();
                }
            }
        }
    }
    return false;
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
bool WorkScrolledArea::imageAvailable()
{
    return !mp_work_area->currentLayer()->image.isNull();
}


int WorkArea::markerSize() const
{
    return m_marker_size;
}

void WorkArea::enableCircleAround(int size)
{
    m_circle_around = size;
}
