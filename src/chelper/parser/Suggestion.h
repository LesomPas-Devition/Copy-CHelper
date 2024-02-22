//
// Created by Yancey666 on 2024/2/21.
//

#ifndef CHELPER_SUGGESTION_H
#define CHELPER_SUGGESTION_H

#include "../util/VectorView.h"
#include "../resources/id/NormalId.h"
#include "../lexer/Token.h"

namespace CHelper {

    class Suggestion {
        //TODO 建议的优先级
        //要被替换的内容
        const VectorView<Token> tokens;
        //内容
        std::shared_ptr<NormalId> content;

    public:
        Suggestion(const VectorView<Token> &tokens, const std::shared_ptr<NormalId> &content);
    };

} // CHelper

#endif //CHELPER_SUGGESTION_H
