//
// Created by Yancey on 2024/2/27.
//

#include "NodeHasPermissionArgument.h"
#include "../param/NodeNormalId.h"
#include "../param/NodeText.h"

namespace CHelper::Node {

    static std::shared_ptr<NodeBase> nodeKey = std::make_shared<NodeNormalId>(
            "PERMISSION", "权限", std::nullopt,
            std::make_shared<std::vector<std::shared_ptr<NormalId>>>(std::vector<std::shared_ptr<NormalId>>{
                    std::make_shared<NormalId>("camera", "玩家能否转动相机视角"),
                    std::make_shared<NormalId>("movement", "玩家能否移动")
            })
    );
    static std::shared_ptr<NodeBase> nodeEqual = std::make_shared<NodeText>(
            "TARGET_SELECTOR_ARGUMENT_EQUAL", "等于",
            std::make_shared<NormalId>("=", "等于"),
            [](const NodeBase *node, TokenReader &tokenReader) -> ASTNode {
                return tokenReader.readSymbolASTNode(node);
            });
    static std::shared_ptr<NodeBase> nodeValue = std::make_shared<NodeNormalId>(
            "PERMISSION_STATUS", "权限状态", std::nullopt,
            std::make_shared<std::vector<std::shared_ptr<NormalId>>>(std::vector<std::shared_ptr<NormalId>>{
                    std::make_shared<NormalId>("enabled", "启用"),
                    std::make_shared<NormalId>("disabled", "禁用")
            })
    );

    NodeHasPermissionArgument::NodeHasPermissionArgument(const std::optional<std::string> &id,
                                                         const std::optional<std::string> &description)
            : NodeBase(id, description, false) {}

    ASTNode NodeHasPermissionArgument::getASTNode(TokenReader &tokenReader) const {
        tokenReader.push();
        std::vector<ASTNode> childNodes;
        // key
        ASTNode astNodeKey = getByChildNode(tokenReader, nodeKey, "key");
        childNodes.push_back(astNodeKey);
        if (astNodeKey.isError()) {
            return ASTNode::andNode(this, childNodes, tokenReader.collect());
        }
        // = or !=
        ASTNode astNodeSeparator = getByChildNode(tokenReader, nodeEqual, "separator");
        childNodes.push_back(astNodeSeparator);
        if (astNodeSeparator.isError()) {
            return ASTNode::andNode(this, childNodes, tokenReader.collect());
        }
        //value
        childNodes.push_back(nodeValue->getASTNode(tokenReader));
        return ASTNode::andNode(this, childNodes, tokenReader.collect());
    }

} // CHelper::Node