//
// Created by Yancey on 2024/2/20.
//

#include "NodeTargetSelectorArgument.h"

namespace CHelper::Node {

    NodeTargetSelectorArgument::NodeTargetSelectorArgument(const std::optional<std::string> &id,
                                                           const std::optional<std::string> &description)
            : NodeBase(id, description, false) {}

    ASTNode NodeTargetSelectorArgument::getASTNode(TokenReader &tokenReader) const {
        //TODO 目标选择器参数
        return tokenReader.getStringASTNode(this);
    }

} // CHelper::Node