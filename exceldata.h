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


#ifndef EXCELDATA_H
#define EXCELDATA_H

#include <QObject>
#include <QAxObject>
#include <QThread>

class ExcelData : public QObject
{
    Q_OBJECT
public:
    explicit ExcelData(const QString filename);
    /* opens excel document*/
    bool open(char mode);
    /* returns if document is opened*/
    bool isOpen() const;
    /* add sheet with name to opened workbook*/
    bool addSheet(const QString sheetname);
    /* sets current sheet name*/
    bool setCurrentSheet(const QString sheetname);
    /* removes sheet sheetname*/
    bool removeSheet(const QString sheetname);
    /* write data to cell*/
    bool write(qint32 i, qint32 j, const QVariant data);
    /* reads data from cell*/
    bool read(qint32 i, qint32 j, QVariant &data);
    /* sets color of cell*/
    bool setColor(qint32 i, qint32 j, const QColor background, const QColor foreground);
    /* gets color of cell */
    bool color(qint32 i, qint32 j, QColor &background, QColor &foreground);
    /* sets visible workbook*/
    void setVisible(bool visible);
    /* closes workbook*/
    void close(void);
    /* saves workbook to file*/
    bool save();

private:
    QAxObject *mp_excel;
    QAxObject *mp_workbook;
    QAxObject *mp_sheet;
    QString m_filename;
    QString m_sheetname;
    bool m_opened;

signals:

public slots:

};



#endif // EXCELDATA_H
