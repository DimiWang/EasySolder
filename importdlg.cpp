#include "importdlg.h"
#include "ui_importdlg.h"
#include <QFileDialog>
#include <QStandardItemModel>
#include <QMessageBox>
#include <QDebug>
#include "mainwindow.h"

const QStringList columnsList
        = QStringList()<<"LayerColumn"<<"PartColumn"<<"PosXColumn"<<"PosYColumn"<<"ValueColumn"<<"DescriptionColumn"<<"CodeColumn";


ImportDlg::ImportDlg(Setup *setup) :
    QDialog(0),
    ui(new Ui::ImportDlg)
{
    ui->setupUi(this);
    mp_setup = setup;
    ui->tvDataIn->setModel( new QStandardItemModel() );    
    ui->cmFormat->setCurrentIndex(0);    
    ui->leFileName->setText(setup->dataFileName);
    ui->leFileBottomImage->setText(setup->bottomImageFileName);
    ui->leFileTopImage->setText(setup->topImageFileName);
    m_last_directory = MainWindow::instance()->lastDirectory();
    ui->tvDataIn->horizontalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvDataIn->verticalHeader()->setContextMenuPolicy(Qt::CustomContextMenu);
    ui->tvDataIn->setContextMenuPolicy(Qt::CustomContextMenu);
    connect(ui->tvDataIn,SIGNAL(customContextMenuRequested(QPoint)),this,SLOT(slot_contextMenu(QPoint)));

}

ImportDlg::~ImportDlg()
{
    delete ui;
}

int ImportDlg::splitCsv(QStringList *plist, const QString &text)
{
    bool in_quota =false;
    QString tmp;
    foreach(const QChar &c, text)
    {
        if(c == '\"') {in_quota=!in_quota; continue;}
        if(c == ',' && !in_quota)
        {
            plist->append(tmp);
            tmp.clear();
            continue;
        }
        tmp.append(c);
    }

    plist->append(tmp);
    return plist->count();
}

bool ImportDlg::checkValid()
{
    return MainWindow::instance()->setupValid();
}

int ImportDlg::exec()
{

        if(!mp_setup->dataFileName.isEmpty() && QFile::exists(mp_setup->dataFileName))
        {
            // load data
            QList<QStringList> import_list;
            m_imported_list = ImportDlg::loadInputDataFile(mp_setup->dataFileName, ui->cmFormat->currentText());
            showImportedFile();

        }

    return QDialog::exec();
}



void ImportDlg::on_pbOpen_clicked()
{
    QString filename = QFileDialog::getOpenFileName(0, "Load file Altium pick and place file",
                                                    m_last_directory,
                                                    "Files (*.csv)");
    if (!QFile::exists(filename) )
    {
        // no file selected
        return;
    }
    QFileInfo fi(filename);
    setLastDirectory(fi.absolutePath());
    ui->leFileName->setText(filename);
    if(ui->cmFormat->currentIndex()==0){
        m_imported_list.clear();
        m_imported_list = loadInputDataFile(filename,"CSV Altium");
        if(!m_imported_list.isEmpty()){
            showImportedFile();
        }
    }
}

QList<QStringList> ImportDlg::loadInputDataFile(const QString &fileName,const QString &format)
{    

    // ALTIUM CSV
    if(format == "CSV Altium"){
        return loadCSVAltium(fileName);
    }


    return QList<QStringList>();
}

void ImportDlg::updateControlsEnabled()
{

}

QString ImportDlg::columnName(int i)
{
 if(mp_setup->dataModel.codeColon==i)    return "Code";
 else if(mp_setup->dataModel.descrColon==i) return "Descr";
 else if(mp_setup->dataModel.layerColon==i) return "Layer";
 else if(mp_setup->dataModel.partColon==i) return "Part";
 else if(mp_setup->dataModel.posXColon ==i) return "Pos X";
 else if(mp_setup->dataModel.posYColon ==i) return "Pos Y";
 else if(mp_setup->dataModel.valueColon==i) return "Value";
 return QString();
}

void ImportDlg::setLastDirectory(const QString &lastDir)
{
    MainWindow::instance()->setLastDirectory(lastDir);
    m_last_directory = lastDir;
}


bool ImportDlg::convertDataTable(QHash<QString, PartItem > *partsMap
                                 , const QList<QStringList> &importData
                                 , const DataModel &dataModel
                                 )
{
    QString error;
    bool result=0;

    if(dataModel.format == "CSV Altium")
    {
        int line_n=0;

        if( partsMap==0) return false;

        foreach(const QStringList &line, importData)
        {
            PartItem item;
            do{

                // PART NAME

                if(dataModel.partColon<line.count() && dataModel.partColon>=0){
                    const QString name = line.at(dataModel.partColon).trimmed();

                    if(!partsMap->contains(name)){
                        item.name = name;
                    }else{
                        int r = QMessageBox::warning(0,"EasySolder.import"
                                                     ,QString("Duplicate Part name met %1").arg(name)
                                                     ,QMessageBox::Ignore|QMessageBox::Abort);
                        if(r== QMessageBox::Ignore) continue;
                        else return false;
                    }
                    item.name = name;
                }else{
                    error = QString("wrong value colon number=%1").arg(dataModel.partColon);
                    break;
                }

                double posX,posY;
                // POS X
                if(dataModel.posXColon<line.count() && dataModel.posXColon>=0){
                    bool ok=0;
                    QString txt = line.at(dataModel.posXColon).trimmed();
                    posX = txt.replace("mm","").toDouble(&ok);
                    if(!ok) {
                        error = QString("Line%1: posX wrong value=%2").arg(line_n).arg(txt);
                        break;
                    }
                    item.position.setX(posX);
                } else {
                    error = QString("wrong pos X colon number=%1").arg(dataModel.posXColon);
                    break;
                }

                // POS Y
                if(dataModel.posYColon<line.count() && dataModel.posYColon>=0){
                    bool ok=0;
                    QString txt = line.at(dataModel.posYColon).trimmed();
                    posY = txt.replace("mm","").toDouble(&ok);
                    if(!ok) {
                        error = QString("Line%1: posY wrong value='%2'").arg(line_n).arg(txt);
                        break;
                    }
                    item.position.setY(posY);
                } else {
                    error = QString("wrong pos Y colon number=%1").arg(dataModel.posYColon);
                    break;
                }

                //LAYER
                if(dataModel.layerColon<line.count() && dataModel.layerColon>=0){
                    QString txt = line.at(dataModel.layerColon);
                    if(txt == dataModel.topLayerVal)
                        item.layer = "TOP";
                    else if(txt == dataModel.botLayerVal){
                        item.layer = "BOTTOM";
                    }else{
                        error = QString("Line%1: wrong layer value='%2'").arg(line_n).arg(txt);                        
                        break;
                    }
                }else {
                    error = QString("wrong layer colon number=%1").arg(dataModel.layerColon);
                    break;
                }

                // VALUE
                if(dataModel.valueColon<line.count() && dataModel.valueColon>=0){
                    const QString value = line.at(dataModel.valueColon).trimmed();
                    if(value.isEmpty()) item.value = "na";
                    else item.value = value;
                }else {
                    error = QString("wrong value colon number=%1").arg(dataModel.valueColon);
                    break;
                }



                // DESCR
                if(dataModel.descrColon>=0) // enabled
                {
                    if(dataModel.descrColon<line.count()&& dataModel.descrColon>=0){
                        const QString descr = line.at(dataModel.descrColon).trimmed();
                        item.description = descr;
                    }else if(dataModel.descrColon!=-1){
                        error = QString("wrong description colon number=%1").arg(dataModel.descrColon);
                        break;
                    }
                }

                // CODE
                if(dataModel.codeColon >=0)//enabled
                {
                    if(dataModel.codeColon<line.count() && dataModel.codeColon>=0){
                        const QString code = line.at(dataModel.codeColon).trimmed();
                        item.order_no = code;
                    }else{
                        error = QString("wrong code colon number=%1").arg(dataModel.codeColon);
                        break;
                    }
                }

                result =1;
            }while(0);

            if(result == false){
                int r = QMessageBox::critical(0,"Error",error,QMessageBox::Ignore|QMessageBox::Abort);
                if(r==QMessageBox::Abort) return false;
                else continue;
            }else{
                    (*partsMap)[item.name] = item;
            }

            line_n++;
        }//foreach
    }// if format...


    return result;
}

void ImportDlg::showImportedFile()
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());

    if(m_imported_list.count()>0){
        mp_model->clear();
        int rowsCount = m_imported_list.count();
        if(mp_setup->dataOffset>=0) rowsCount -= mp_setup->dataOffset;

        const int columnsCount = m_imported_list.at(0).count();
        mp_model->setRowCount(rowsCount);
        mp_model->setColumnCount(columnsCount);

        for(int row=0;row<rowsCount;row++)
        {
            const QStringList &rowData = m_imported_list[row];
            for(int col=0;col<rowData.count();col++)
            {
                mp_model->setItem(row, col, new QStandardItem(rowData[col]));
            }
        }

        for(int i=0;i<columnsCount;i++)
        {
            const QString column = columnName(i);
            if( !column.isEmpty()){
                    mp_model->setHeaderData(i,Qt::Horizontal,column);
            }else
                mp_model->setHeaderData(i,Qt::Horizontal,QString("%1").arg(i+1));
        }

        int i2=0;
        for(int i=0;i<rowsCount;i++){
            if(i<mp_setup->dataOffset)
                mp_model->setHeaderData(i,Qt::Vertical,QString("..."));
            else {
                mp_model->setHeaderData(i,Qt::Vertical,QString("%1").arg(i2+1));
                i2++;
            }
        }
    }
}

void ImportDlg::accept()
{
    mp_setup->dataFileName =  ui->leFileName->text();
    mp_setup->topImageFileName = ui->leFileTopImage->text();
    mp_setup->bottomImageFileName  = ui->leFileBottomImage->text();

    if(!checkValid()){
            return;
        }

        MainWindow::instance()->setLastDirectory(m_last_directory);
        QDialog::accept();
}

QList<QStringList> ImportDlg::loadCSVAltium(const QString &fileName)
{
    QList<QStringList> list;
    const int NAME=0;//designator
    const int FOOTPRINT=1;
    const int MID_X = 2;
    const int MID_Y = 3;
    const int REF_X = 4;
    const int REF_Y = 5;
    const int PAD_X = 6;
    const int PAD_Y = 7;
    const int LAYER = 8;
    const int ROTATION = 9;
    const int VALUE = 10; //comment


    QFile file(fileName);
    if (file.open(QFile::ReadOnly))
    {
        QString line = file.readLine(1000);//dummy read header
        while(!file.atEnd())
        {
            QStringList l ;
            splitCsv( &l ,line);
            // read next line
            line =QString(file.readLine(1000));

            //ignore all lines not 11 chains
            if(l.size()>0)
            {
                list.append(l);
            }else{
                int r = QMessageBox::warning(0,"EasySolder.Import",QString("File has empty string"),QMessageBox::Abort|QMessageBox::Ignore);
                if(r == QMessageBox::Abort) {list.clear(); break;}
                else continue;
            }


        }
        file.close();

    }
    else{
        QMessageBox::critical(0,"EasySolder.Import",QString("Can't open file %1").arg(fileName),QMessageBox::Ok);
        list.clear();
    }
    return list;
}




void ImportDlg::on_pbOpenBottom_clicked()
{
    QString filename = QFileDialog::getOpenFileName(0, "Load image for BOTTOM",
                                                    "",
                                                    "Files (*.png)");

    if ( !QFile::exists(filename) )
    {
        return ;
    }
    ui->leFileBottomImage->setText(filename);
    QFileInfo fi(filename);
    setLastDirectory(fi.absolutePath());
}

void ImportDlg::on_pbOpenTop_clicked()
{
    QString filename = QFileDialog::getOpenFileName(0, "Load image for TOP",
                                                    m_last_directory,
                                                    "Files (*.png)");

    if ( !QFile::exists(filename) )
    {
        return ;
    }
    ui->leFileTopImage->setText(filename);
    QFileInfo fi(filename);
    setLastDirectory(fi.absolutePath());
}


void ImportDlg::slot_contextMenu(const QPoint &p)
{
   makeMenu(p);
}


void ImportDlg::slot_SetDataStart(bool on)
{
    if(on){
        int row = ui->tvDataIn->currentIndex().row();
        if(row>=0 && row < ui->tvDataIn->model()->rowCount()){
            mp_setup->dataOffset = row;
        }else{
             QMessageBox::critical(0,"","Select row or cell with data start",QMessageBox::Ok);
        }
    }else{
        mp_setup->dataOffset =-1;
    }
    showImportedFile();
}

void ImportDlg::slot_SetProjectPathCell(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int row = ui->tvDataIn->currentIndex().row();
        int col = ui->tvDataIn->currentIndex().column();
        if( row >=0 && row<mp_model->rowCount() && col>=0 && col<=mp_model->columnCount()){
            const QString &filepath = mp_model->item(row,col)->text();
            const QFileInfo fi(filepath);
            mp_setup->projectPath = fi.path();
            mp_setup->projectName = fi.baseName();
            bool dir_exists = fi.exists();
            QMessageBox::information(0,"",QString("Default path =%1(%3)\n"
                                          "Default file name=%2")
                                     .arg(mp_setup->projectPath)
                                     .arg(mp_setup->projectName).arg(dir_exists?"Accessible":"Not accessible")
                                     ,QMessageBox::Ok);
        if(dir_exists){
            setLastDirectory(mp_setup->projectPath);
        }
        }
       else  QMessageBox::critical(0,"","Select one cell with project name",QMessageBox::Ok);
    }
    else{
        mp_setup->projectPath.clear();
        mp_setup->projectName.clear();
    }
}

void ImportDlg::slot_SetPartNameColumn(bool on)
{

    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
            mp_setup->dataModel.partColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"Part");
            ui->cbPart->setChecked(1);
        }

    }else{
        mp_setup->dataModel.partColon = -1;
        ui->cbPart->setChecked(1);
    }

}

void ImportDlg::slot_SetCodeColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
            mp_setup->dataModel.codeColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"Code");
            ui->cbCode->setChecked(1);
        }
    }else{
        mp_setup->dataModel.codeColon =-1;
        ui->cbCode->setChecked(1);
    }
}

void ImportDlg::slot_SetValueColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
            mp_setup->dataModel.valueColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"Value");
            ui->cbValue->setChecked(1);
        }
    }else{
        mp_setup->dataModel.valueColon =-1;
        ui->cbValue->setChecked(1);
    }
}

void ImportDlg::slot_SetPosXColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
           mp_setup->dataModel.posXColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"PosX");
            ui->cbPosX->setChecked(1);
        }
    }else{
        mp_setup->dataModel.posXColon =-1;
        ui->cbPosX->setChecked(1);
    }
}

void ImportDlg::slot_SetPosYColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
            mp_setup->dataModel.posYColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"PosY");
            ui->cbPosY->setChecked(1);
        }
    }else{
        mp_setup->dataModel.posYColon =-1;
        ui->cbPosY->setChecked(1);
    }
}

void ImportDlg::slot_SetLayerColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        QDialog dlg;
        QGridLayout *play = new QGridLayout;
        dlg.setLayout(play);
        play->addWidget(new QLabel("Top"),0,0,1,1);
        play->addWidget(new QLabel("Bottom"),1,0,1,1);
        QComboBox *pcmTop = new QComboBox();
        QComboBox *pcmBot = new QComboBox();
        play->addWidget(pcmTop,0,1,1,1);
        play->addWidget(pcmBot,1,1,1,1);
          QStringList layers;
          const int offs = mp_setup->dataOffset>=0?mp_setup->dataOffset:0;
            foreach(const QStringList &l, m_imported_list.mid(offs))
            {
                if(col < l.count() ){
                    const QString & layer = l[col];
                    if(!layers.contains(layer)){
                        layers.append(layer);
                    }
                }
            }

        pcmTop->insertItems(0,layers);
        pcmBot->insertItems(0,layers);

        QDialogButtonBox *buttonBox = new QDialogButtonBox(&dlg);
        buttonBox->setOrientation(Qt::Horizontal);
        buttonBox->setStandardButtons(QDialogButtonBox::Cancel|QDialogButtonBox::Ok);
        play->addWidget(buttonBox, 2, 0, 1, 3);

        QObject::connect(buttonBox, SIGNAL(accepted()), &dlg, SLOT(accept()));
        QObject::connect(buttonBox, SIGNAL(rejected()), &dlg, SLOT(reject()));

        dlg.adjustSize();
        if(dlg.exec()==QDialog::Accepted){
            if(col>=0 && col <mp_model->columnCount()){
                mp_setup->dataModel.layerColon = col;
                mp_setup->dataModel.topLayerVal = pcmTop->currentText();
                 mp_setup->dataModel.botLayerVal = pcmBot->currentText();
                mp_model->setHeaderData(col,Qt::Horizontal,"Layer");
                ui->cbLayer->setChecked(1);
            }
        }


    }else{
        mp_setup->dataModel.layerColon=-1;
        mp_setup->dataModel.topLayerVal.clear();
        mp_setup->dataModel.botLayerVal.clear();
        ui->cbLayer->setChecked(1);
    }
}

void ImportDlg::slot_SetDescriptionColumn(bool on)
{
    QStandardItemModel *mp_model = static_cast<QStandardItemModel*>(ui->tvDataIn->model());
    if(on){
        int col = ui->tvDataIn->currentIndex().column();
        if(col>=0 && col <mp_model->columnCount()){
            mp_setup->dataModel.descrColon = col;
            mp_model->setHeaderData(col,Qt::Horizontal,"Descr");
            ui->cbDescr->setChecked(1);
        }
    }else{
        mp_setup->dataModel.descrColon = -1;
        ui->cbDescr->setChecked(1);
    }
}

void ImportDlg::makeMenu(const QPoint &p)
{
    QMenu m;
    QAction *paction;

        paction = m.addAction("Data start row",this,SLOT(slot_SetDataStart(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataOffset>=0    );
        paction = m.addAction("Project path cell",this,SLOT(slot_SetProjectPathCell(bool)));
        paction->setCheckable(1);
        paction->setChecked(   ! mp_setup->projectPath.isEmpty()  );
        paction = m.addAction("Part column",this,SLOT(slot_SetPartNameColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.partColon>=0   );

        paction = m.addAction("Value column",this,SLOT(slot_SetValueColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.valueColon>=0  );

        paction = m.addAction("PosX column",this,SLOT(slot_SetPosXColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.posXColon>=0 );

        paction = m.addAction("PosY column",this,SLOT(slot_SetPosYColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.posYColon>=0 );

        paction = m.addAction("Layer column",this,SLOT(slot_SetLayerColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.layerColon>=0 );

        // not mandatory
        paction = m.addAction("[Code column]",this,SLOT(slot_SetCodeColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.codeColon>=0   );

        paction = m.addAction("[Description column]",this,SLOT(slot_SetDescriptionColumn(bool)));
        paction->setCheckable(1);
        paction->setChecked(    mp_setup->dataModel.descrColon>=0 );

    m.exec(ui->tvDataIn->mapToGlobal(p));
}


