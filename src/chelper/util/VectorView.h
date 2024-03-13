//
// Created by Yancey on 2023/12/18.
//

#pragma once

#ifndef CHELPER_VECTORVIEW_H
#define CHELPER_VECTORVIEW_H

#include "Exception.h"

namespace CHelper {

    template<class T>
    class VectorView {
    public:
        std::shared_ptr<std::vector<T>> vector;
        size_t start, end;

        VectorView(const std::shared_ptr<std::vector<T>> vector, size_t start, size_t end)
                : vector(vector),
                  start(start),
                  end(end) {
#if CHelperDebug == true
            if (start > end) {
                throw Exception::WrongRange(start, end);
            }
#endif
        }

        [[nodiscard]] bool isEmpty() const {
            return start >= end;
        };

        [[nodiscard]] bool hasValue() const {
            return start < end;
        };

        const T &operator[](size_t which) const {
            return (*vector)[start + which];
        }

        [[nodiscard]] size_t size() const {
            return end - start;
        }

        template<typename Function>
        Function forEach(Function function) const {
            return std::for_each(vector->begin() + start, vector->begin() + end, function);
        }
    };

} // CHelper

#endif //CHELPER_VECTORVIEW_H
