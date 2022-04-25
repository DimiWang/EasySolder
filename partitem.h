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
#ifndef PARTITEM_H
#define PARTITEM_H


struct PartItem
{
    typedef enum
    {
        NONE,
        RESISTOR,
        CAPACITOR,
        DIODE,
        TRANSISTOR,
        OTHER,
        COUNT
    } PartType;

    PartItem() {type = NONE; done =false;}
    QString name;//designator
    PartType type;
    QString order_no;
    QString value;
    QString layer;
    QPointF position;
    QString description;

    double rotation;
    double footprint;
    double mid_x;
    double mid_y;
    double ref_x;
    double ref_y;
    double pad_x;
    double pad_y;

    bool done;
};


struct DataModel{
    DataModel(){
        partColon = -1;
        codeColon = -1;
        posXColon = -1;
        posYColon = -1;
        descrColon = -1;
        layerColon = -1;
        valueColon = -1;
    }

    QString format;
    int partColon;
    int codeColon;
    int posXColon;
    int posYColon;
    int descrColon;
    int layerColon;
    int valueColon;
    QString topLayerVal;
    QString botLayerVal;
};

struct Setup{
    Setup(){
        dataOffset =-1;
    }

    QString dataFileName;
    QString topImageFileName;
    QString bottomImageFileName;
    QString projectPath;
    QString projectName;
    int dataOffset;    
    DataModel dataModel;

};
#endif // PARTITEM_H
