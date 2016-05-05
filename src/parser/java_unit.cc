#include <parser/java_unit.h>

Node::Node()
{
    this->tag = NodeEmpty;
    this->nodeData = nullptr;
}

Node::Node(NodeTag tag, void *nodeData)
{
    this->tag = tag;
    this->nodeData = nodeData;
}


Node Node::binary(NodeTag tag, Node l, Node r)
{
    BinaryNode *bin = new BinaryNode;
    bin->left = l;
    bin->right = r;
    return Node(tag, bin);
}


Node Node::unary(NodeTag tag, Node n)
{
    UnaryNode *un = new UnaryNode;
    un->next = n;
    return Node(tag, un);
}
