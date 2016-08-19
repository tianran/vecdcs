#include "DAO.h"

using namespace std;

/* intended to be compatibale with Intel x86 and java:
 *  Little-Endian is used by Intel x86 processors,
 *  whereas JVM uses Big-Endian */
namespace DAO {
    
    void write_magic(ostream& os, const string& magic) {
        os.write(magic.c_str(), 4);
    }

    void write_double(ostream& os, double value) {
        char* cast = reinterpret_cast<char *>(&value);
        char rev[sizeof(value)];
        for (size_t i = 0; i < sizeof(value); ++i) {
            rev[i] = cast[sizeof(value) - 1 - i];
        }
        os.write(rev, sizeof(value));
    }

    void write_float(ostream& os, float value) {
        char* cast = reinterpret_cast<char *>(&value);
        char rev[sizeof(value)];
        for (size_t i = 0; i < sizeof(value); ++i) {
            rev[i] = cast[sizeof(value) - 1 - i];
        }
        os.write(rev, sizeof(value));
    }

    void write_int(ostream& os, int value) {
        char* cast = reinterpret_cast<char *>(&value);
        char rev[sizeof(value)];
        for (size_t i = 0; i < sizeof(value); ++i) {
            rev[i] = cast[sizeof(value) - 1 - i];
        }
        os.write(rev, sizeof(value));
    }

    void write_long(ostream& os, long value) {
        char* cast = reinterpret_cast<char *>(&value);
        char rev[sizeof(value)];
        for (size_t i = 0; i < sizeof(value); ++i) {
            rev[i] = cast[sizeof(value) - 1 - i];
        }
        os.write(rev, sizeof(value));
    }

    string read_magic(istream& is) {
        char buff[4];
        is.read(buff, 4);
        return string(buff, 4);
    }

    double read_double(istream& is) {
        double value;
        char rev[sizeof(value)];
        is.read(rev, sizeof(value));
        char* cast = reinterpret_cast<char *>(&value);
        for (size_t i = 0; i < sizeof(value); ++i) {
            cast[i] = rev[sizeof(value) - 1 - i];
        }
        return value;
    }

    float read_float(istream& is) {
        float value;
        char rev[sizeof(value)];
        is.read(rev, sizeof(value));
        char* cast = reinterpret_cast<char *>(&value);
        for (size_t i = 0; i < sizeof(value); ++i) {
            cast[i] = rev[sizeof(value) - 1 - i];
        }
        return value;
    }

    int read_int(istream& is) {
        int value;
        char rev[sizeof(value)];
        is.read(rev, sizeof(value));
        char* cast = reinterpret_cast<char *>(&value);
        for (size_t i = 0; i < sizeof(value); ++i) {
            cast[i] = rev[sizeof(value) - 1 - i];
        }
        return value;
    }

    long read_long(istream& is) {
        long value;
        char rev[sizeof(value)];
        is.read(rev, sizeof(value));
        char* cast = reinterpret_cast<char *>(&value);
        for (size_t i = 0; i < sizeof(value); ++i) {
            cast[i] = rev[sizeof(value) - 1 - i];
        }
        return value;
    }
}
