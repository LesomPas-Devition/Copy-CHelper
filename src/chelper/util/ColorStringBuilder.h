//
// Created by Yancey666 on 2024/2/22.
//

#ifndef CHELPER_COLORSTRINGBUILDER_H
#define CHELPER_COLORSTRINGBUILDER_H

#include <string>

namespace CHelper{

    class ColorStringBuilder {
    private:
        std::string content;

        ColorStringBuilder& append(const std::string& color, const std::string& str);
    public:
        ColorStringBuilder& normal(const std::string& str);

        ColorStringBuilder& grey(const std::string& str);

        ColorStringBuilder& red(const std::string& str);

        ColorStringBuilder& green(const std::string& str);

        ColorStringBuilder& yellow(const std::string& str);

        ColorStringBuilder& blue(const std::string& str);

        ColorStringBuilder& purple(const std::string& str);

        ColorStringBuilder& cyan(const std::string& str);

        std::string build();

    };

} // CHelper

#endif //CHELPER_COLORSTRINGBUILDER_H
