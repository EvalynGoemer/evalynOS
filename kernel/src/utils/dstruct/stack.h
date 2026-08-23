#pragma once

typedef struct stack_node stack_node_t;
typedef struct stack stack_t;

struct stack {
    stack_node_t* head;
};

struct stack_node {
    stack_node_t* next;
};

static inline void stack_push(stack_t* stack, stack_node_t* node) {
    node->next = stack->head;
    stack->head = node;
}

static inline stack_node_t* stack_pop(stack_t* stack) {
    if (!stack->head) return nullptr;
    stack_node_t* tmp = stack->head;
    stack->head = stack->head->next;
    return tmp;
}
