
#include "adapt.h"

std::string QStringToString(QString qstr)
{
    QByteArray ba = qstr.toLocal8Bit();
    const char *c_str2 = ba.data();
    std::string str(c_str);
    return str;
}
