//
// Created by Yancey on 2024/2/28.
//

#include "NodeJsonFloat.h"
#include "../../util/TokenUtil.h"

namespace CHelper::Node {

    NodeJsonFloat::NodeJsonFloat(const std::optional<std::string> &id,
                                 const std::optional<std::string> &description,
                                 const std::optional<float> &min,
                                 const std::optional<float> &max)
            : NodeBase(id, description, false),
              min(min),
              max(max) {}

    NodeJsonFloat::NodeJsonFloat(const nlohmann::json &j,
                                 [[maybe_unused]] const CPack &cpack)
            : NodeBase(j, false),
              min(JsonUtil::fromJsonOptionalUnlikely<float>(j, "min")),
              max(JsonUtil::fromJsonOptionalUnlikely<float>(j, "max")) {}

    NodeType *NodeJsonFloat::getNodeType() const {
        return NodeType::JSON_FLOAT.get();
    }

    void NodeJsonFloat::toJson(nlohmann::json &j) const {
        NodeBase::toJson(j);
        JsonUtil::toJsonOptionalUnlikely(j, "min", min);
        JsonUtil::toJsonOptionalUnlikely(j, "max", max);
    }

    ASTNode NodeJsonFloat::getASTNode(TokenReader &tokenReader, const CPack *cpack) const {
        return tokenReader.readFloatASTNode(this);
    }

    bool NodeJsonFloat::collectIdError(const ASTNode *astNode,
                                       std::vector<std::shared_ptr<ErrorReason>> &idErrorReasons) const {
        if (HEDLEY_UNLIKELY(astNode->isError())) {
            return true;
        }
        std::string str = TokenUtil::toString(astNode->tokens);
        char *end;
        float value = std::strtof(str.c_str(), &end);
        if (HEDLEY_UNLIKELY(end == str.c_str() || *end != '\0' ||
                            value == HUGE_VALF || value == -HUGE_VALF ||
                            (min.has_value() && value < min) ||
                            (max.has_value() && value > max))) {
            idErrorReasons.push_back(ErrorReason::idError(astNode->tokens, std::string("数值不在范围")
                    .append("[").append(std::to_string(min.value_or(std::numeric_limits<float>::lowest())))
                    .append(", ").append(std::to_string(max.value_or(std::numeric_limits<float>::max())))
                    .append("]").append("内 -> ").append(str)));
        }
        return true;
    }

} // CHelper::Node