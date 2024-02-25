//
// Created by Yancey on 2023/11/10.
//

#include "NodePerCommand.h"
#include "NodeLF.h"
#include "../../resources/CPack.h"
#include "../../util/TokenUtil.h"

namespace CHelper::Node {

    NodePerCommand::NodePerCommand(const std::optional<std::string> &id,
                                   std::vector<std::string> name,
                                   const std::optional<std::string> &description,
                                   std::vector<std::shared_ptr<Node::NodeBase>> nodes,
                                   std::vector<std::shared_ptr<Node::NodeBase>> startNodes)
            : NodeBase(id, description, false),
              name(std::move(name)),
              nodes(std::move(nodes)),
              startNodes(std::move(startNodes)) {}

    NodePerCommand::NodePerCommand(const nlohmann::json &j,
                                   const CPack &cpack)
            : NodeBase(j) {
        Profile::push(ColorStringBuilder().red("loading node name").build());
        name = FROM_JSON(j, name, std::vector<std::string>);
        if (j.contains("node")) {
            Profile::next(ColorStringBuilder().red("loading nodes").build());
            for (const auto &item: j.at("node")) {
                nodes.push_back(getNodeFromJson(item, cpack));
            }
        }
        Profile::next(ColorStringBuilder().red("loading start nodes").build());
        auto startNodeIds = FROM_JSON(j, start, std::vector<std::string>);
        for (const auto &startNodeId: startNodeIds) {
            Profile::next(ColorStringBuilder()
                                  .red("linking startNode \"")
                                  .purple(startNodeId)
                                  .red("\" to nodes")
                                  .build());
            if (startNodeId == "LF") {
                startNodes.push_back(NodeLF::getInstance());
                continue;
            }
            for (auto &node: nodes) {
                if (node->id == startNodeId) {
                    startNodes.push_back(node);
                    break;
                }
            }
        }
        if (j.contains("ast")) {
            Profile::next(ColorStringBuilder().red("loading ast").build());
            auto ast = FROM_JSON(j, ast, std::vector<std::vector<std::string>>);
            for (const auto &childNodes: ast) {
                Profile::next(ColorStringBuilder().red("linking child nodes to parent node").build());
                if (childNodes.empty()) {
                    throw Exception::RequireParentNodeId(name);
                }
                auto parentNodeId = childNodes.at(0);
                Profile::next(ColorStringBuilder()
                                      .red("linking child nodes to parent node \"")
                                      .purple(parentNodeId)
                                      .red("\"")
                                      .build());
                if (childNodes.size() == 1) {
                    throw Exception::RequireChildNodeIds(name, parentNodeId);
                }
                std::shared_ptr<Node::NodeBase> parentNode = nullptr;
                for (auto &node: nodes) {
                    if (node->id == parentNodeId) {
                        parentNode = node;
                        break;
                    }
                }
                if (parentNode == nullptr) {
                    throw Exception::UnknownNodeId(name, parentNodeId);
                }
                for_each(childNodes.begin() + 1, childNodes.end(), [&](const auto &childNodeId) {
                    Profile::next(ColorStringBuilder()
                                          .red("linking child nodes \"")
                                          .purple(childNodeId)
                                          .red("\" to parent node \"")
                                          .purple(parentNodeId)
                                          .red("\"")
                                          .build());
                    if (childNodeId == "LF") {
                        parentNode->nextNodes.push_back(Node::NodeLF::getInstance());
                        return;
                    }
                    std::shared_ptr<Node::NodeBase> childNode = nullptr;
                    for (auto &node: nodes) {
                        if (node->id == childNodeId) {
                            childNode = node;
                            break;
                        }
                    }
                    if (childNode == nullptr) {
                        throw Exception::UnknownNodeId(name, childNodeId);
                    }
                    parentNode->nextNodes.push_back(childNode);
                });
            }
        }
        Profile::pop();
    }

    //因为节点可能之间互相绑定，所以要在析构的时候解除绑定
    NodePerCommand::~NodePerCommand() {
        for (const auto &item: nodes) {
            item->nextNodes.clear();
        }
    }

    NodeType NodePerCommand::getNodeType() const {
        return NodeType::PER_COMMAND;
    }

    void NodePerCommand::toJson(nlohmann::json &j) const {
        TO_JSON(j, name);
        TO_JSON_OPTIONAL(j, description)
        TO_JSON(j, nodes);
    }

    ASTNode NodePerCommand::getASTNode(TokenReader &tokenReader) const {
        tokenReader.push();
        //命令名字的检查
        ASTNode commandName = tokenReader.getStringASTNode(this, "commandName");
        if (commandName.tokens.size() == 0) {
            return ASTNode::andNode(this, {commandName}, commandName.tokens,
                                    ErrorReason::contentError(commandName.tokens, "指令名字为空"));
        }
        std::string str = TokenUtil::toString(commandName.tokens);
        bool isError = commandName.isError();
        if (!isError) {
            isError = true;
            for (const auto &item: name) {
                if (str == item) {
                    isError = false;
                    break;
                }
            }
        }
        if (isError) {
            tokenReader.pop();
            return ASTNode::andNode(this, {commandName}, commandName.tokens,
                                    ErrorReason::contentError(commandName.tokens, FormatUtil::format(
                                            "指令名字不匹配，找不到名为{0}的指令", str)),
                                    "perCommand");
        }
        //命令检测
        std::vector<ASTNode> childASTNodes;
        childASTNodes.reserve(startNodes.size());
        for (const auto &item: startNodes) {
            tokenReader.push();
            childASTNodes.push_back(item->getASTNodeWithNextNode(tokenReader));
            tokenReader.restore();
        }
        tokenReader.push();
        tokenReader.skipToLF();
        ASTNode astNodePerCommand = ASTNode::orNode(this, childASTNodes, tokenReader.collect());
        //返回结果
        return ASTNode::andNode(this, {commandName, astNodePerCommand}, tokenReader.collect(),
                                nullptr, "perCommand");
    }

    std::optional<std::string> NodePerCommand::collectDescription(const ASTNode *node, size_t index) const {
        if (node->id == "commandName") {
            return description;
        }
        return std::nullopt;
    }

    bool NodePerCommand::collectSuggestions(const ASTNode *astNode, size_t index,
                                            std::vector<Suggestion> &suggestions) const {
        if (astNode->id != "commandName") {
            return false;
        }
        std::string str = TokenUtil::toString(astNode->tokens)
                .substr(0, index - TokenUtil::getStartIndex(astNode->tokens));
        for (const auto &item: name) {
            if (StringUtil::isStartOf(item, str)) {
                suggestions.emplace_back(astNode->tokens, std::make_shared<NormalId>(item, description));
            }
        }
        return true;
    }

    void NodePerCommand::collectStructure(const ASTNode *astNode,
                                          StructureBuilder &structure,
                                          bool isMustHave) const {
        if (astNode == nullptr) {
            structure.appendWhiteSpace().append("命令");
        } else if (astNode->id == "perCommand") {
            if (astNode->tokens.size() <= 1 || astNode->childNodes[0].isError()) {
                structure.appendWhiteSpace().append("命令");
            }
        } else if (astNode->id == "commandName") {
            if (astNode->isError()) {
                structure.appendWhiteSpace().append("命令");
            } else {
                structure.appendWhiteSpace().append(TokenUtil::toString(astNode->tokens));
            }
        }
    }

} // CHelper::Node