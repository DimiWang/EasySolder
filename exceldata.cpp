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


#include "exceldata.h"
#include <QMessageBox>
#include <QFile>
#include <QDebug>
#include <QRgb>



/** @brief Constructor
 *
 *
 *  @param const QString filename  - file name *.xlsx
 *  @return void
 */
ExcelData::ExcelData(const QString filename) :
    QObject(0)
{
    m_opened = false;
    mp_excel =  0;
    mp_workbook = 0;
    mp_sheet = 0;
    m_filename = filename;
    mp_excel = new QAxObject( "Excel.Application", 0 );
    if (mp_excel)
    {
        mp_excel->setProperty("DisplayAlerts", false);
    }
    else
    {

    }
}


/** @brief opens excel file for read or write
 *
 *
 *  @param char mode - mode 'r' -read 'w'-write
 *  @return bool success = true if success
 */
bool ExcelData::open(char mode)
{
    bool result = false;
    do
    {
        /* check if opened and file name is empty*/
        if (mp_excel == NULL || m_opened || m_filename.isEmpty()) break;

        /* try open workbooks*/
        QAxObject *workbooks = mp_excel->querySubObject( "Workbooks" );
        if (workbooks == NULL) break;

        bool file_exists = QFile::exists(m_filename);
        if (file_exists)
        {
            mode = 'r';
        }
        if (mode == 'r' )
        {
            mp_workbook = workbooks->querySubObject( "Open(const QString&)", m_filename );
        }
        else if (mode == 'w' )
        {
            mp_workbook = workbooks->querySubObject("Add()");
            if (mp_workbook == NULL) break;

            mp_workbook->dynamicCall("SaveAs(const QString &)", m_filename);
            mp_workbook->dynamicCall("Close()");

            /* and try to open newly created file */
            mp_workbook = workbooks->querySubObject( "Open(const QString&)", m_filename );
        }
        result = mp_workbook != 0;
        m_opened = result;
    }
    while (0);
    return result;
}

/** @brief Check if file is opened
 *
 *
 *  @param
 *  @return is opened = true
 */
bool ExcelData::isOpen() const
{
    return m_opened;
}


/** @brief Adds sheet to workbook
 *
 *
 *  @param const QString sheetname  - sheet name
 *  @return ( bool ) - success = true
 */
bool ExcelData::addSheet(const QString sheetname)
{
    if (m_opened)
    {
        QAxObject *sheets = mp_workbook->querySubObject( "Worksheets" );
        if ( sheets )
        {
            sheets->dynamicCall("Add()");
            mp_sheet = sheets->querySubObject( "Item( int )", 1 );
            if (mp_sheet)
            {
                mp_sheet->setProperty("Name", sheetname);
                return true;
            }
        }
    }
    return false;
}

/** @brief sheetn name
 *
 *
 *  @param const QString sheetname  -  Sets active sheet
 *  @return( bool ) - success = true
 */
bool ExcelData::setCurrentSheet(const QString sheetname)
{
    if (m_opened)
    {
        QAxObject *sheets = mp_workbook->querySubObject("Sheets (const QString&)", sheetname);
        if ( sheets )
        {
            sheets->dynamicCall("Activate");
            QAxObject *sheet  = mp_workbook->querySubObject("ActiveSheet");
            if (sheet )
            {
                mp_sheet = sheet;
                return true;
            }
        }
    }
    return false;
}

/** @brief removes sheet
 *
 *
 *  @param const QString sheetname
 *  @return ( bool ) success = true
 */
bool ExcelData::removeSheet(const QString sheetname)
{
    if (m_opened)
    {
        QAxObject *sheets = mp_workbook->querySubObject("Sheets (const QString&)", sheetname);
        if ( sheets )
        {
            sheets->dynamicCall("Delete()");
            return true;
        }
    }
    return false;;
}

/** @brief
 *
 *
 *  @param
 *  @return( bool ) success = true
 */
bool ExcelData::save(void)
{
    if ( mp_workbook )
    {
        mp_workbook->dynamicCall( "Save()");
        return true;
    }
    return false;
}

/** @brief writes data to cell i,j
 *
 *
 *  @param  qint32 i
 *          qint32 j
 *          const QVariant data
 *  @return ( bool ) success = true
 */
bool ExcelData::write(qint32 i, qint32 j, const QVariant data)
{
    if (m_opened && i > 0 && j > 0 && mp_sheet)
    {
        QAxObject *cell = mp_sheet->querySubObject("Cells( int, int)", i, j);
        if ( cell )
        {
            cell->setProperty("Value", data);
            return true;
        }
    }
    return false;
}

/** @brief rads data from cell i,j
 *  @param qint32 i
 *          qint32 j
 *          QVariant &data
 *  @return ( bool ) success = true
 */
bool ExcelData::read(qint32 i, qint32 j, QVariant &data)
{
    bool result = false;
    if (m_opened && i > 0 && j > 0 && mp_sheet)
    {
        QAxObject *cell = mp_sheet->querySubObject("Cells( int, int)", i, j);
        if ( cell )
        {
            data = cell->property("Value");
            result = true;
        }
    }
    return result;
}

/** @brief Sets color of background ,foreground cell
 *
 *
 *  @param qint32 i
 *    qint32 j
 *    const QColor background
 *    const QColor foreground
 *  @return ( bool ) - success = true
 */
bool ExcelData::setColor(qint32 i, qint32 j, const QColor background, const QColor foreground)
{
    bool result = false;
    if (m_opened && i > 0 && j > 0 && mp_sheet)
    {
        QAxObject *cell = mp_sheet->querySubObject("Cells( int, int)", i, j);
        if ( cell )
        {
            QAxObject *interior = cell->querySubObject("Interior");
            QAxObject *font = cell->querySubObject("Font");
            if ( interior && font)
            {
                quint32 color;
                color = (background.red() << 0) | (background.green() << 8) | (background.blue() << 16);
                result = interior->setProperty("Color", QVariant(color));
                color = (foreground.red() << 0) | (foreground.green() << 8) | (foreground.blue() << 16);
                result &= font->setProperty("Color", QVariant(color));
                result = true;
            }
        }
    }
    return result;
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
/****************************************************************************
 * @function name: ExcelData::color()
 *
 * @param:
 *      qint32 i
 *      qint32 j
 *      QColor &background
 *      QColor &foreground
 * @description: gets color of background and foregorund
 * @return: ( bool ) success = true
 ****************************************************************************/
bool ExcelData::color(qint32 i, qint32 j, QColor &background, QColor &foreground)
{
    bool result = false;
    if (m_opened && i > 0 && j > 0 && mp_sheet)
    {
        QAxObject *cell = mp_excel->querySubObject("Cells( int, int)", i, j);
        if ( cell )
        {
            QAxObject *interior = cell->querySubObject("Interior");
            QAxObject *font = cell->querySubObject("Font");
            if ( interior && font)
            {
                QVariant color1 = interior->property("Color");
                background = QColor::fromRgb(color1.toInt());
                QVariant color2 = font->property("Color");
                foreground = QColor::fromRgba(color2.toInt());
                result = true;
            }
        }
    }
    return result;
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
/****************************************************************************
 * @function name: ExcelData::setVisible()
 *
 * @param:
 *
 *      bool visible
 * @description: sets visibility of workbook
 * @return: ( void )
 ****************************************************************************/
void ExcelData::setVisible(bool visible)
{
    if ( mp_excel)
    {
        mp_excel->setProperty("Visible", visible);
    }
}

/** @brief
 *
 *
 *  @param
 *  @return
 */
/****************************************************************************
 * @function name: ExcelData::close()
 *
 * @param:
 *             void
 * @description: closes workbook
 * @return: ( void )
 ****************************************************************************/
void ExcelData::close()
{
    try
    {
        this->save();
        mp_workbook->dynamicCall( "Close()");
        delete mp_excel;
    }
    catch (...)
    {
        m_opened = false;
    }
    mp_sheet = NULL;
    mp_workbook = NULL;
    mp_excel =  NULL;
}


