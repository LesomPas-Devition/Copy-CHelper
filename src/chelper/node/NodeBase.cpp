//
// Created by Yancey on 2023/11/10.
//

#include "NodeBase.h"
#include "param/NodeLF.h"

namespace CHelper::Node {

    NodeBase::NodeBase(const std::optional<std::string> &id,
                       const std::optional<std::string> &description,
                       bool isMustAfterWhiteSpace)
            : id(id),
              description(description),
              isMustAfterWhiteSpace(isMustAfterWhiteSpace) {}

    NodeBase::NodeBase(const nlohmann::json &j)
            : id(FROM_JSON_OPTIONAL(j, id, std::string)),
              description(FROM_JSON_OPTIONAL(j, description, std::string)),
              isMustAfterWhiteSpace(true) {
    }

    std::shared_ptr<NodeBase> NodeBase::getNodeFromJson(const nlohmann::json &j,
                                                        const CPack &cpack) {
        Profile::push(ColorStringBuilder().red("loading type").build());
        std::string type = FROM_JSON(j, type, std::string);
        Profile::next(ColorStringBuilder().red("loading node ").purple(type).build());
        for (const auto &item: NodeType::NODE_TYPES) {
            if (item->nodeName == type) {
                Profile::pop();
                return item->createNodeByJson(j, cpack);
            }
        }
        throw Exception::UnknownNodeType(type);
    }

    NodeType NodeBase::getNodeType() const {
        return NodeType::UNKNOWN;
    }

    void NodeBase::toJson(nlohmann::json &j) const {
        TO_JSON_OPTIONAL(j, id)
        TO_JSON_OPTIONAL(j, description)
        j.push_back({"type", getNodeType().nodeName});
    }

    ASTNode NodeBase::getASTNodeWithNextNode(TokenReader &tokenReader) const {
        //TODO 把所有定位的getASTNode()改为getASTNodeWithNextNode()
        //空格检测
        tokenReader.push();
        if (isMustAfterWhiteSpace && tokenReader.skipWhitespace() == 0) {
            VectorView <Token> tokens = tokenReader.collect();
            return ASTNode::simpleNode(this, tokens, ErrorReason::requireWhiteSpace(tokens));
        }
        tokenReader.pop();
        tokenReader.push();
        //当前节点
        DEBUG_GET_NODE_BEGIN(this)
        ASTNode currentASTNode = getASTNode(tokenReader);
        DEBUG_GET_NODE_END(this)
        if (currentASTNode.isError() || nextNodes.empty()) {
            tokenReader.pop();
            return ASTNode::andNode(this, {currentASTNode}, currentASTNode.tokens, nullptr, "compound");
        }
        //子节点
        std::vector<ASTNode> childASTNodes;
        childASTNodes.reserve(nextNodes.size());
        for (const auto &item: nextNodes) {
            tokenReader.push();
            childASTNodes.push_back(item->getASTNodeWithNextNode(tokenReader));
            tokenReader.restore();
        }
        tokenReader.push();
        tokenReader.skipToLF();
        ASTNode nextASTNode = ASTNode::orNode(this, childASTNodes, tokenReader.collect(), nullptr, "nextNode");
        return ASTNode::andNode(this, {currentASTNode, nextASTNode}, tokenReader.collect(), nullptr, "compound");
    }

    ASTNode NodeBase::getByChildNode(TokenReader &tokenReader,
                                     const std::shared_ptr<NodeBase> &childNode,
                                     const std::string &astNodeId) const {
        ASTNode node = childNode->getASTNode(tokenReader);
        return ASTNode::andNode(this, {node}, node.tokens, nullptr, astNodeId);
    }

    /**
     * 第一个节点一定要有，其他节点不一定有
     *
     * @param isIgnoreChildNodesError true - 第一个错误节点到后面都不算做子节点
     *                                false - 第一个内容为空的错误节点到后面都不算做子节点
     */
    ASTNode NodeBase::getOptionalASTNode(TokenReader &tokenReader,
                                         bool isIgnoreChildNodesError,
                                         const std::vector<std::shared_ptr<NodeBase>> &childNodes,
                                         const std::string &astNodeId) const {
        tokenReader.push();
        std::vector<ASTNode> childASTNodes;
        for (const auto &item: childNodes) {
            tokenReader.push();
            tokenReader.push();
            ASTNode astNode = item->getASTNode(tokenReader);
            const VectorView <Token> tokens = tokenReader.collect();
            if (astNode.isError() && !childASTNodes.empty() && (isIgnoreChildNodesError || tokens.isEmpty())) {
                tokenReader.restore();
                break;
            }
            childASTNodes.push_back(astNode);
            tokenReader.pop();
            if (astNode.isError()) {
                break;
            }
        }
        return ASTNode::andNode(this, childASTNodes, tokenReader.collect(), nullptr, astNodeId);
    }

    std::optional<std::string> NodeBase::collectDescription(const ASTNode *node, size_t index) const {
        return description;
    }

    //创建AST节点的时候只得到了结构的错误，ID的错误需要调用这个方法得到
    bool NodeBase::collectIdError(const ASTNode *astNode,
                                  std::vector<std::shared_ptr<ErrorReason>> &idErrorReasons) const {
        return false;
    }

    bool NodeBase::collectSuggestions(const ASTNode *astNode,
                                      size_t index,
                                      std::vector<Suggestion> &suggestions) const {
        return false;
    }

    void NodeBase::collectStructure(const ASTNode *astNode,
                                    StructureBuilder &structure,
                                    bool isMustHave) const {

    }

    void NodeBase::collectStructureNormal(StructureBuilder &structure, bool isMustHave) const {
        collectStructure(nullptr, structure, isMustHave);
        if (nextNodes.empty()) {
            return;
        }
        for (const auto &item: nextNodes) {
            if (item == NodeLF::getInstance()) {
                isMustHave = false;
            }
        }
        nextNodes[0]->collectStructureNormal(structure, isMustHave);
    }

} // CHelper::Node
