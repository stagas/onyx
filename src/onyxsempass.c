#define BH_DEBUG
#include "onyxsempass.h"
#include "onyxutils.h"

OnyxSemPassState onyx_sempass_create(bh_allocator alloc, bh_allocator node_alloc, OnyxMessages* msgs) {
    OnyxSemPassState state = {
        .allocator = alloc,
        .node_allocator = node_alloc,

        .msgs = msgs,

        .curr_local_group = NULL,
        .symbols = NULL,
    };

    bh_table_init(global_heap_allocator, state.symbols, 61);

    return state;
}


// NOTE: If the compiler is expanded to support more targets than just
// WASM, this function may not be needed. It brings all of the locals
// defined in sub-scopes up to the function-block level. This is a
// requirement of WASM, but not of other targets.
static void collapse_scopes(OnyxProgram* program) {
    bh_arr(AstNodeBlock*) traversal_queue = NULL;
    bh_arr_new(global_scratch_allocator, traversal_queue, 4);
    bh_arr_set_length(traversal_queue, 0);

    bh_arr_each(AstNodeFunction *, func, program->functions) {
        AstNodeLocalGroup* top_locals = (*func)->body->locals;

        bh_arr_push(traversal_queue, (*func)->body);
        while (!bh_arr_is_empty(traversal_queue)) {
            AstNodeBlock* block = traversal_queue[0];

            if (block->base.kind == AST_NODE_KIND_IF) {
                AstNodeIf* if_node = (AstNodeIf *) block;
                if (if_node->true_block.as_block != NULL)
                    bh_arr_push(traversal_queue, if_node->true_block.as_block);

                if (if_node->false_block.as_block != NULL)
                    bh_arr_push(traversal_queue, if_node->false_block.as_block);

            } else {

                if (block->locals != top_locals && block->locals->last_local != NULL) {
                    AstNodeLocal* last_local = block->locals->last_local;
                    while (last_local && last_local->prev_local != NULL) last_local = last_local->prev_local;

                    last_local->prev_local = top_locals->last_local;
                    top_locals->last_local = block->locals->last_local;
                    block->locals->last_local = NULL;
                }

                AstNode* walker = block->body;
                while (walker) {
                    if (walker->kind == AST_NODE_KIND_BLOCK) {
                        bh_arr_push(traversal_queue, (AstNodeBlock *) walker);

                    } else if (walker->kind == AST_NODE_KIND_WHILE) {
                        bh_arr_push(traversal_queue, ((AstNodeWhile *) walker)->body);

                    } else if (walker->kind == AST_NODE_KIND_IF) {
                        if (((AstNodeIf *) walker)->true_block.as_block != NULL)
                            bh_arr_push(traversal_queue, ((AstNodeIf *) walker)->true_block.as_block);

                        if (((AstNodeIf *) walker)->false_block.as_block != NULL)
                            bh_arr_push(traversal_queue, ((AstNodeIf *) walker)->false_block.as_block);
                    }

                    walker = walker->next;
                }
            }

            bh_arr_deleten(traversal_queue, 0, 1);
        }
    }
}

void onyx_sempass(OnyxSemPassState* state, OnyxProgram* program) {
    onyx_resolve_symbols(state, program);
    if (onyx_message_has_errors(state->msgs)) return;

    onyx_type_check(state, program);
    if (onyx_message_has_errors(state->msgs)) return;

    collapse_scopes(program);
}
