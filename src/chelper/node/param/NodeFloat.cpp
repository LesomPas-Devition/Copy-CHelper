//
// Created by Yancey on 2023/11/11.
//

#include <cfloat>
#include "NodeFloat.h"
#include "../../util/TokenUtil.h"

namespace CHelper::Node {

    NodeFloat::NodeFloat(const std::optional<std::string> &id,
                         const std::optional<std::string> &description,
                         const std::optional<float> &min,
                         const std::optional<float> &max)
            : NodeBase(id, description, false),
              min(min),
              max(max) {}

    NodeFloat::NodeFloat(const nlohmann::json &j,
                         [[maybe_unused]] const CPack &cpack)
            : NodeBase(j, true),
              min(FROM_JSON_OPTIONAL(j, min, float)),
              max(FROM_JSON_OPTIONAL(j, max, float)) {}

    std::shared_ptr<NodeType> NodeFloat::getNodeType() const {
        return NodeType::FLOAT;
    }

    void NodeFloat::toJson(nlohmann::json &j) const {
        NodeBase::toJson(j);
        TO_JSON_OPTIONAL(j, min)
        TO_JSON_OPTIONAL(j, max)
    }

    ASTNode NodeFloat::getASTNode(TokenReader &tokenReader, const CPack &cpack) const {
        return tokenReader.readFloatASTNode(this);
    }

    static std::optional<float> str2float(const std::string &string) {
        double result;
        std::stringstream stringStream;
        stringStream << string;
        stringStream >> std::setprecision(16) >> result;
        stringStream.clear();
        if (result < FLT_MIN || result > FLT_MAX) {
            return std::nullopt;
        }
        return static_cast<float>(result);
    }

    bool NodeFloat::collectIdError(const ASTNode *astNode,
                                   std::vector<std::shared_ptr<ErrorReason>> &idErrorReasons) const {
        if (astNode->isError()) {
            return true;
        }
        std::string str = TokenUtil::toString(astNode->tokens);
        std::optional<float> num = str2float(str);
        if (!num.has_value() || (min.has_value() && num.value() < min) || (max.has_value() && num.value() > max)) {
            idErrorReasons.push_back(ErrorReason::idError(astNode->tokens, std::string("数值不在范围")
                    .append("[").append(std::to_string(min.value_or(FLT_MIN)))
                    .append(", ").append(std::to_string(max.value_or(FLT_MAX)))
                    .append("]").append("内 -> ").append(str)));
        }
        return true;
    }

    void NodeFloat::collectStructure(const ASTNode *astNode,
                                     StructureBuilder &structure,
                                     bool isMustHave) const {
        structure.append(isMustHave, description.value_or("数字"));
    }

} // CHelper::Node
