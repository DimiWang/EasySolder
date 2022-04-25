#include <iostream>
#include <fstream>
#include <ctime>
#include <stdlib.h>
#include <string.h>

using namespace std;

typedef enum Mode{ModeDateTime , /**/
                  ModeDateTimeStruct,/**/
                  ModeVersion,/**/
                  ModeVersionBinary/**/
                 } VersionEnum_t;

VersionEnum_t mode ;
unsigned int minor=0;
unsigned int major=0;
bool newVersion=false;
time_t rawtime;
struct tm * timeinfo = NULL;
static char tmp[100]={0};

string version_text;
string date_time_text;
string out_file_name;

bool generateFiles(void)
{
    ofstream file;
    //saving version h file
    file.open( out_file_name.c_str());
    if(!file.is_open())
    {
        cerr <<endl<< "Unable to open file:"<<out_file_name<<endl;
        return false;
    }
    file <<    "#ifndef VERSION_H "<<endl
         <<    "#define VERSION_H"<<endl;
    if(mode != ModeDateTime) {
        file << "#define VERSION 0x"<< version_text<<endl;
    }    
    file <<     "#define VERSION_DATE \"" << date_time_text<< "\""<<endl;
    file <<     "#endif"<<endl;;
    file.close();

    //saving app.inf file
    file.open ("app.inf");
    file << date_time_text<<endl<<version_text;
    file.close();
    return true;
}

string currentDateTime()
{
    string str;
    time ( &rawtime );
    timeinfo = localtime ( &rawtime );
    str = ctime (&rawtime) ;
    str.erase(str.find('\n', 0), 1);
    //str.erase(str.find('\r', 0), 1);
    return str;
}

bool parseApp_inf(){
    string str;
    bool parse_ok =  false;
    bool app_file_present = false;
    ifstream iff;
    iff.open("app.inf");
    if(iff.is_open() )    {
        app_file_present = true;
        cout<<">>app file present"<<endl;
        iff.getline(tmp,100);//ignore line
        iff.getline(tmp,100);
    }
    switch(mode)
    {
        case ModeDateTimeStruct:
        case ModeDateTime:        {
             parse_ok = true;
        }
        break;
        case ModeVersion:        {
            if(app_file_present &&  sscanf(tmp,"\"%02d.%04d\"",&major,&minor)) {
                parse_ok = true;
            }
        }
        break;
        case ModeVersionBinary:        {
            if(app_file_present && sscanf(tmp,"%04x%04x",&major,&minor)) {
                 parse_ok = true;
            }
        }
        break;
    }
    if(parse_ok == false) {
        cerr<<">>bad app.inf file(second line:"<<string(tmp)<<")"<<endl;
    }
    return parse_ok;
}

bool obtainVersion(void){

    /* getting current time*/
    date_time_text = string(currentDateTime());
    major=1;
    minor=0;

    if(parseApp_inf()){
        if(newVersion) {
            major++;
            minor=0;
        }
        else minor++;
    }

    switch(mode)
    {
        //generates date time format independently
        case ModeDateTime:
        {
        }
        break;

        case ModeDateTimeStruct:
        {
            sprintf(tmp,"{{%du,%du,%du},{%du,%du,%du}}"
                    ,timeinfo->tm_mday, timeinfo->tm_mon + 1, (unsigned int)timeinfo->tm_year+1900,
                    timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);
        }
        break;

        case ModeVersion:
        {
            sprintf(tmp,"\"%02d.%04d\"",major,minor);
        }
        break;

        case ModeVersionBinary:
        {
            sprintf(tmp,"%04x%04x",major,minor);
        }
        break;
        default:
            break;
    }
    version_text = string(tmp);
    return generateFiles();
}

int main(int argc, char *argv[])
{
    int i;
    /* default*/
    out_file_name = "version.h";
    mode =  ModeDateTime;
    if(argc>1)  {
        for(i=1; i < argc; i++ )  {
            char *p=(char*)argv[i];
            if(p[0] == '-')  {
                 switch(p[1]){
                 case ('m'):
                     switch(p[2]){
                        case ('v'):
                            mode = ModeVersion;
                         break;
                        case ('d'):
                             mode = ModeDateTime;
                             if(p[3] == 's'){
                                 mode = ModeDateTimeStruct;
                             }
                        break;
                        case ('b'):
                             mode=ModeVersionBinary;
                        break;
                        default:break;
                     }
                 break;

                 case ('n'):
                     newVersion=true;
                 break;

                 case ('f'):
                     out_file_name = string(&p[2]);
                 break;
                }                 
            }
        }/*for(i=1; i < argc; i++ )*/
    }/*if(argc>1) */
    else {
        cout<<"\t*****Version generation utility 1.03*****"<<endl;
        cout<<"\tvrgen.exe [-m,v,b,ds,d] [-n] [-f\"FileName\"]"<<endl;
        cout<<"\t\t-mv - version in format 0001.0001 -> 0001.0002 ->,..."<<endl;
        cout<<"\t\t-n - new major version 00010001 -> 00020000"<<endl;
        cout<<"\t\t-mb - binary 00010001 ->00010002 ->,..."<<endl;
        cout<<"\t\t-md or no parameter - format  Tue Apr 23 10:37:59 2013"<<endl;
        cout<<"\t\t-mds - {{dd,MM,yyyy},{hh,mm,ss}}"<<endl;
    }
    if(obtainVersion() == true){
        cout<<endl<<"Ok"<<endl<<date_time_text<<endl<<version_text;
    }
    return 0;
}

