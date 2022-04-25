#ifndef IMPORTDLG_H
#define IMPORTDLG_H

#include <QDialog>
#include "partitem.h"
#include <QHash>
#include <QString>
#include <QVariantMap>


namespace Ui {
class ImportDlg;
}

class ImportDlg : public QDialog
{
    Q_OBJECT

public:
    explicit ImportDlg(Setup *setup);
    ~ImportDlg();
    int exec();


    static int  splitCsv(QStringList *plist, const QString &text);
    static bool convertDataTable(QHash<QString, PartItem > *partsMap
                                 , const QList<QStringList> &importData
                                 , const DataModel &dataModel );

    static QList<QStringList> loadCSVAltium(const QString &fileName);
    static QList<QStringList> loadInputDataFile(const QString &fileName, const QString &format);
    void updateControlsEnabled();
    QString columnName(int i);
    void setLastDirectory(const QString &lastDir);
private slots:
    void on_pbOpen_clicked();
    void on_pbOpenBottom_clicked();
    void on_pbOpenTop_clicked();
    void slot_contextMenu(const QPoint &p);
    void slot_SetDataStart(bool);
    void slot_SetProjectPathCell(bool);
    //columns
    void slot_SetPartNameColumn(bool);
    void slot_SetCodeColumn(bool);
    void slot_SetValueColumn(bool);
    void slot_SetPosXColumn(bool);
    void slot_SetPosYColumn(bool);
    void slot_SetLayerColumn(bool);
    void slot_SetDescriptionColumn(bool);

private:
    void makeMenu( const QPoint &p);

    Setup *mp_setup;

    QString m_last_directory;
    bool checkValid();
    QList<QStringList> m_imported_list;
    Ui::ImportDlg *ui;

    void showImportedFile();

 private slots:
    virtual void accept();
};

#endif // IMPORTDLG_H
