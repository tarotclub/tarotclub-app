#ifndef ASSETS_H
#define ASSETS_H

#include <string>

class Assets
{
public:
    struct Meta {
        char *data;
        int size;

        Meta() {
            data = nullptr;
            size = 0;
        }
    };

    Assets();

    static bool Get(const std::string &name, std::string &out);
};

#endif // ASSETS_H
