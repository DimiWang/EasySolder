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
#include "customdialogs.h"
#include <QRadioButton>
#include <QPushButton>
#include <QDialogButtonBox>
#include <QLineEdit>
#include <Windows.h>
#include "stdio.h"
/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogCheckBoxes::DialogCheckBoxes(Type type, QWidget *parent) :
    QDialog(parent)
{

    m_type = type;
    setupUi();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogCheckBoxes::inflate(const QStringList &list)
{
    qint32 count = list.size();
    qint32 i = 0;
    while (count--)
    {
        QAbstractButton *pbutton;
        if (m_type == CHECKBOX)
        {
            pbutton = new QCheckBox;
        }
        else if (m_type == RADIOBUTTON)
        {
            pbutton = new QRadioButton;
        }
        QListWidgetItem *pwidgetitem = new QListWidgetItem;
        pbutton->setText(list[i++]);
        pbutton->setFixedSize(QSize(100, 15));
        mp_widgetitems->append(pwidgetitem);
        mp_checkboxes->append(pbutton);
        mp_list_widget->addItem(pwidgetitem);
        mp_list_widget->setItemWidget(pwidgetitem, pbutton);
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogCheckBoxes::addWidget(QWidget *pwidget)
{
    mp_layout->addWidget(pwidget);
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
qint32 DialogCheckBoxes::readStringList(QStringList &list)
{
    qint32 i = 0;
    list.clear();
    for (i = 0; i < mp_checkboxes->count(); i++)
    {
        if ((*mp_checkboxes)[i]->isChecked())
            list.append((*mp_checkboxes)[i]->text());
    }
    return list.count();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
qint32 DialogCheckBoxes::readIndexList(QList<qint32> &list)
{
    qint32 i = 0;
    for (i = 0; i < mp_checkboxes->count(); i++)
    {
        if ((*mp_checkboxes)[i]->isChecked())
            list.append(i);
    }
    return list.count();
}

void DialogCheckBoxes::setupUi()
{
    mp_list_widget =  new QListWidget(this);
    mp_layout = new QVBoxLayout(this);
    mp_buttonbox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mp_checkboxes = new QList<QAbstractButton *>();
    mp_widgetitems = new QList<QListWidgetItem *>();
    mp_layout->addWidget(mp_list_widget);
    mp_layout->addWidget(mp_buttonbox);
    setLayout(mp_layout);
    if(m_type != RADIOBUTTON)
    {
        QPushButton *ppbOn = mp_buttonbox->addButton("All On", QDialogButtonBox::ActionRole);
        QPushButton *ppbOff = mp_buttonbox->addButton("All Off", QDialogButtonBox::ActionRole);
        connect(ppbOn, SIGNAL(clicked()), this, SLOT(slot_SetAllOn()));
        connect(ppbOff, SIGNAL(clicked()), this, SLOT(slot_SetAllOff()));
    }
    connect(mp_buttonbox, SIGNAL(accepted()), this, SLOT(accept()));
    connect(mp_buttonbox, SIGNAL(rejected()), this, SLOT(close()));
}


/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogCheckBoxes::slot_SetAllOn()
{
    qint32 i = 0;
    for (i = 0; i < mp_checkboxes->count(); i++)
    {
        (*mp_checkboxes)[i]->setChecked(true);
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogCheckBoxes::slot_SetAllOff()
{
    qint32 i = 0;
    for (i = 0; i < mp_checkboxes->count(); i++)
    {
        (*mp_checkboxes)[i]->setChecked(false);
    }
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogCheckBoxes::~DialogCheckBoxes()
{
    for (qint32 i = 0; i < mp_checkboxes->count(); i++)
    {
        delete mp_checkboxes->at(i);
        delete mp_widgetitems->at(i);
    }
    mp_checkboxes->clear();
    mp_widgetitems->clear();
    delete mp_buttonbox;
    delete mp_layout;
    delete mp_list_widget;
    delete mp_widgetitems;
    delete mp_checkboxes;
}



/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogProgressBar::DialogProgressBar(QWidget *parent, const QPixmap &pixmap)
    :QDialog(parent)
{
    setupUi();
    setLayout(&m_layout);
    this->setModal(1);
    m_layout.addWidget(&m_progress_bar);
    m_layout.addWidget(&m_label);
    if(!pixmap.isNull() )
    {
        m_image.setPixmap(pixmap);
        m_image.setFixedSize(200,200);
    }
    m_layout.addWidget(&m_image);


}
/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogProgressBar::~DialogProgressBar()
{
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogProgressBar::addWidget(QWidget *pwidget)
{

    m_layout.addWidget(pwidget);
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogProgressBar::setProgress(qint32 value, qint32 maximum)
{
    m_progress_bar.setValue(value);
    m_progress_bar.setMaximum(maximum);
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
qint32 DialogProgressBar::progress()
{
    return m_progress_bar.value();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogProgressBar::setText(const QString &text)
{
    m_label.setText(text);
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogProgressBar::sleep(qint32 sleep)
{
    Sleep(sleep);
}

void DialogProgressBar::setupUi()
{
}


/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogPart::DialogPart(QWidget *parent) :
    QDialog(parent)
{    
    setupUi();
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogPart::setData(const QString &name, const PartItem &item)
{
    char tmp[100];    
        this->setWindowTitle(name);
        mp_lbDescription->setText(item.description);
        mp_lbName->setText("<b>NAME</b>:\t\t"+name);
        mp_lbValue->setText("<b>VALUE:</b>\t\t"+item.value);
        mp_lbLayer->setText("<b>LAYER:</b>\t\t"+item.layer);
        mp_lbOrderNo->setText("<b>ORDER NO:</b>\t\t"+item.order_no);
        ::sprintf(tmp,"<b>POSITION:</b> \t\tX:%2.2f Y:%2.2f",item.position.x(), item.position.y());
        mp_lbPosition->setText(QString(tmp));    
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
DialogPart::~DialogPart()
{
    delete  mp_pbClose;
    delete mp_lbName;
    delete mp_lbValue;
    delete mp_lbLayer;
    delete mp_lbPosition;
    delete mp_lbOrderNo;
    delete mp_lbDescription;
}
/** @brief
 *
 *
 *  @param
 *  @return
 */
void DialogPart::setupUi()
{
    resize(300, 230);
    mp_lbName = new QLabel(this);
    mp_lbName->setGeometry(QRect(10, 10, 280, 16));
    mp_lbDescription = new QLabel(this);
    mp_lbDescription->setGeometry(QRect(10, 35, 280, 40));
    mp_lbDescription->setFrameShape(QFrame::Box);
    mp_lbDescription->setWordWrap(true);
    mp_lbValue = new QLabel(this);
    mp_lbValue->setGeometry(QRect(10, 85, 280, 20));
    mp_lbLayer = new QLabel(this);
    mp_lbLayer->setGeometry(QRect(10, 110, 280, 20));
    mp_lbPosition = new QLabel(this);
    mp_lbPosition->setGeometry(QRect(10, 135, 280, 20));
    mp_lbOrderNo = new QLabel(this);
    mp_lbOrderNo->setGeometry(QRect(10, 160, 280, 20));
    mp_pbClose = new QPushButton("Close",this);
    mp_pbClose->setGeometry(QRect(100, 190, 80, 25));
    connect(this->mp_pbClose,SIGNAL(clicked()),this,SLOT(close()));
} // setupUi
