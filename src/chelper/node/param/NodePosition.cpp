//
// Created by Yancey on 2023/12/2.
//

#include "NodePosition.h"
#include "NodeRelativeFloat.h"

namespace CHelper::Node {

    NODE_TYPE("POSITION", NodePosition)

    NodePosition::NodePosition(const std::optional<std::string> &id,
                               const std::optional<std::string> &description)
            : NodeBase(id, description) {}

    NodePosition::NodePosition(const nlohmann::json &j,
                               const CPack &cpack)
            : NodeBase(j, cpack) {}

    ASTNode NodePosition::getASTNode(TokenReader &tokenReader, const CPack &cpack) const {
        tokenReader.push();
        // 0 - 绝对坐标，1 - 相对坐标，2 - 局部坐标
        std::vector<ASTNode> threeChildNodes;
        int types[3];
        for (int &type: types) {
            auto node = NodeRelativeFloat::getASTNode(this, tokenReader, cpack, true);
            type = node.first;
            threeChildNodes.push_back(node.second);
        }
        //判断有没有错误
        VectorView <Token> tokens = tokenReader.collect();
        ASTNode result = ASTNode::andNode(this, threeChildNodes, tokens, nullptr, "position");
        if (!result.isError()) {
            int type = 0;
            for (int i: types) {
                if (i == 0 || i == type) {
                    continue;
                } else if (type == 0) {
                    type = i;
                } else {
                    return ASTNode::andNode(this, threeChildNodes, tokenReader.collect(),
                                            ErrorReason::other(tokens, "坐标参数不能同时使用相对坐标和局部坐标"),
                                            "position");
                }
            }
        }
        return result;
    }

    void NodePosition::collectStructure(const ASTNode *astNode,
                                        StructureBuilder &structure,
                                        bool isMustHave) const {
        if (astNode->id == "position") {
            structure.append(isMustHave, description.value_or("位置"));
        }
    }

} // CHelper::Node