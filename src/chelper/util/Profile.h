//
// Created by Yancey666 on 2024/2/22.
//

#ifndef CHELPER_PROFILE_H
#define CHELPER_PROFILE_H

#include <string>

//这个类是为了跟踪代码的运行，在遇到bug的时候方便排查错误的位置
namespace CHelper::Profile {

    void push(const std::string& str);

    void pop();

    void next(const std::string& str);

    void clear();

    std::string getStackTrace();

} // CHelper::Profile

#endif //CHELPER_PROFILE_H
