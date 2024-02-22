//
// Created by Yancey666 on 2023/12/15.
//

#include "ASTNode.h"
#include "../node/NodeBase.h"
#include "../util/StringUtil.h"
#include "../util/TokenUtil.h"

namespace CHelper {

    ASTNode::ASTNode(ASTNodeMode::ASTNodeMode mode,
                     const Node::NodeBase *node,
                     const std::vector<ASTNode> &childNodes,
                     const VectorView<Token> &tokens,
                     const std::vector<std::shared_ptr<ErrorReason>> &errorReasons,
                     std::string id,
                     int whichBest)
            : mode(mode),
              node(node),
              childNodes(childNodes),
              tokens(tokens),
              errorReasons(errorReasons),
              id(std::move(id)),
              whichBest(whichBest) {}

    ASTNode ASTNode::simpleNode(const Node::NodeBase *node,
                                const VectorView<Token> &tokens,
                                const std::shared_ptr<ErrorReason> &errorReason,
                                const std::string &id) {
        std::vector<std::shared_ptr<ErrorReason>> errorReasons;
        if (errorReason != nullptr) {
            errorReasons.push_back(errorReason);
        }
        return {ASTNodeMode::NONE, node, {}, tokens, errorReasons, id};
    }


    ASTNode ASTNode::andNode(const Node::NodeBase *node,
                             const std::vector<ASTNode> &childNodes,
                             const VectorView<Token> &tokens,
                             const std::shared_ptr<ErrorReason> &errorReason,
                             const std::string &id) {
        if (errorReason != nullptr) {
            return {ASTNodeMode::AND, node, childNodes, tokens, {errorReason}, id};
        }
        for (const auto &item: childNodes) {
            if (item.isError()) {
                return {ASTNodeMode::AND, node, childNodes, tokens, item.errorReasons, id};
            }
        }
        return {ASTNodeMode::AND, node, childNodes, tokens, {}, id};
    }

    ASTNode ASTNode::orNode(const Node::NodeBase *node,
                            const std::vector<ASTNode> &childNodes,
                            const std::optional<VectorView<Token>> &tokens,
                            const std::shared_ptr<ErrorReason> &errorReason,
                            const std::string &id) {
        bool isError = true;
        int whichBest = 0;
        size_t start = 0;
        std::vector<std::shared_ptr<ErrorReason>> errorReasons;
        for (int i = 0; i < childNodes.size(); ++i) {
            const ASTNode &item = childNodes[i];
            if (!item.isError()) {
                if (isError) {
                    start = 0;
                }
                isError = false;
                whichBest = i;
                errorReasons.clear();
            }
            if (!isError) {
                continue;
            }
            for (const auto &item2: item.errorReasons) {
                if (start > item2->tokens.start) {
                    continue;
                }
                bool isAdd = true;
                if (start < item2->tokens.start) {
                    start = item2->tokens.start;
                    whichBest = i;
                    errorReasons.clear();
                } else {
                    for (const auto &item3: errorReasons) {
                        if (*item2 == *item3) {
                            isAdd = false;
                        }
                    }
                }
                if (isAdd) {
                    errorReasons.push_back(item2);
                }
            }
        }
        if (errorReason != nullptr) {
            errorReasons = {errorReason};
        }
        return {ASTNodeMode::OR, node, childNodes, tokens.value_or(childNodes[whichBest].tokens),
                errorReasons, id, whichBest};
    }

    bool ASTNode::isError() const {
        return !errorReasons.empty();
    }

    bool ASTNode::hasChildNode() const {
        return !childNodes.empty();
    }

    std::optional<std::string> ASTNode::collectDescription(size_t index) const {
        std::optional<std::string> description = node->getDescription(this, index);
        if (description.has_value()) {
            return description;
        }
        switch (mode) {
            case ASTNodeMode::NONE:
                return std::nullopt;
            case ASTNodeMode::AND:
                for (const ASTNode &astNode: childNodes) {
                    description = astNode.collectDescription(index);
                    if (description.has_value()) {
                        return description;
                    }
                }
                return std::nullopt;
            case ASTNodeMode::OR:
                return childNodes[whichBest].collectDescription(index);
        }
        return std::nullopt;
    }

    //创建AST节点的时候只得到了结构的错误，ID的错误需要调用这个方法得到
    void ASTNode::collectIdErrors(std::vector<std::shared_ptr<ErrorReason>> &idErrorReasons) const {
        if (node->collectIdError(this, idErrorReasons)) {
            return;
        }
        switch (mode) {
            case ASTNodeMode::NONE:
                break;
            case ASTNodeMode::AND:
                for (const ASTNode &astNode: childNodes) {
                    astNode.collectIdErrors(idErrorReasons);
                }
                break;
            case ASTNodeMode::OR:
                childNodes[whichBest].collectIdErrors(idErrorReasons);
                break;
        }
    }

    void ASTNode::collectSuggestions(std::vector<Suggestion> &suggestions, size_t index) const {
        if (index < TokenUtil::getStartIndex(tokens) || index > TokenUtil::getEndIndex(tokens)) {
            return;
        }
        if (node->collectSuggestions(this, suggestions)) {
            return;
        }
        switch (mode) {
            case ASTNodeMode::NONE:
                break;
            case ASTNodeMode::AND:
                for (const ASTNode &astNode: childNodes) {
                    astNode.collectSuggestions(suggestions, index);
                }
                break;
            case ASTNodeMode::OR:
                childNodes[whichBest].collectSuggestions(suggestions, index);
                break;
        }
    }

    void ASTNode::collectStructure(StructureBuilder &structure) const {
        node->collectStructure(this, structure);
        if (structure.isDirty()) {
            return;
        }
        switch (mode) {
            case ASTNodeMode::NONE:
                structure.appendUnknownIfNotDirty();
                break;
            case ASTNodeMode::AND:
                for (const ASTNode &astNode: childNodes) {
                    astNode.collectStructure(structure);
                }
                break;
            case ASTNodeMode::OR:
                childNodes[whichBest].collectStructure(structure);
                break;
        }
    }

    std::string ASTNode::getDescription(size_t index) const {
        return collectDescription(index).value_or("未知");
    }

    std::vector<std::shared_ptr<ErrorReason>> ASTNode::getErrorReasons() const {
        //TODO 根据错误等级调整错误的位置
        //因为大多数情况下优先先显示ID错误，所以先添加ID错误
        std::vector<std::shared_ptr<ErrorReason>> result;
        collectIdErrors(result);
        for (const auto &item: errorReasons) {
            result.push_back(item);
        }
        return result;
    }

    std::vector<Suggestion> ASTNode::getSuggestions(size_t index) const {
        std::vector<Suggestion> result;
        collectSuggestions(result, index);
        return result;
    }

    std::string ASTNode::getStructure() const {
        StructureBuilder structureBuilder;
        collectStructure(structureBuilder);
        return structureBuilder.build();
    }

    std::string ASTNode::getColors() const {
        //TODO getColors()
        return node->getNodeType().nodeName;
    }

    std::ostream &operator<<(std::ostream &os, const CHelper::ASTNode &astNode) {
        os << R"({"isError": )"
           << (astNode.isError() ? "true" : "false")
           << R"(, "mode": )";
        switch (astNode.mode) {
            case CHelper::ASTNodeMode::NONE:
                os << R"("NONE")";
                break;
            case CHelper::ASTNodeMode::AND:
                os << R"("AND")";
                break;
            case CHelper::ASTNodeMode::OR:
                os << R"("OR")";
                break;
            default:
                os << R"("UNKNOWN")";
                break;
        }
        os << R"(, "type": )"
           << "\"" << astNode.node->getNodeType().nodeName << "\""
           << R"(, "description": )"
           << "\"" << astNode.node->description.value_or("unknown") << "\""
           << R"(, "content": ")"
           << CHelper::TokenUtil::toString(astNode.tokens)
           << R"(", "errorReasons": )";
        if (astNode.isError()) {
            os << "[";
            bool isFirst = true;
            for (const auto &item: astNode.errorReasons) {
                if (item->errorReason.find("指令名字不匹配") != std::string::npos) {
                    continue;
                }
                if (isFirst) {
                    isFirst = false;
                } else {
                    os << ", ";
                }
                os << R"({"content": ")"
                   << CHelper::TokenUtil::toString(item->tokens)
                   << R"(", "reason": ")"
                   << item->errorReason
                   << R"("})";
            }
            os << "]";
        } else {
            os << "null";
        }
        os << R"(, "childNodes": )";
        if (astNode.hasChildNode()) {
            os << "[";
            bool isFirst = true;
            for (const CHelper::ASTNode &item: astNode.childNodes) {
                if (astNode.id == "command" && item.childNodes.size() < 2) {
                    continue;
                }
                if (isFirst) {
                    isFirst = false;
                } else {
                    os << ", ";
                }
                os << item;
            }
            os << "]";
        } else {
            os << "null";
        }
        return os << "}";
    }

} // CHelper