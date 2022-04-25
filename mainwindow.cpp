/*
 *
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




#define SETTING_ValueFontSize "ALL/ValueFontSize"
#define SETTING_PointColor "ALL/PointColor"
#define SETTING_PointSize "ALL/PointSize"
#define SETTING_PointAround "ALL/PointAround"
#define SETTING_ReverseBottomX "ALL/ReverseBottomX"


#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QFileDialog>
#include <QDebug>
#include <QScrollArea>
#include <QStringBuilder>
#include "customdialogs.h"
#include "partitem.h"
#include "exceldata.h"
#include "dialogpart.h"
#include <QMessageBox>
#include <QSettings>
#include "version/v.h"
#include "importdlg.h"
#include <QStandardItemModel>




Qt::GlobalColor color_set[6] = {Qt::red,Qt::magenta,Qt::cyan,Qt::green,Qt::blue,Qt::yellow};
int size_set[5] = {15,20,36,40,50};


/** @brief Constuructor
 *
 *
 *  @param QWidget *parent
 *  @return
 */
MainWindow::MainWindow() :
    QMainWindow(0),
    ui(new Ui::MainWindow)
{
    setupUi();
    m_soldering =0;m_data_loaded=0;
    slot_apply_settings();
    updateControlsEnable();
}

MainWindow::~MainWindow()
{
    delete ui;
}

QString MainWindow::tablePartNameSelected()
{
    const int row = ui->tvParts->currentIndex().row();
    return tablePartNameByIndex(row);
}

QString MainWindow::tablePartNameByIndex(int index)
{
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    return pmodel->item(index,0)->text();
}

void MainWindow::setTableCurrentLine(int line)
{
    // TODO
    //set currentline
}

int MainWindow::tableLines()
{
    return ui->tvParts->model()->rowCount();
}

int MainWindow::tableCurrentLine()
{
    const int row = ui->tvParts->currentIndex().row();
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    return row;
}

void MainWindow::setTableText(int row, const QString &text1, const QString &text2,const QString &text3)
{
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    QStandardItem *pitem1 = pmodel->item(row,0);
    QStandardItem *pitem2 = pmodel->item(row,1);
    QStandardItem *pitem3 = pmodel->item(row,2);
    if(pitem1 == NULL){
        pitem1 = new QStandardItem;
        pmodel->setItem(row,0,pitem1);
        pitem2 = new QStandardItem;
        pmodel->setItem(row,1,pitem2);
        pitem3 = new QStandardItem;
        pmodel->setItem(row,2,pitem3);
    }
    pitem1->setText(text1);
    pitem2->setText(text2);
    pitem3->setText(text3);
}

void MainWindow::setTableRowColor(int row, const QColor &color)
{
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    pmodel->item(row,0)->setForeground(QBrush(color));
    pmodel->item(row,1)->setForeground(QBrush(color));
    pmodel->item(row,2)->setForeground(QBrush(color));
}

void MainWindow::clearTableSelection()
{
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    ui->tvParts->clearSelection();
}

void MainWindow::tableSelect(QList<int> selList, MarkType type)
{
    foreach(const int &row,selList){
        if(type==RowSelect){
            ui->tvParts->selectRow(row);
        }
        else{
            for(int col=0;col<3;col++){

            }
        }
    }
}

void MainWindow::makeStatistics()
{
    collectPartsInfo();
    if(m_soldering){
        if(currentLayer() == "TOP" && m_totalCountTop>0){
            double progress = (100*m_doneCountTop/m_totalCountTop);
            ui->pbProgress->setValue(progress);
        }
        else if(currentLayer() == "BOTTOM" && m_totalCountBot>0){
            double progress = (100*m_doneCountBot/m_totalCountBot);
            ui->pbProgress->setValue(progress);
        }
    }

    //total

    ui->lbTopStat->setText(QString("Top:%1(%2)")
                         .arg(m_doneCountTop )
                         .arg(m_totalCountTop )
                         );

    ui->lbBotStat->setText(QString("Bottom:%1(%2)")
                         .arg(m_doneCountBot )
                         .arg(m_totalCountBot )
                         );

}

bool MainWindow::loadProjectData()
{
    clearLists();
    QHash<QString, PartItem> loaded_parts_map;
    QList<QStringList> data = ImportDlg::loadInputDataFile(m_setup.dataFileName,m_setup.dataModel.format);


    if( ImportDlg::convertDataTable(&loaded_parts_map, data.mid(m_setup.dataOffset), m_setup.dataModel) ){
        m_parts_map = loaded_parts_map;
    }else{

        return false;
    }

    if (m_parts_map.count()>0)
    {
        prepareBulkListBy(ITEM_VALUE);
    }

    // load image TOP
    QImage img(m_setup.topImageFileName);
    mp_area->setImage(WorkArea::TOP,img);
    mp_area->setEnabled(true);
    ui->pbTopImage->setEnabled(1);

    //load BOTTOM image and is ok (if not opk is not ERROR)
    if(!m_setup.bottomImageFileName.isEmpty()){
        QImage img(m_setup.bottomImageFileName);
        mp_area->setImage(WorkArea::BOTTOM,img);
        mp_area->setEnabled(true);
        ui->pbBottomImage->setEnabled(1);
    }

    m_table_need_rebuild = 1;
    if (m_parts_map.count()>0)
    {
        prepareBulkListBy(ITEM_VALUE);
    }
    uiUpdateList();
    uiUpdatePreview();
    mp_area->workArea()->calibrate();
    mp_area->workArea()->repaint();
    collectPartsInfo();
    return true;
}


bool MainWindow::setupValid()
{
    bool result =false;
    do{
        if(!m_setup.bottomImageFileName.isEmpty() ){
            if( !QFile::exists(m_setup.bottomImageFileName) ){
                QMessageBox::critical(0,"EasySolder","Bottom image file used but not exist");
                break;
            }
            if( m_setup.dataModel.topLayerVal == m_setup.dataModel.botLayerVal){
                QMessageBox::critical(0,"EasySolder","Bottom == Top");
                break;
            }
        }

        if(!QFile::exists(m_setup.topImageFileName)){
            QMessageBox::critical(0,"EasySolder","Top image file not exist");
            break;
        }

        if(!QFile::exists(m_setup.dataFileName)){
            QMessageBox::critical(0,"EasySolder","Data file name not exist");
            break;
        }

        if(m_setup.dataOffset<0)  {
            QMessageBox::critical(0,"EasySolder","Data offset should be >=0");
            break;
        }

        if(m_setup.dataModel.partColon<0)    {
            QMessageBox::critical(0,"EasySolder","Part colon");
            break;}
        if(m_setup.dataModel.posXColon<0) {
            QMessageBox::critical(0,"EasySolder","Pos X not valid");
            break;
        }
        if(m_setup.dataModel.posYColon<0) {
         QMessageBox::critical(0,"EasySolder","Pos Y not valid");
            break;
        }
        if(m_setup.dataModel.layerColon<0)  {
            QMessageBox::critical(0,"EasySolder","Layer colon not valid");
            break;
        }
        if(m_setup.dataModel.valueColon<0)  {
            QMessageBox::critical(0,"EasySolder","Value colon not valid");
            break;
        }

        result =1;
    }while(0);

    return result;
}

void MainWindow::updateWindowTitle()
{
    if(!m_currentProjectFileName.isEmpty()){
     setWindowTitle(QString("EasySolder v%1 from %2-%3").arg(VERSION,0,16).arg(QString( VERSION_DATE)).arg(m_currentProjectFileName));
    }else{
        setWindowTitle(QString("EasySolder v%1 from %2").arg(VERSION,0,16).arg(QString( VERSION_DATE)));
    }
}

void MainWindow::updateControlsEnable()
{
    if(m_data_loaded)
    {
        ui->pbPrev->setEnabled(1);
        ui->pbNext->setEnabled(1);
        ui->tvParts->setEnabled(1);
        ui->pbReset->setEnabled(1);

        if(m_soldering)
        {
            ui->pbPrev->setEnabled(1);
            ui->pbNext->setText("Done");
            ui->tvParts->setSelectionMode(QAbstractItemView::MultiSelection);
            ui->pbReset->setEnabled(1);
            ui->pbLoadPrj->setEnabled(0);
            ui->pbSavePrj->setEnabled(0);
            ui->pbImportData->setEnabled(0);
            ui->pbTopImage->setEnabled(1);
            ui->pbBottomImage->setEnabled(1);
        }
        else {
            ui->pbPrev->setEnabled(0);
            ui->pbNext->setText("Start");
            ui->tvParts->setSelectionMode(QAbstractItemView::SingleSelection);
            ui->pbReset->setEnabled(0);
            ui->pbLoadPrj->setEnabled(1);
            ui->pbSavePrj->setEnabled(1);
            ui->pbImportData->setEnabled(1);
            ui->pbTopImage->setEnabled(0);
            ui->pbBottomImage->setEnabled(0);
        }
        if(!m_setup.dataFileName.isEmpty() && !m_setup.topImageFileName.isEmpty()){
            ui->pbSavePrj->setEnabled(1);
        }else {
            ui->pbSavePrj->setEnabled(0);
        }
    }else{
        ui->pbPrev->setEnabled(0);
        ui->pbNext->setEnabled(0);
        ui->tvParts->setEnabled(0);
        ui->pbReset->setEnabled(0);
    }

    if(m_setup.bottomImageFileName.isEmpty()){
        ui->cmLayer->setCurrentIndex(0);
        ui->cmLayer->setEnabled(0);
        ui->pbBottomImage->setEnabled(0);
    }else{
        ui->cmLayer->setEnabled(1);
        ui->pbBottomImage->setEnabled(1);
    }
}

void MainWindow::clearDoneStatus()
{
    foreach(const QString &itemName, m_parts_map.keys())
    {
        m_parts_map[itemName].done= false;
    }
}

void MainWindow::switchToTop()
{
    mp_area->switchImage(WorkArea::TOP);
    ui->pbBottomImage->blockSignals(1);
    ui->pbTopImage->blockSignals(1);
    ui->pbTopImage->setChecked(1);
    ui->pbTopImage->blockSignals(0);
    ui->pbBottomImage->blockSignals(0);
}

void MainWindow::switchToBottom()
{
    mp_area->switchImage(WorkArea::BOTTOM);
    ui->pbTopImage->blockSignals(1);
    ui->pbBottomImage->blockSignals(1);
    ui->pbBottomImage->setChecked(1);
    ui->pbBottomImage->blockSignals(0);
    ui->pbTopImage->blockSignals(0);
}

void MainWindow::setupUi()
{        

    mp_menu = new QMenu();
    ui->setupUi(this);
    mp_lbInfo = new QLabel();
    statusBar()->addWidget(mp_lbInfo);
    mp_area = new WorkScrolledArea(this);
    mp_area->setEnabled(0);
    ui->widget_2->setLayout(new QVBoxLayout);
    ui->widget_2->layout()->addWidget(mp_area);
    ui->pbTopImage->setEnabled(0);
    ui->pbBottomImage->setEnabled(0);
    ui->tvParts->setContextMenuPolicy(Qt::CustomContextMenu);

    ui->tvParts->setSelectionBehavior(QAbstractItemView::SelectRows);
    ui->tvParts->verticalHeader()->hide();
    ui->tvParts->setAutoFillBackground(true);
    ui->tvParts->setFocusPolicy(Qt::NoFocus);
    ui->tvParts->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);

    setStyleSheet("QTableView:item:selected {background-color: #3c414d; color: #FFFFFF}\n"
                  "QTableView:item:selected:focus {background-color: #1b5bf5;}")    ;
    m_table_need_rebuild = 1;
    m_both_layers =false;
    m_sort_by = NAME;
    QAction *pactionUseAsTP1 = new QAction("use as RefPoint1",0);
    QAction *pactionUseAsTP2 = new QAction("use as RefPoint2",0);
    mp_menu->addAction(pactionUseAsTP1);
    mp_menu->addAction(pactionUseAsTP2);

    ui->tvParts->setModel(new QStandardItemModel);
    connect(ui->tvParts,SIGNAL(customContextMenuRequested(QPoint))
            ,this,SLOT(slot_partContextMenu(QPoint)));
    connect(pactionUseAsTP1,SIGNAL(triggered())
            ,this,SLOT(slot_actionUseAsTP1()));
    connect(pactionUseAsTP2,SIGNAL(triggered())
            ,this,SLOT(slot_actionUseAsTP2()));
    connect(mp_area->workArea(), SIGNAL(signal_repainted())
            , this, SLOT(slot_repaint_preview()));
    connect(mp_area->workArea(), SIGNAL(signal_info(QString))
            , this, SLOT(slot_info(QString)));
    connect(ui->cmMarkerColor,SIGNAL(currentIndexChanged(int))
            ,this,SLOT(slot_apply_settings()));
    connect(ui->leMarkerSize,SIGNAL(textChanged(QString))
            ,this,SLOT(slot_apply_settings()));
    connect(ui->leMarkerAround,SIGNAL(textChanged(QString))
            ,this,SLOT(slot_apply_settings()));
    connect(ui->cmValueTextSize,SIGNAL(currentIndexChanged(int))
            ,this,SLOT(slot_apply_settings()));
    connect(ui->tvParts, SIGNAL(doubleClicked(QModelIndex))
            ,this,SLOT(slot_tvParts_itemDoubleClicked(QModelIndex)));
    connect(ui->tvParts, SIGNAL(clicked(QModelIndex))
            ,this,SLOT(slot_currentItemChanged(QModelIndex)));

    connect(ui->pbSortByName,SIGNAL(clicked()),SLOT(slot_resort()));
    connect(ui->pbSortByValue,SIGNAL(clicked()),SLOT(slot_resort()));
    connect(ui->pbTopImage,SIGNAL(clicked()),SLOT(slot_changeVisibleLayer()));
    connect(ui->pbBottomImage,SIGNAL(clicked()),SLOT(slot_changeVisibleLayer()));

    loadSettings();
    updateWindowTitle();




}

Qt::GlobalColor MainWindow::markerColorSetting()
{
    return color_set[ui->cmMarkerColor->currentIndex()];
}



/** @brief Constuructor
 *
 *
 *  @param QWidget *parent
 *  @return
 */
void MainWindow::resizeEvent(QResizeEvent *)
{
    uiUpdatePreview();
}

void MainWindow::keyPressEvent(QKeyEvent *pev)
{
    if(pev->key() == Qt::Key_Return)
    {
        this->on_pbNext_clicked();
    }
    else if(pev->key() == Qt::Key_Backspace)
    {
        this->on_pbPrev_clicked();
    }
}

void MainWindow::closeEvent(QCloseEvent *pev)
{
    QMessageBox msg;
    msg.setStandardButtons(QMessageBox::Cancel|QMessageBox::Close);
    msg.setText("Close application?");
    saveSettings();
    if (msg.exec() ==QMessageBox::Close) {

        pev->accept();
    } else {
        pev->ignore();
    }
}



/** @brief opens Custom csv file
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_openCsvFile()
{
    // TODO open csv
}

/** @brief opens Altitum csv file
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_openAltiumFile()
{

}

void MainWindow::slot_currentItemChanged(QModelIndex  index)
{
    const QString itemName = tablePartNameSelected();
    if(m_parts_map.contains(itemName)){
        const PartItem &item = m_parts_map[itemName];
        mp_area->workArea()->clearPoints();
        mp_area->workArea()->addPoint(item.position);
        uiUpdateItemInfo(itemName);
    }
}


/** @brief clears list of items(elements)
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::clearLists()
{    
    m_parts_map.clear();
    m_bulk.clear();
}

/** @brief updates preview
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::uiUpdatePreview()
{
    if (!mp_area->workArea()->layer(WorkArea::TOP)->image.isNull())
    {
        bool active = mp_area->workArea()->layer(WorkArea::TOP) == mp_area->workArea()->currentLayer();
        QRect area_rect = mp_area->workArea()->rect();
        qreal k = (qreal)ui->lbImgTop->geometry().height() / (qreal)area_rect.height();
        QRect visible_rect = mp_area->workArea()->visibleRegion().boundingRect();
        const QImage &topImg = mp_area->workArea()->layer(WorkArea::TOP)->image;
        QPixmap pix = QPixmap::fromImage(topImg.scaled(area_rect.size() * k));
        if(active){
            QPainter p(&pix);
            QPen pen(Qt::red);
            pen.setWidth(2);
            p.setPen(pen);
            p.drawRect(visible_rect.x()*k, visible_rect.y()*k, visible_rect.width()*k, visible_rect.height()*k);
            p.end();
        }
        ui->lbImgTop->setPixmap(pix);
    }

    if (!mp_area->workArea()->layer(WorkArea::BOTTOM)->image.isNull())
    {
        bool active = mp_area->workArea()->layer(WorkArea::BOTTOM) == mp_area->workArea()->currentLayer();
        QRect area_rect = mp_area->workArea()->rect();
        qreal k = (qreal)ui->lbImgBot->geometry().height() / (qreal)area_rect.height();
        QRect visible_rect = mp_area->workArea()->visibleRegion().boundingRect();
        const QImage &botImg = mp_area->workArea()->layer(WorkArea::BOTTOM)->image;
        QPixmap pix = QPixmap::fromImage( botImg.scaled(area_rect.size() * k) );
        if(active){
            QPainter p(&pix);
            QPen pen(Qt::red);
            pen.setWidth(2);
            p.setPen(pen);
            p.drawRect(visible_rect.x()*k, visible_rect.y()*k, visible_rect.width()*k, visible_rect.height()*k);
            p.end();
        }
        ui->lbImgBot->setPixmap(pix);
    }
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_apply_settings()
{
    int valueFontSize = size_set[ui->cmValueTextSize->currentIndex()];
    Qt::GlobalColor pointColor = color_set[ui->cmMarkerColor->currentIndex()];
    int pointSize = ui->leMarkerSize->text().toInt();
    QFont font("Arial Black",valueFontSize);
    font.setBold(1);
    ui->lbValue->setFont(font);
    ui->lbOrderNo->setFont(font);
    ui->lbOrderNo->setStyleSheet("color:grey");
    mp_area->workArea()->setMarkerColor(pointColor);
    mp_area->workArea()->setMarkerSize(pointSize);
    int circleAround = ui->leMarkerAround->text().toInt();
    mp_area->workArea()->enableCircleAround(circleAround);
    mp_area->refresh();
}


/** @brief load settings from ini file
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::loadSettings()
{
    QSettings settings(qApp->applicationName()+".ini",QSettings::IniFormat);
    ui->cmValueTextSize->setCurrentIndex(settings.value(SETTING_ValueFontSize,1).toInt());
    ui->cmMarkerColor->setCurrentIndex(settings.value(SETTING_PointColor,1).toInt());
    ui->leMarkerSize->setText(QString::number(settings.value(SETTING_PointSize,5).toInt()));
    ui->leMarkerAround->setText(QString::number(settings.value(SETTING_PointAround,0).toInt()));
}

/** @brief save settings
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::saveSettings()
{
    QSettings settings(qApp->applicationName()+".ini",QSettings::IniFormat);;
    settings.setValue(SETTING_ValueFontSize, ui->cmValueTextSize->currentIndex());
    settings.setValue(SETTING_PointColor, ui->cmMarkerColor->currentIndex());
    settings.setValue(SETTING_PointSize, ui->leMarkerSize->text().toInt());
    settings.setValue(SETTING_PointAround, ui->leMarkerAround->text().toInt());
    settings.setValue("LastDirectory", m_last_directory);
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::setLastDirectory(const QString &dir)
{
    m_last_directory = dir;
    saveSettings();
}

MainWindow *MainWindow::instance()
{
    static MainWindow *pinstance=0;
    if(pinstance==0){
        pinstance =  new MainWindow();
    }
    return pinstance;
}

QString MainWindow::lastDirectory()
{
    return m_last_directory;
}

QStringList MainWindow::getAsSortedListBy(ItemSortBy type)
{
    QStringList tmp;
    QStringList result;
    foreach (const QString &itemName, m_parts_map.keys())
    {
        const PartItem &item = m_parts_map[itemName];
        if (type == NAME)
        {
            tmp.append(itemName);
        }
        else if (type == VALUE)
        {
            tmp.append(item.value % "#" % itemName);

        }
        else if (type == ORDER_NO)
        {
            tmp.append(item.order_no % "#" % itemName);
        }
        else if (type == DESCRIPTION)
        {
            tmp.append(item.description % "#" % itemName);
        }
    }
    qSort(tmp.begin(),tmp.end());
    QStringList::const_iterator li = tmp.constBegin();
    while (li != tmp.constEnd())
    {
        if (type == NAME)
        {
            result.append((*li));
        }
        else {
            result.append((*li).split('#')[1]);
        }
        ++li;
    }
    return result;
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::populateTypes()
{
    foreach (const QString &itemName, m_parts_map.keys())
    {
        if(itemName.size()==0) continue;

        char c = itemName.toLatin1().constData()[0];
        PartItem item = m_parts_map[itemName];
        if (c == 'C')
        {
            item.type = PartItem::CAPACITOR;
        } else if (c == 'R')
        {
            item.type = PartItem::RESISTOR;
        }
        else if (c == 'Q')
        {
            item.type = PartItem::TRANSISTOR;
        }
        else if (c == 'D')
        {
            item.type = PartItem::DIODE;
        }
        else
        {
            item.type = PartItem::OTHER;
        }
    }
}

/**
 *  @brief
 *  @param bool rebuild
 *  @return void
 */
void MainWindow::uiUpdateList()
{    
    if(m_soldering){
        ui->tvParts->setSelectionMode(QAbstractItemView::MultiSelection);
    }else{
        ui->tvParts->setSelectionMode(QAbstractItemView::SingleSelection);
    }

    QStringList list = getAsSortedListBy(m_sort_by);
    QStringList currentValueParts;
    int firstItem =  -1;
    int row=0;

    if(m_table_need_rebuild){
        rebuildTable();
        m_table_need_rebuild =0;
    }

    clearTableSelection();

    foreach (const QString &itemName,list)
    {
        const PartItem &item = m_parts_map[itemName];
        if(item.value == m_current_value){
            currentValueParts.append(item.name);
        }
    }
    foreach(const QString &itemName,currentValueParts){
        list.removeOne(itemName);
        list.insert(0,itemName);
    }

    foreach (const QString &itemName,list)
    {
        const PartItem &item = m_parts_map[itemName];
        if(currentLayer() == item.layer )
        {
            QString item_value;
            if(ui->pbSortByValue->text() == "by ORDER NO")
            {
                setTableText(row,itemName,item.order_no,item.layer);
                item_value= item.order_no;
            }
            else
            {
                setTableText(row,itemName,item.value,item.layer);
                item_value = item.value;
            }
            if(item_value == m_current_value && !item.done)
            {
                if(firstItem==-1) {
                    setTableCurrentLine(firstItem);
                    firstItem = row;
                }
                ui->tvParts->selectRow(row);
            }

            if(ui->cbHideDone->isChecked() )
            {
                if(!item.done){
                    setTableRowColor(row,Qt::black);
                    row++;
                }
            }
            else
            {
                if(item.done)
                {
                    setTableRowColor(row,Qt::gray);
                }else{
                    setTableRowColor(row,Qt::black);
                }
                row++;
            }
        }

    }

    makeStatistics();
}

/**
 *  @brief Prepares bulk list. This list is then used for soldering process.
 *  @param ItemSortBy bulk_by . Available only ORDER_NO and VALUE.
 *  @return void
 */
void MainWindow::prepareBulkListBy(BulkBy bulk_by)
{    
    m_bulk.clear();
    for (qint32 item_type = 0; item_type < PartItem::COUNT; item_type++)
    {
        /* find types */
        QStringList list;
        foreach(const QString &itemName , m_parts_map.keys())
        {
            if (m_parts_map[itemName].type == item_type)
            {
                list.append(itemName);
            }
        }
        /* sort by values */

        switch(bulk_by)
        {

        case ITEM_ORDER_NO:
            for (qint32 i = 0; i < list.size(); i++)
            {
                QString name = list.at(i);
                QString value = m_parts_map[name].order_no;
                if (!m_bulk.contains( value))
                {
                    m_bulk.insert(value, QStringList());
                }
                m_bulk[value].append(name);
            }
            break;
        default:
        case ITEM_VALUE:
            for (qint32 i = 0; i < list.size(); i++)
            {
                QString name = list.at(i);
                QString value = QString::number(m_parts_map[name].type) % m_parts_map[name].value;
                if (!m_bulk.contains( value))
                {
                    m_bulk.insert(value, QStringList());
                }
                m_bulk[value].append(name);
            }
            break;
        }

    }
    /* set to begin */
    m_bulk_mode = bulk_by;
    m_bulk_current_value = "";

}



/** @brief slot on button Next clicked
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_pbNext_clicked()
{        
    m_soldering =1;
    if(ui->cbHideDone->isChecked()){
        m_table_need_rebuild =1;
    }

    int unsolderedCount;
    do{
        collectPartsInfo();
        if(currentLayer() == "TOP"){
            unsolderedCount = m_totalCountTop -m_doneCountTop;

        }else{
            unsolderedCount = m_totalCountBot -m_doneCountBot;
        }
        nextPartsBulk(FORWARD);
        if(addPartsBulk())
        {
            mp_area->refresh();
            uiUpdateList();
            break;
        }
    }while(unsolderedCount>0);


    // nothing to solder after
    if(unsolderedCount ==0)
    {
        uiUpdateList();
        QMessageBox::information(0,"finished!"
                                 ,QString("Layer %1 finsihed").arg(currentLayer()));
        m_soldering = false;
    }


    makeStatistics();
    updateControlsEnable();
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_pbPrev_clicked()
{
    if(ui->cbHideDone->isChecked()){
        m_table_need_rebuild =1;
    }
    nextPartsBulk(BACKWARD);
    if(addPartsBulk())
    {
        mp_area->refresh();
        uiUpdateList();
    }
    if(m_soldering){
        ui->tvParts->setSelectionMode(QAbstractItemView::MultiSelection);
    }
    else{
        ui->tvParts->setSelectionMode(QAbstractItemView::SingleSelection);
    }
}


/** @brief add points to workarea
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::nextPartsBulk(Direction dir)
{
    if(m_bulk.size())
    {
        /*[1] increment current item*/
        if(!m_bulk_current_value.isEmpty() )
        {
            int index_of_current = m_bulk.keys().indexOf(m_bulk_current_value);
            int count = m_bulk.keys().count();
            //forward
            if(dir == FORWARD)
            {
                index_of_current = (index_of_current+count+1)%count;
                QStringList list  =  m_bulk[m_bulk_current_value];
                // mark as done
                for(QStringList::const_iterator i= list.constBegin();i !=  list.constEnd(); )
                {
                    PartItem &item = m_parts_map[*i];
                    if(currentLayer() == item.layer)
                    {
                        item.done = true;
                    }
                    ++i;
                }
                m_bulk_current_value = m_bulk.keys().at(index_of_current);
            }
            //backward
            else {
                index_of_current = (index_of_current+count-1)%count;
                m_bulk_current_value = m_bulk.keys().at(index_of_current);
                QStringList list  =  m_bulk[m_bulk.keys().at(index_of_current)];
                // mark as done
                for(QStringList::const_iterator i= list.constBegin();i !=  list.constEnd(); )
                {
                    PartItem &item = m_parts_map[*i];
                    if(currentLayer() == item.layer)
                    {
                        item.done = false;
                    }
                    ++i;
                }
            }
        }
        else m_bulk_current_value = m_bulk.keys().at(0);
    }
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
bool MainWindow::addPartsBulk()
{
    bool result = false;
    //adding points
    mp_area->workArea()->clearPoints();
    if(m_bulk.contains(m_bulk_current_value))
    {
        if(!m_bulk_current_value.isEmpty())
        {
            QStringList list  =  m_bulk[m_bulk_current_value];
            QString order_no;
            QString text ;
            PartItem *pitem;
            int parts_count=0;
            for(QStringList::const_iterator i= list.constBegin();i !=  list.constEnd(); )
            {
                pitem = &m_parts_map[*i];
                if(order_no.isEmpty()) order_no = pitem->order_no;
                if(currentLayer().contains(pitem->layer) && !pitem->done)
                {
                    mp_area->workArea()->addPoint(pitem->position);
                    text += *i;
                    if (i != list.constEnd()) {text += ", ";}
                    parts_count++;
                }
                ++i;
            }
            // if some points in list
            if(parts_count)
            {
                switch(m_bulk_mode)
                {
                case ITEM_ORDER_NO:
                    m_current_value = pitem->order_no;
                    break;
                case ITEM_VALUE:
                    m_current_value  = pitem->value;
                    break;
                default:
                    break;
                }
                ui->lbValue->setText(pitem->value);
                ui->lbOrderNo->setText(pitem->order_no);
                ui->lbOrderNo->setText(order_no);
                ui->lbCount->setText(QString("%1").arg(parts_count));
                ui->lbPartsList->setText(text);
                result = true;
            }
        }
    }
    mp_area->focusAllPoints();/* update ui */
    return result;
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void  MainWindow::collectPartsInfo()
{
    m_totalCountTop =0;
    m_doneCountTop =0;
    foreach(const QString &itemName, m_parts_map.keys())
    {
        const PartItem &item = m_parts_map[itemName];
        if( "TOP" == item.layer)
        {
            if(item.done) m_doneCountTop++;
            m_totalCountTop++;

        }
    }
    m_totalCountBot =0;
    m_doneCountBot =0;
    foreach(const QString &itemName, m_parts_map.keys())
    {
           const PartItem &item = m_parts_map[itemName];
        if( "BOTTOM" == item.layer)
        {
            if(item.done) m_doneCountBot++;
            m_totalCountBot++;
        }
    }

}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_repaint_preview()
{
    uiUpdatePreview();
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_info(const QString &text)
{
    mp_lbInfo->setText(text);
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_tvParts_itemDoubleClicked(QModelIndex index)
{
    Q_ASSERT(pwidgetitem);
    const QString itemName = tablePartNameByIndex(index.row());
    if ( m_parts_map.contains(itemName))
    {
        const PartItem &item = m_parts_map[itemName];
        DialogPart dialog;
        dialog.setGeometry(QCursor::pos().x()-dialog.rect().width()
                           , QCursor::pos().y()
                           , dialog.rect().width()
                           , dialog.rect().height());
        dialog.setData(itemName, item);
        dialog.exec();
    }
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_changeVisibleLayer()
{
    if(ui->pbTopImage->isChecked())
        mp_area->switchImage(WorkArea::TOP);
    else
        mp_area->switchImage(WorkArea::BOTTOM);

    if( (ui->pbTopImage->isChecked() && ui->cmLayer->currentIndex()!=0)
            || (ui->pbBottomImage->isChecked() && ui->cmLayer->currentIndex()!=1) )
    {
        QMessageBox::warning(0,"","Image layer is different with parts layer");
    }
}

void MainWindow::slot_partContextMenu(const QPoint &)
{
    mp_menu->exec(QCursor::pos());
}

void MainWindow::slot_openProject()
{

    QString filename = QFileDialog::getOpenFileName(0, "Load file EasySolder project",
                                                    lastDirectory(),
                                                    "Files (*.prj)");
}




/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_cmLayer_currentIndexChanged(int index)
{
    if(index==0)
    {
        switchToTop();
    }
    else {
        switchToBottom();
    }
    m_table_need_rebuild = 1;
    uiUpdateList();
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_resort()
{    

    if(ui->pbSortByName->isChecked())
    {
        m_sort_by = NAME;
        prepareBulkListBy(ITEM_VALUE);
    }
    else if(ui->pbSortByValue->isChecked())
    {
        switch(m_bulk_mode)
        {
        case ITEM_ORDER_NO:
            ui->pbSortByValue->setText("by VALUE");
            m_sort_by = VALUE;
            prepareBulkListBy(ITEM_VALUE);
            break;
        case ITEM_VALUE:
            ui->pbSortByValue->setText("by ORDER NO");
            m_sort_by = ORDER_NO;
            prepareBulkListBy(ITEM_ORDER_NO);
            break;
        default:
            break;
        }
    }
    uiUpdateList();
}

/** @brief
 *
 *
 *  @param
 *  @return const QString
 */
const QString MainWindow::currentLayer()
{
    if(m_both_layers) return QString("TOP & BOTTOM");
    return ui->cmLayer->currentText();
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::uiUpdateItemInfo(const QString &item_name)
{
    QString value;
    QString order_no;
    if(m_parts_map.contains(item_name))
    {
        value = m_parts_map[item_name].value;
        order_no = m_parts_map[item_name].order_no;
    }
    ui->lbValue->setText(value);
    ui->lbOrderNo->setText(order_no);
}

void MainWindow::rebuildTable()
{
    QStandardItemModel *pmodel = static_cast<QStandardItemModel*>(ui->tvParts->model());
    pmodel->clear();
    int rowsCount=0;

    if(ui->cbHideDone->isChecked()){
        foreach(const PartItem &part, m_parts_map.values()){
            if(!part.done && part.layer == currentLayer()) rowsCount++;
        }
    }else{
        foreach(const PartItem &part, m_parts_map.values()){
            if(part.layer == currentLayer()) rowsCount++;
        }
    }

    pmodel->setRowCount(rowsCount);
    pmodel->setColumnCount(3);
    pmodel->setHeaderData(0,Qt::Horizontal,"Part");
    pmodel->setHeaderData(1,Qt::Horizontal,"Value");
    pmodel->setHeaderData(2,Qt::Horizontal,"Layer");
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_cbBothLayers_toggled(bool checked)
{
    m_both_layers = checked;
    uiUpdateList();
}

/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_actionUseAsTP1()
{
    if(isDataLoaded() && mp_area->imageAvailable())
    {
        const QString  partName = tablePartNameSelected();
        const PartItem &item = m_parts_map[partName];
        mp_area->workArea()->currentLayer()->tp1_original.setX(item.position.x());
        mp_area->workArea()->currentLayer()->tp1_original.setY(item.position.y());
        mp_area->workArea()->slot_locateTP1_click();

    }
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::slot_actionUseAsTP2()
{
    if(isDataLoaded()&& mp_area->imageAvailable())
    {
        const QString  partName = tablePartNameSelected();
        const PartItem &item = m_parts_map[partName];
        mp_area->workArea()->currentLayer()->tp2_original.setX(item.position.x());
        mp_area->workArea()->currentLayer()->tp2_original.setY(item.position.y());
        mp_area->workArea()->slot_locateTP2_click();
    }
}
/** @brief
 *
 *
 *  @param
 *  @return bool
 */
bool MainWindow::isDataLoaded() const
{
    return !m_parts_map.isEmpty();
}


/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_pbFind_clicked()
{    
    int count = ui->tvParts->model()->rowCount();
    bool find_wildcard = ui->leFind->text().contains("*");
    for(int i=0;i<count;i++)
    {
        int index = (i + m_last_found_index)%count;
        bool found = false;
        if(find_wildcard){
            found = QRegExp(ui->leFind->text(),Qt::CaseInsensitive, QRegExp::Wildcard)
                    .exactMatch( tablePartNameByIndex(index) );
        }
        else {
            found = (ui->leFind->text() == tablePartNameByIndex(index));
        }

        if(found)
        {
            setTableCurrentLine(index);
            m_last_found_index = index+1;
            break;
        }
    }
}
/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_pbReset_clicked()
{    
    clearDoneStatus();
    setTableCurrentLine(0);
    ui->lbCount->setText("");
    ui->lbImgBot->setText("");
    ui->lbImgTop->setText("");
    ui->lbOrderNo->setText("");
    ui->lbPartsList->setText("");
    ui->lbValue->setText("");
    m_current_value = "";
    m_bulk_current_value = "";
    makeStatistics();
    //m_bulk.clear();
    m_soldering = 0;
    uiUpdateList();
    updateControlsEnable();
}
/** @brief
 *
 *
 *  @param
 *  @return void
 */
void MainWindow::on_cbHideDone_clicked()
{
    m_table_need_rebuild = 1;
    uiUpdateList();
}

void MainWindow::on_leFind_textChanged(const QString &arg1)
{
    m_last_found_index=0;
    on_pbFind_clicked();
}

void MainWindow::on_pbImportData_clicked()
{    
    ImportDlg importDlg(&m_setup);
    if(importDlg.exec() == QDialog::Accepted)
    {
        bool ok = setupValid();        
        if(ok){
            m_data_loaded = 1;
            loadProjectData();
        }
    }

    makeStatistics();
    updateControlsEnable();
}


void MainWindow::saveCurrentProject()
{
    QString prjFileName;

    if(!projectSaved()){
        prjFileName = QFileDialog::getSaveFileName(0,"Save file",lastDirectory(),"*.esprj");
        m_currentProjectFileName = prjFileName;
    }
    else prjFileName = m_currentProjectFileName;

    QSettings settings(prjFileName, QSettings::IniFormat);
    settings.setValue("Files/DataFormat", m_setup.dataModel.format);
    settings.setValue("Files/Data", m_setup.dataFileName);
    settings.setValue("Files/TopImage", m_setup.topImageFileName);
    settings.setValue("Files/BottomImage", m_setup.bottomImageFileName);
    settings.setValue("Files/DataOffset", m_setup.dataOffset);

    settings.setValue("Model/PartColon", m_setup.dataModel.partColon);
    settings.setValue("Model/CodeColon", m_setup.dataModel.codeColon);
    settings.setValue("Model/ValueColon", m_setup.dataModel.valueColon);
    settings.setValue("Model/PosXColon", m_setup.dataModel.posXColon);
    settings.setValue("Model/PosYColon", m_setup.dataModel.posYColon);
    settings.setValue("Model/DescriptionColon", m_setup.dataModel.descrColon);
    settings.setValue("Model/LayerColon", m_setup.dataModel.layerColon);
    settings.setValue("Model/TopLayer", m_setup.dataModel.topLayerVal);
    settings.setValue("Model/BottomLayer", m_setup.dataModel.botLayerVal);


    WorkArea::LayerSettings *playerTOP = mp_area->workArea()->layer(WorkArea::TOP);
    if(!playerTOP->tp1.isNull()){
        settings.setValue("TopTP1_Enabled",true);
        settings.setValue("TopTP1_X",playerTOP->tp1.x());
        settings.setValue("TopTP1_Y", playerTOP->tp1.y());
        settings.setValue("TopTP1_X1",playerTOP->tp1_original.x());
        settings.setValue("TopTP1_Y1", playerTOP->tp1_original.y());
    }
    if(!playerTOP->tp2.isNull()){
        settings.setValue("TopTP2_Enabled", true);
        settings.setValue("TopTP2_X", playerTOP->tp2.x());
        settings.setValue("TopTP2_Y", playerTOP->tp2.y());
        settings.setValue("TopTP2_X1",playerTOP->tp2_original.x());
        settings.setValue("TopTP2_Y1", playerTOP->tp2_original.y());
    }

    WorkArea::LayerSettings *playerBOT = mp_area->workArea()->layer(WorkArea::BOTTOM);
    if(!playerBOT->tp1.isNull()){
        settings.setValue("BottomTP1_Enabled",  true);
        settings.setValue("BottomTP1_X",  playerBOT->tp1.x());
        settings.setValue("BottomTP1_Y",  playerBOT->tp1.y());
        settings.setValue("BottomTP1_X1",  playerBOT->tp1_original.x());
        settings.setValue("BottomTP1_Y1",  playerBOT->tp1_original.y());
    }
    if(!playerBOT->tp2.isNull()){
        settings.setValue("BottomTP2_Enabled", true);
        settings.setValue("BottomTP2_X", playerBOT->tp2.x());
        settings.setValue("BottomTP2_Y", playerBOT->tp2.y());
        settings.setValue("BottomTP2_X1", playerBOT->tp2_original.x());
        settings.setValue("BottomTP2_Y1", playerBOT->tp2_original.y());
    }
}

void MainWindow::on_pbSavePrj_clicked()
{
    if(dataCorrect()){
        saveCurrentProject();
    }
    collectPartsInfo();
}

bool MainWindow::dataCorrect()
{
    if(!m_setup.dataFileName.isEmpty()
            && !m_setup.topImageFileName.isEmpty()
            && !m_setup.bottomImageFileName.isEmpty()){
        return true;
    }
    return false;
}

void MainWindow::on_pbLoadPrj_clicked()
{
    QString fileName = QFileDialog::getOpenFileName(0,"EasySolder",QString(),"*.esprj");

    // dialog window Cancel pressed
    if(fileName.isNull()) return;

    if(QFile::exists(fileName)){
        m_currentProjectFileName =fileName;
    }

    QSettings projectFile(m_currentProjectFileName, QSettings::IniFormat);
    m_setup.dataModel.format = projectFile.value("Files/DataFormat").toString();
    m_setup.dataFileName = projectFile.value("Files/Data" ).toString();
    m_setup.topImageFileName = projectFile.value("Files/TopImage", m_setup.topImageFileName).toString();
    m_setup.bottomImageFileName = projectFile.value("Files/BottomImage").toString();
    m_setup.dataOffset = projectFile.value("Files/DataOffset").toInt();

    m_setup.dataModel.partColon = projectFile.value("Model/PartColon" ).toInt();
    m_setup.dataModel.codeColon = projectFile.value("Model/CodeColon" ).toInt();
    m_setup.dataModel.valueColon = projectFile.value("Model/ValueColon" ).toInt();
    m_setup.dataModel.posXColon = projectFile.value("Model/PosXColon").toInt();
    m_setup.dataModel.posYColon = projectFile.value("Model/PosYColon").toInt();
    m_setup.dataModel.descrColon = projectFile.value("Model/DescriptionColon").toInt();
    m_setup.dataModel.layerColon = projectFile.value("Model/LayerColon").toInt();
    m_setup.dataModel.topLayerVal = projectFile.value("Model/TopLayer").toString();
    m_setup.dataModel.botLayerVal = projectFile.value("Model/BottomLayer").toString();



    if(projectFile.value("TopTP1_Enabled").toBool()){
        mp_area->workArea()->layer(WorkArea::TOP)->tp1.setX(projectFile.value("TopTP1_X").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp1.setY(projectFile.value("TopTP1_Y").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp1_original.setX(projectFile.value("TopTP1_X1").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp1_original.setY(projectFile.value("TopTP1_Y1").toDouble());

    }
    if(projectFile.value("TopTP2_Enabled").toBool()){
        mp_area->workArea()->layer(WorkArea::TOP)->tp2.setX(projectFile.value("TopTP2_X").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp2.setY(projectFile.value("TopTP2_Y").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp2_original.setX(projectFile.value("TopTP2_X1").toDouble());
        mp_area->workArea()->layer(WorkArea::TOP)->tp2_original.setY(projectFile.value("TopTP2_Y1").toDouble());
    }
    if(projectFile.value("BottomTP1_Enabled").toBool()){
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp1.setX(projectFile.value("BottomTP1_X").toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp1.setY(projectFile.value("BottomTP1_Y").toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp1_original.setX(projectFile.value("BottomTP1_X1").toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp1_original.setY(projectFile.value("BottomTP1_Y1").toDouble());
    }
    if(projectFile.value("BottomTP2_Enabled" ).toBool()){
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp2.setX(projectFile.value("BottomTP2_X").toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp2.setY(projectFile.value("BottomTP2_Y" ).toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp2_original.setX(projectFile.value("BottomTP2_X1").toDouble());
        mp_area->workArea()->layer(WorkArea::BOTTOM)->tp2_original.setY(projectFile.value("BottomTP2_Y1" ).toDouble());
    }
    if(setupValid())
    {
        m_data_loaded = 1;
        loadProjectData();
    }

    makeStatistics();
    updateControlsEnable();
}

void MainWindow::on_cmLayerImage_currentIndexChanged(int index)
{

}
