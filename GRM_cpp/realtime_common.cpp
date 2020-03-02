#include "grm.h"
string IO_Path_ChangeDrive(char strV, string strPath)
{
    if (strPath.substr(1, 1) != ":")    {
        return "-1";
    }
    return strV + strPath.substr(1);
}
