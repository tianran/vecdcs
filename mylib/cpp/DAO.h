#ifndef __DAO_H
#define __DAO_H

#include <iostream>
#include <string>

namespace DAO {
    
    void write_magic(std::ostream& os, const std::string& magic);
    void write_double(std::ostream& os, double value);
    void write_float(std::ostream& os, float value);
    void write_int(std::ostream& os, int value);
    void write_long(std::ostream& os, long value);

    std::string read_magic(std::istream& is);
    double read_double(std::istream& is);
    float read_float(std::istream& is);
    int read_int(std::istream& is);
    long read_long(std::istream& is);
}

#endif //__DAO_H
