#define BH_DEBUG
#include "onyxsempass.h"
#include "onyxutils.h"

SemState semstate;

void onyx_sempass_init(bh_allocator alloc, bh_allocator node_alloc) {
    semstate = (SemState) {
        .allocator = alloc,
        .node_allocator = node_alloc,

        .global_scope = NULL,
        .curr_scope = NULL,

        .block_stack = NULL,

        .defer_allowed = 1,
    };

    bh_arr_new(global_heap_allocator, semstate.block_stack, 4);
}

void onyx_sempass(ProgramInfo* program) {
    semstate.program = program;

    onyx_resolve_symbols(program);
    if (onyx_message_has_errors()) return;

    onyx_type_check(program);
    if (onyx_message_has_errors()) return;
}
