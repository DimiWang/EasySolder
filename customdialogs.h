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

#ifndef CUSTOMDIALOGS_H
#define CUSTOMDIALOGS_H

#include <QCheckBox>
#include <QDialog>
#include <QDialogButtonBox>
#include <QListWidgetItem>
#include <QVBoxLayout>
#include <QProgressBar>
#include <QLabel>
#include "PartItem.h"

class DialogCheckBoxes : public QDialog
{
    Q_OBJECT
public:
    typedef enum
    {
        CHECKBOX,
        RADIOBUTTON
    } Type;
    explicit DialogCheckBoxes(Type type = CHECKBOX, QWidget *parent = 0);
    ~DialogCheckBoxes();
    void inflate(const QStringList &list);
    void addWidget(QWidget *pwidget);
    qint32 readStringList(QStringList &list);
    qint32 readIndexList(QList<qint32> &list);

private:
    Type m_type;
    QListWidget *mp_list_widget;
    QVBoxLayout *mp_layout;
    QList<QListWidgetItem *> *mp_widgetitems;
    QList<QAbstractButton *> *mp_checkboxes;
    QDialogButtonBox *mp_buttonbox;
    void setupUi();
signals:

private slots:
    void slot_SetAllOn();
    void slot_SetAllOff();

};

class DialogProgressBar : public QDialog
{
    Q_OBJECT
public:
    explicit DialogProgressBar(QWidget *parent = 0, const QPixmap &pixmap = QPixmap());
    ~DialogProgressBar();
    void addWidget(QWidget *pwidget);
    void setProgress(qint32 value,qint32 maximum);
    qint32 progress();
    void setText(const QString &text);
    static void sleep(qint32 sleep);
private:

    QProgressBar m_progress_bar;
    QVBoxLayout m_layout;
    QLabel m_label;
    QLabel m_image;
    void setupUi();

};


class DialogPart : public QDialog
{
    Q_OBJECT

public:
    explicit DialogPart(QWidget *parent = 0);
    void setData(const QString &name, const PartItem &item);
    ~DialogPart();

private:
    QPushButton *mp_pbClose;
    QLabel *mp_lbName;
    QLabel *mp_lbValue;
    QLabel *mp_lbLayer;
    QLabel *mp_lbPosition;
    QLabel *mp_lbOrderNo;
    QLabel *mp_lbDescription;
    void setupUi();

};

#endif // CUSTOMDIALOGS_H
