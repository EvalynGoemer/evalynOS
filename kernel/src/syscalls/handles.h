#include <stdint.h>
#include <utils/dstruct/rbtree.h>
#include <mem/address_space.h>

typedef enum : uint8_t {
    HANDLE_NONE = 0,
    HANDLE_ADDRSPACE,
} handle_type_t;

#define GET_HANDLE_ID(idtype) ((idtype) >> 8)
#define GET_HANDLE_TYPE(idtype) ((idtype) & 0xFF)
#define SET_HANDLE_ID(idtype, id) ((idtype) = ((idtype) & 0xFF) | ((id) << 8))
#define SET_HANDLE_TYPE(idtype, type) ((idtype) = ((idtype) & ~0xFFull) | ((type) & 0xFF))

typedef struct {
    uint64_t idtype;
    rbtree_node_t node;
    union {
        address_space_t* addrspace;
    };
} handle_t;

static inline uint64_t handle_tree_von(rbtree_node_t* node) {
    handle_t* entry = CONTAINER_OF(node, handle_t, node);
    return GET_HANDLE_ID(entry->idtype);
}

extern handle_t* new_handle();
extern void free_handle(handle_t* handle);
extern handle_t* find_handle(uint64_t id);
