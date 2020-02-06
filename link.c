#include "stdint.h"

typedef struct _node
{
    uint32_t value;
    struct _node *next;
} Link_Node;

Link_Node *Link_Create_New_Node(uint32_t value)
{
    Link_Node *result;

    result = (Link_Node *)malloc(sizeof(Link_Node));
    result->value = value;
    return result;
}

Link_Node *Link_Init()
{
    Link_Node *result = 0;
    result = Link_Create_New_Node(0);
    result->value = (uint32_t)(result); // 最后一个节点的地址，保存在头节点的value中
    result->next = 0;
    return result;
}

void Link_Shift(uint32_t value, Link_Node *link)
{
    Link_Node *new_node;
    Link_Node *old_first_node;

    new_node = Link_Create_New_Node(value);

    old_first_node = link->next;
    link->next = new_node;
    new_node->next = old_first_node;
}

void Link_Append(uint32_t value, Link_Node *link)
{
    Link_Node *last_node = 0;
    last_node = (Link_Node *)link->value;

    Link_Node *new_node = Link_Create_New_Node(value);
    new_node->next = 0;
    last_node->next = new_node;

    link->value = (uint32_t)new_node;
}
