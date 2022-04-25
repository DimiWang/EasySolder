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
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QHash>
#include <QMap>
#include <QPointF>
#include <QString>
#include <QStringList>
#include "workarea.h"
#include <QListWidgetItem>
#include "partitem.h"


namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT
    Setup m_setup;
     QString m_currentProjectFileName;

    QString m_last_directory;
    explicit MainWindow();

public:
   static MainWindow *instance();
    void setLastDirectory(const QString &dir);
    QString lastDirectory();
    typedef enum {
        VALUE,
        ORDER_NO,
        DESCRIPTION,
        NAME
    } ItemSortBy;

    typedef enum {
        ITEM_VALUE,
        ITEM_ORDER_NO
    } BulkBy;

    typedef enum {
        FORWARD,
        BACKWARD
    } Direction;



    /* List grouped by value for soldering*/
    typedef  QMap<QString, QStringList> BulkByValue;
    typedef  QMap<QString, QStringList>::const_iterator BulkByValueIterator;


    QStringList getAsSortedListBy(ItemSortBy type);
    void populateTypes();
    BulkBy m_bulk_mode;
    void prepareBulkListBy(BulkBy mode);
    void nextPartsBulk(Direction dir);
    bool addPartsBulk();
    void collectPartsInfo();
    void clearLists();    
    void uiUpdateItemInfo(const QString &item_name);
    QString currentValue();

    void uiUpdateList();
    void rebuildTable();
    void uiUpdatePreview();

    void loadSettings();
    void saveSettings();
    const QString currentLayer();
    typedef enum {TOP_THEN_BOTTOM,TOM_AND_BOTTOM}Mode;

    ~MainWindow();


    void setTableCurrentLine(int line);
    int tableLines();
    QString tablePartNameSelected();
    QString tablePartNameByIndex(int index);
    int tableCurrentLine();
    void setTableText(int row, const QString &text1, const QString &text2, const QString &text3);
    void setTableRowColor(int row,const QColor &color);

    void clearTableSelection();
    typedef enum {RowSelect,RowMarkDone} MarkType;
    void tableSelect(QList<int> selList, MarkType type);
    void makeStatistics();
    bool loadProjectData();
    bool setupValid();
    void updateWindowTitle();
    void updateControlsEnable();
    void clearDoneStatus();
    void switchToTop();
    void switchToBottom();
protected:
    void resizeEvent(QResizeEvent *);
    void keyPressEvent(QKeyEvent *);
    void closeEvent(QCloseEvent *pev);

private slots:
    void on_pbNext_clicked();    
    void on_pbPrev_clicked();    
    void on_cmLayer_currentIndexChanged(int index);
    void slot_resort();
    void slot_apply_settings();
    void slot_repaint_preview();
    void slot_info(const QString &text);
    void slot_tvParts_itemDoubleClicked(QModelIndex index);
    void slot_changeVisibleLayer();
    void slot_openCsvFile();
    void slot_openAltiumFile();
    void slot_currentItemChanged(QModelIndex index);
    void slot_partContextMenu(const QPoint &);
    void slot_openProject();

    void on_cbBothLayers_toggled(bool checked);
    void slot_actionUseAsTP1();
    void slot_actionUseAsTP2();
    void on_pbFind_clicked();
    void on_pbReset_clicked();
    void on_cbHideDone_clicked();
    void on_leFind_textChanged(const  QString &arg1);
    void on_pbImportData_clicked();
    void on_pbSavePrj_clicked();
    void on_pbLoadPrj_clicked();

    void on_cmLayerImage_currentIndexChanged(int index);

private:
    int m_totalCountTop;
    int m_totalCountBot;
    int m_doneCountBot;
    int m_doneCountTop;
    bool m_table_need_rebuild;
    bool dataCorrect();





    bool projectSaved() const { return !m_currentProjectFileName.isEmpty();}
    void saveCurrentProject();
    bool isDataLoaded() const;
    void setupUi();

    Qt::GlobalColor markerColorSetting();
    int m_last_found_index;
    bool m_both_layers;
    bool m_soldering;
    bool m_data_loaded;
    /* current sort method*/
    ItemSortBy m_sort_by;
    Ui::MainWindow *ui;
    /* all parts */
    QHash<QString, PartItem> m_parts_map;
    /* lits of list (Bulk) iterator*/
    QString m_bulk_current_value;
    QString m_current_value;
    /* lits of list (Bulk)*/
    BulkByValue m_bulk;
    QLabel *mp_lbInfo;
    WorkScrolledArea *mp_area;
    QMenu *mp_menu;


};

#endif // MAINWINDOW_H
