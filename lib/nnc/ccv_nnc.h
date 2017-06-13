/**********************************************************
 * C-based/Cached/Core Computer Vision Library
 * Liu Liu, 2010-02-01
 **********************************************************/

/**********************************************************
 * CCV - Neural Network Collection
 **********************************************************/

#ifndef GUARD_ccv_nnc_h
#define GUARD_ccv_nnc_h

#include <ccv.h>

// These are generated by cmd/build-cmd.rb
#include "cmd/ccv_nnc_cmd.h"
#include "cmd/ccv_nnc_backend.h"

enum {
	// Attributes that enable tensor allocation optimization
	CCV_NNC_CMD_ATTR_INPLACE      = 0x01, // Is it a inplace operation? (Thus, the input tensor can be the same as the output tensor). This is actually a stronger assumption than it seems. It says that the input tensors can be the same as any of the output tensors. Thus, input tensors of [a, b] and output tensors of [b, a] or [a, a] or [b, b] are perfectly supported if your compute node supports this flag.
	// Attributes that enable symbolic graph simplification
	CCV_NNC_CMD_ATTR_PASSTHROUGH  = 0x02, // This doesn't compute anything, but pass the first n tensors to the output (useful for backprop that is identical).
	CCV_NNC_CMD_ATTR_OUTPUT_ONES  = 0x04, // All the output tensors are 1s (unit).
	CCV_NNC_CMD_ATTR_NULL_IS_ONES = 0x08, // Accept nullptr input as if these are tensors with 1s (unit).
};

enum {
	CCV_NNC_ACCUMULATE_OUTPUT = 0x01, // Enable accumulate outputs.
	CCV_NNC_ZERO_MEMORY_ALLOC = 0x02, // Don't allocate any extra memory for this operation.
};

enum {
	CCV_NNC_EXEC_SUCCESS   = 0,
	CCV_NNC_EXEC_INVALID   = -1, // Invalid input.
	CCV_NNC_EXEC_NO_KERNEL = -2,
	CCV_NNC_EXEC_OOM       = -3,
};

typedef struct {
	struct {
		int dim[CCV_NNC_MAX_DIM_ALLOC];
	} size; /**< [size] The window size for the layer. For full connect layer, it is 1 because it is 1x1 convolutional layer with count of filters */
	union {
		struct {
			int count; /**< [convolution.count] The number of filters for convolutional layer. */
		} convolution;
		struct {
			int reserved;
		} pool;
		struct {
			float kappa; /**< [rnorm.kappa] As of b[i] = a[i] / (rnorm.kappa + rnorm.alpha * sum(a, i - rnorm.size / 2, i + rnorm.size / 2)) ^ rnorm.beta */
			float alpha; /**< [rnorm.alpha] See **rnorm.kappa**. */
			float beta; /**< [rnorm.beta] See **rnorm.kappa**. */
		} rnorm;
		struct {
			float a[3]; /**< BLAS scalars. */
			int count; /**< [blas.count] The number of outputs for blas layer. */
		} blas;
		void* userdata;
	};
} ccv_nnc_cmd_param_t;

typedef struct {
	struct {
		int dim[CCV_NNC_MAX_DIM_ALLOC];
	} stride;
	struct {
		int begin[CCV_NNC_MAX_DIM_ALLOC];
		int end[CCV_NNC_MAX_DIM_ALLOC];
	} border;
} ccv_nnc_hint_t;

typedef struct ccv_nnc_stream_context_s ccv_nnc_stream_context_t;

typedef struct ccv_nnc_cmd_s {
	uint32_t cmd;
	uint32_t backend;
	int algorithm;
	ccv_nnc_cmd_param_t info;
	// This has to be the same as the ccv_nnc_cmd_exec_f type.
	// This is for type CCV_NNC_COMPUTE_CUSTOM
	int(*exec)(const struct ccv_nnc_cmd_s cmd, const ccv_nnc_hint_t hint, const int flags, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const ccv_nnc_stream_context_t* stream_context);
} ccv_nnc_cmd_t;

// For forward functions, the input tensors and output tensors can be arbitrary.
// However, for backward functions (backpropagation, or gradient functions in other libs),
// the input is: 0~m-1: gradient for output tensors, 1~n: input tensors for forward functions, n+1~n+m: output tensors for forward functions,
// the output is: 0~n-1: output gradients w.r.t. input tensors.
// Which input / output tensors can be ignored can be specified in the cmd config structs.
typedef int(*ccv_nnc_cmd_exec_f)(const ccv_nnc_cmd_t cmd, const ccv_nnc_hint_t hint, const int flags, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const ccv_nnc_stream_context_t* stream_context);

typedef int(*ccv_nnc_cmd_autotune_f)(const ccv_nnc_cmd_t cmd, const size_t max_workspace_size, const ccv_nnc_hint_t hint, const int flags, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const ccv_nnc_stream_context_t* stream_context);

static inline int ccv_nnc_tensor_nd(const int dim[CCV_NNC_MAX_DIM_ALLOC])
{
	int i;
	for (i = 0; i < CCV_NNC_MAX_DIM_ALLOC; i++)
		if (dim[i] == 0)
			return i;
	return CCV_NNC_MAX_DIM_ALLOC;
}

/**
 * Level-0 API
 */

void ccv_nnc_init(void);

/**
 * Level-1 API
 */

// For tensor
CCV_WARN_UNUSED(ccv_nnc_tensor_t*) ccv_nnc_tensor_new(const void* const ptr, const ccv_nnc_tensor_param_t params, const int flags);
// Allocating on stack
CCV_WARN_UNUSED(ccv_nnc_tensor_t) ccv_nnc_tensor(const void* const ptr, const ccv_nnc_tensor_param_t params, const int flags);
void ccv_nnc_tensor_free(ccv_nnc_tensor_t* const tensor);
CCV_WARN_UNUSED(ccv_nnc_tensor_view_t*) ccv_nnc_tensor_view_new(const ccv_nnc_tensor_t* const tensor, const int ofs[CCV_NNC_MAX_DIM_ALLOC], const int dim[CCV_NNC_MAX_DIM_ALLOC]);
// Allocating on stack
CCV_WARN_UNUSED(ccv_nnc_tensor_view_t) ccv_nnc_tensor_view(const ccv_nnc_tensor_t* const tensor, const int ofs[CCV_NNC_MAX_DIM_ALLOC], const int dim[CCV_NNC_MAX_DIM_ALLOC]);
void ccv_nnc_tensor_view_free(ccv_nnc_tensor_view_t* const tensor_view);
// All these functions afterwards should be compatible with both tensor and tensor view unless assertion.
void ccv_nnc_tensor_zero(void* const tensor);
int ccv_nnc_tensor_eq(const ccv_nnc_tensor_t* const a, const ccv_nnc_tensor_t* const b);

// For computation node
// Return high precision time unit.
uint64_t ccv_nnc_cmd_mono_time(void);
CCV_WARN_UNUSED(const char*) ccv_nnc_cmd_name(const uint32_t cmd);
CCV_WARN_UNUSED(const char*) ccv_nnc_cmd_backend_name(const uint32_t backend);
CCV_WARN_UNUSED(int) ccv_nnc_cmd_ok(const uint32_t cmd, const uint32_t backend);
CCV_WARN_UNUSED(ccv_nnc_cmd_t) ccv_nnc_cmd(const uint32_t cmd, ccv_nnc_cmd_exec_f exec, const ccv_nnc_cmd_param_t params, const int flags);
// Verify the hint
CCV_WARN_UNUSED(int) ccv_nnc_hint_verify(const ccv_nnc_hint_t hint, const ccv_nnc_cmd_param_t cmd, const ccv_nnc_tensor_param_t a, const ccv_nnc_tensor_param_t b);
// Auto find the best hint for a given input / output (on forward pass only).
CCV_WARN_UNUSED(ccv_nnc_hint_t) ccv_nnc_hint_auto(const ccv_nnc_cmd_param_t cmd, const ccv_nnc_tensor_param_t a, const ccv_nnc_tensor_param_t b);
// Auto find the outputs for the given inputs / hint.
void ccv_nnc_hint_tensor_auto(const ccv_nnc_cmd_t cmd, const ccv_nnc_tensor_param_t* const inputs, const int input_size, const ccv_nnc_hint_t hint, ccv_nnc_tensor_param_t* const outputs, const int output_size);
// Run autotune to find the best kernel and configuration for the given input, returned is the modified
// cmd that contains the updated configuration.
CCV_WARN_UNUSED(ccv_nnc_cmd_t) ccv_nnc_cmd_autotune(const ccv_nnc_cmd_t cmd, const size_t max_workspace_size, const ccv_nnc_hint_t hint, const int flags, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const ccv_nnc_stream_context_t* const stream_context);
CCV_WARN_UNUSED(int) ccv_nnc_cmd_bitmask(const ccv_nnc_cmd_t cmd, const uint64_t* const input_bitmasks, const int input_bitmask_size, const uint64_t* const output_bitmasks, const int output_bitmask_size);
int ccv_nnc_cmd_exec(const ccv_nnc_cmd_t cmd, const ccv_nnc_hint_t hint, const int flags, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const ccv_nnc_stream_context_t* const stream_context);
CCV_WARN_UNUSED(int) ccv_nnc_cmd_attr(const ccv_nnc_cmd_t cmd, const int flags);
CCV_WARN_UNUSED(int) ccv_nnc_cmd_is_forward(const ccv_nnc_cmd_t cmd);
CCV_WARN_UNUSED(int) ccv_nnc_cmd_is_backward(const ccv_nnc_cmd_t cmd);

// Control flow constructs
// Follow heavily based along CUDA's stream / event idea.
enum {
	CCV_STREAM_CONTEXT_CPU = 0x1,
	CCV_STREAM_CONTEXT_GPU = 0x2,
};
#define CCV_STREAM_GET_CONTEXT(type) ((type) & 0x3)
#define CCV_STREAM_GET_DEVICE(type) ((type) & 0xff00)
#define CCV_STREAM_GET_DEVICE_ID(type) (CCV_STREAM_GET_DEVICE(type) >> 8)
// Flag is a combination of CPU / GPU and DEVICE_ID
CCV_WARN_UNUSED(ccv_nnc_stream_context_t*) ccv_nnc_stream_context_new(const int type);
void ccv_nnc_stream_context_wait(const ccv_nnc_stream_context_t* const stream);
void ccv_nnc_stream_context_free(ccv_nnc_stream_context_t* const stream_context);

typedef struct ccv_nnc_stream_signal_s ccv_nnc_stream_signal_t;

CCV_WARN_UNUSED(ccv_nnc_stream_signal_t*) ccv_nnc_stream_signal_new(const int type);
void ccv_nnc_stream_context_emit_signal(const ccv_nnc_stream_context_t* const stream, const ccv_nnc_stream_signal_t* const signal);
void ccv_nnc_stream_context_wait_signal(const ccv_nnc_stream_context_t* const stream, const ccv_nnc_stream_signal_t* const signal);
void ccv_nnc_stream_signal_free(ccv_nnc_stream_signal_t* const signal);

/**
 * Level-2 API
 */

enum {
	CCV_NNC_SHORT_DOT_GRAPH = 0x0,
	CCV_NNC_LONG_DOT_GRAPH  = 0x1,
};

typedef struct ccv_nnc_graph_s ccv_nnc_graph_t;

typedef struct {
	int32_t d; // This is int because sometimes I piggy-back on negatives to carry out some internal computations.
	const ccv_nnc_graph_t* graph;
} ccv_nnc_graph_exec_t;

#define CCV_NO_GRAPH_EXEC(exec) ((exec).graph == 0)

// Create an empty graph.
// Note that all graph mutation methods are not thread-safe.
// You should only operate the graph in serial fashion.
CCV_WARN_UNUSED(ccv_nnc_graph_t*) ccv_nnc_graph_new(void);
// Create a node with specific command execution, as well as its inputs & outputs.
// Underlying, the graph maintains the backing object for the node, and all you get is
// a on-stack object to index the backing object from the graph.
CCV_WARN_UNUSED(ccv_nnc_graph_exec_t) ccv_nnc_graph_exec_new(ccv_nnc_graph_t* const graph, const ccv_nnc_cmd_t cmd, const ccv_nnc_hint_t hint, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size);
void ccv_nnc_graph_exec_set_hint(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t exec, const ccv_nnc_hint_t hint);
void ccv_nnc_graph_exec_set_io(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t exec, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size);
// Concatenate input graph nodes with an output graph node to create a new graph.
// Return non-zero if cannot concat successfully.
int ccv_nnc_graph_exec_concat(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t source, const ccv_nnc_graph_exec_t destination);
// Disconnect input graph nodes with an output graph nodes in this graph.
// Return non-zero if cannot disjoin successfully.
int ccv_nnc_graph_exec_disjoin(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t source, const ccv_nnc_graph_exec_t destination);
// Generate output that can be parsed by GraphViz (DOT language).
void ccv_nnc_graph_dot(const ccv_nnc_graph_t* const graph, const int flags, FILE* out);
// Run the autotune function for all the inputs / outputs, afterwards, assigning the optimized cmd back.
void ccv_nnc_graph_autotune(ccv_nnc_graph_t* const graph, const size_t max_workspace_size, const int flags, const ccv_nnc_graph_exec_t* const sources, const int source_size, const ccv_nnc_graph_exec_t* const destinations, const int destination_size);
// Run the graph from source nodes all the way to the destination nodes.
void ccv_nnc_graph_run(const ccv_nnc_graph_t* const graph, const int flags, const ccv_nnc_graph_exec_t* const sources, const int source_size, const ccv_nnc_graph_exec_t* const destinations, const int destination_size);
// The sources / destinations.
void ccv_nnc_graph_set_sources(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t* const sources, const int source_size);
ccv_nnc_graph_exec_t* ccv_nnc_graph_sources(const ccv_nnc_graph_t* const graph);
int ccv_nnc_graph_source_size(const ccv_nnc_graph_t* const graph);
void ccv_nnc_graph_set_destinations(ccv_nnc_graph_t* const graph, const ccv_nnc_graph_exec_t* const destinations, const int destination_size);
ccv_nnc_graph_exec_t* ccv_nnc_graph_destinations(const ccv_nnc_graph_t* const graph);
int ccv_nnc_graph_destination_size(const ccv_nnc_graph_t* const graph);
// This graph, and its relevant auxiliary objects (opaque to user) are deallocated.
void ccv_nnc_graph_free(ccv_nnc_graph_t* const graph);

/**
 * Level-3 API
 */

typedef struct ccv_nnc_symbolic_graph_s ccv_nnc_symbolic_graph_t;

// Opaque pointer to an arena of allocated tensors.
typedef struct ccv_nnc_tensor_arena_s ccv_nnc_tensor_arena_t;

// Opaque pointer to an arena of allocated execs.
typedef struct ccv_nnc_graph_exec_arena_s ccv_nnc_graph_exec_arena_t;

typedef struct {
	ccv_nnc_tensor_param_t info;
	int32_t d;
	const ccv_nnc_symbolic_graph_t* graph;
} ccv_nnc_tensor_symbol_t;

typedef struct {
	int32_t d;
	const ccv_nnc_symbolic_graph_t* graph;
} ccv_nnc_graph_exec_symbol_t;

enum {
	CCV_NNC_SYM_TENSOR_INIT_ZEROS = 0x01, // Initialize underlying tensor for the symbol with zeros
};

// Create an empty symbolic graph.
// Note that all graph mutation methods are not thread-safe.
// You should only operate the graph in serial fashion.

// Create a new symbolic graph. It is an opaque data structure that maintains the whole graph of computation in its symbolic form.
CCV_WARN_UNUSED(ccv_nnc_symbolic_graph_t*) ccv_nnc_symbolic_graph_new(void);
// Create an tensor symbol (thus, with no actual memory space allocation) in a symbolic graph.
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_tensor_symbol_new(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_param_t info, const char* const name);
// Create an alias to the tensor symbol as tensor view (thus, pointing to the same memory region, but with a different header info and offset).
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_tensor_symbol_alias_new(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t tensor_symbol, const int ofs[CCV_NNC_MAX_DIM_ALLOC], const int inc[CCV_NNC_MAX_DIM_ALLOC], const ccv_nnc_tensor_param_t info, const char* const name);
// For a given alias, this method resolve to referenced tensor symbol.
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_tensor_symbol_resolve_alias(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t tensor_alias);
// Create a graph node (an operation that takes a set of inputs and generates a set of outputs).
ccv_nnc_graph_exec_symbol_t ccv_nnc_graph_exec_symbol_new(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_cmd_t cmd, const ccv_nnc_tensor_symbol_t* const inputs, const int input_size, const ccv_nnc_tensor_symbol_t* const outputs, const int output_size, const char* const name);
// Return the command on this exec symbol
CCV_WARN_UNUSED(ccv_nnc_cmd_t) ccv_nnc_graph_exec_symbol_cmd(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t exec);
// The operation defaults to use `ccv_nnc_hint_auto` find the best hints for a set of inputs / outputs.
// However, you can also set your own hints. Return non-zero if cannot set successfully.
int ccv_nnc_graph_exec_symbol_set_hint(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t exec, const ccv_nnc_hint_t hint);
// Set the tensor symbol info again. Thus, its dimensionality depends on the tensor input.
int ccv_nnc_tensor_symbol_set(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t tensor, const ccv_nnc_tensor_param_t info);
// Set the flags for this tensor symbol. The flags are only used for symbol, not for tensor.
int ccv_nnc_tensor_symbol_set_flags(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t tensor, const int flags);
CCV_WARN_UNUSED(int) ccv_nnc_tensor_symbol_flag(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t tensor, const int flags);
// Manually concatenate input graph nodes with an output graph node to create a new graph.
// Return non-zero if cannot concat successfully.
int ccv_nnc_graph_exec_symbol_concat(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t source, const ccv_nnc_graph_exec_symbol_t destination);
// Manually disconnect input graph nodes with an output graph node for this graph.
// Return non-zero if cannot disjoin successfully.
int ccv_nnc_graph_exec_symbol_disjoin(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t source, const ccv_nnc_graph_exec_symbol_t destination);
// Automatic concatenate these nodes together based on its inputs / outputs.
// Return non-zero if cannot figure out.
// Imagining this is to generate the execution flow based on input tensors and output tensors.
// nil for execs and 0 for exec_size means to loop over all the execs on the graph and autogen.
int ccv_nnc_graph_exec_symbol_autogen(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t* const execs, const int exec_size);
// Generate a duplicate of the provided graph.
// While generating the duplicate, it calls the function pointer to re-process the node type.
typedef ccv_nnc_cmd_t(*ccv_nnc_symbolic_graph_subst_f)(const ccv_nnc_graph_exec_symbol_t symbol, const ccv_nnc_cmd_t cmd);
CCV_WARN_UNUSED(ccv_nnc_symbolic_graph_t*) ccv_nnc_symbolic_graph_dup(const ccv_nnc_symbolic_graph_t* const graph, ccv_nnc_symbolic_graph_subst_f subst);
// The source / destination generated by the autogen.
void ccv_nnc_symbolic_graph_set_sources(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t* const sources, const int source_size);
ccv_nnc_graph_exec_symbol_t* ccv_nnc_symbolic_graph_sources(const ccv_nnc_symbolic_graph_t* const graph);
int ccv_nnc_symbolic_graph_source_size(const ccv_nnc_symbolic_graph_t* const graph);
void ccv_nnc_symbolic_graph_set_destinations(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t* const destinations, const int destination_size);
ccv_nnc_graph_exec_symbol_t* ccv_nnc_symbolic_graph_destinations(const ccv_nnc_symbolic_graph_t* const graph);
int ccv_nnc_symbolic_graph_destination_size(const ccv_nnc_symbolic_graph_t* const graph);
// Generate output that can be parsed by GraphViz (DOT language).
void ccv_nnc_symbolic_graph_dot(const ccv_nnc_symbolic_graph_t* const graph, const int flags, FILE* out);

typedef struct {
	ccv_nnc_tensor_symbol_t symbol;
	const ccv_nnc_tensor_t* tensor;
} ccv_nnc_tensor_bind_t;

// Compile a symbolic graph into a graph that can be executed, and a set of tensors (opaque data structure tensor arena) are allocated based on which tensor symbols are the input and which are the outputs. The tensor allocation is done to minimize the required storage.
// tensor_binds provide custom binding for these tensors. You still responsible to manage the life-time of these tensors.
void ccv_nnc_symbolic_graph_compile(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_bind_t* const tensor_binds, const int tensor_binds_size, const ccv_nnc_graph_exec_symbol_t* const sources, const int source_size, const ccv_nnc_graph_exec_symbol_t* const destinations, const int destination_size, ccv_nnc_graph_t** const graph_ref, ccv_nnc_tensor_arena_t** const tensor_arena_ref, ccv_nnc_graph_exec_arena_t** const graph_exec_arena_ref);
// Free the symbolic graph and its associated memory. Note that if you compiled a graph / tensor arena out of this symbolic graph, these won't be free'd.
void ccv_nnc_symbolic_graph_free(ccv_nnc_symbolic_graph_t* const graph);
// Find corresponding tensor by a symbol from the tensor arena.
CCV_WARN_UNUSED(ccv_nnc_tensor_t*) ccv_nnc_tensor_from_symbol(const ccv_nnc_tensor_arena_t* const tensor_arena, const ccv_nnc_tensor_symbol_t symbol);
// Bind a tensor to a symbol. You still responsible to manage the life-time of the tensor to make sure it is not freed until everything is done.
int ccv_nnc_tensor_bind_symbol(const ccv_nnc_tensor_arena_t* const tensor_arena, const ccv_nnc_tensor_symbol_t symbol, const ccv_nnc_tensor_t* const tensor);
// Free the opaque tensor arena structure.
void ccv_nnc_tensor_arena_free(ccv_nnc_tensor_arena_t* const tensor_arena);
// Find corresponding graph exec by a exec symbol from graph exec arena.
CCV_WARN_UNUSED(ccv_nnc_graph_exec_t) ccv_nnc_graph_exec_from_symbol(const ccv_nnc_graph_exec_arena_t* const graph_exec_arena, const ccv_nnc_graph_exec_symbol_t symbol);
// Return the node that can drive all the source nodes from the compilation.
CCV_WARN_UNUSED(ccv_nnc_graph_exec_t) ccv_nnc_graph_exec_source(const ccv_nnc_graph_exec_arena_t* const graph_exec_arena);
// Return the node that can drain all the destination nodes from the compilation.
CCV_WARN_UNUSED(ccv_nnc_graph_exec_t) ccv_nnc_graph_exec_destination(const ccv_nnc_graph_exec_arena_t* const graph_exec_arena);
// Free the opaque graph exec arena structure.
void ccv_nnc_graph_exec_arena_free(ccv_nnc_graph_exec_arena_t* const graph_exec_arena);

/**
 * Level-4 API
 */
// Compute the backward graph, assuming the provided symbolic graph only contain the "forward" part from sources to destinations.
// This effectively is called the "autograd" or automatic differentiation process (specifically, "reverse AD") in other libs.
void ccv_nnc_symbolic_graph_backward(ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t* const sources, const int source_size, const ccv_nnc_graph_exec_symbol_t* const destinations, const int destination_size, const ccv_nnc_tensor_symbol_t* const f_symbols, const int f_symbol_size, const ccv_nnc_tensor_symbol_t* const wrt_symbols, const int wrt_symbol_size);
// Get the symbol that contains the gradient. The list will be flushed if the ccv_nnc_symbolic_graph_backward function is called again.
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_tensor_symbol_for_backward(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t symbol);
// This has to get the exec symbol from the tensor.
CCV_WARN_UNUSED(ccv_nnc_graph_exec_symbol_t) ccv_nnc_graph_exec_symbol_for_backward(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t symbol);

// Construct a "while" loop in a symbolic graph.
//
// In NNC, a computation graph cannot allow cycles. Thus, there is no flexible way to express loops.
//
// A little survey on this problem:
//
// Caffe2 supports specific type of recurrent neural network.
// TensorFlow as it stands, supports while construct. Its while construct is very straightforward, a body and a condition is provided, you can construct whatever graph as you want.
// MxNet supports recurrent neural network by unrolling it into normal none-looped graph.
// Theano supports "scan" ops, which is a terminable loop (with loop variant, known as sequence).
// CNTK supports this with custom BrainScript. Within BrainScript, you can access the previous state in a function, therefore, effectively supports calling a method multiple times (looping over).
//
// Of above, Caffe2 and MxNet gave up on supporting generic loop for performance reasons.
// TensorFlow supports generic while loop, with all the trouble it may introduce (see the Nested while loop bug in TensorFlow that recently fixed).
// Theano picked a point seems pretty sweet, although there are limitations.
// CNTK's BrainScript is a DSL, they can do whatever they want with the drawback now that they need to implement a language runtime.
// TensorFlow, Theano and CNTK all support auto-differentiation over the while loop with tape (Wengert list).
//
// A simple way to support loop is to support conditional jump. In fact, conditional jump is a more generic way of doing loops. However,
// if you put this into the consideration that fully differentiable computation graph wanna to be supported, it is terrible. With conditional
// jump, it is really hard for you to know which tensor is used where, thus keep track for reverse accumulation (backward propagation). There
// is no counter or whatsoever, it is pretty hard to trace back on which line is executed how many times. Compounding this with NNC's promise
// that as long as it shows on the graph can be "parallel" computed, it will be parallel computed, it is close to impossible to track if
// conditional jump used in its raw form. Certain restrictions must be applied to how to do the loop. The compromise comes from closer
// examination of NNC's preferences.
//
// NNC prefers to have the graph without cycles. It also prefers to be fully differentiable. Another important criteria is that most
// functions in NNC require SSA (Static Single Assignment) representation. With these in mind, supporting while loop has to be strict.
//
// Luckily, there are well-formalized way of supporting this in literature and practice. Because it is well-formalized, translating this
// into existing NNC implementation is actually pretty straightforward. We are going to introduce a special version of while loop. In
// literature that discussed about SSA, it may be called parameterized loop. For us, it works like this:
//
// To construct a while loop for existing NNC graph, you need to be able to separate the existing graph into two sub-graphs.
//
// The while-loop sub-graph (WL sub-graph) contains a set of incoming nodes (I-nodes), Condition false output nodes (CFO-nodes) and end nodes (E-nodes).
// Each set have its own properties, but in short, all incoming edges to the WL sub-graph connect to one of the I-nodes, but nothing else. All outgoing
// edges from the WL sub-graph connect to one of the CFO-nodes, but nothing else. A nodes can be either a I-node, CFO-node or E-node, non-exclusively.
// There are also 3 types of tensors used for all nodes in WL sub-graph: Input tensors (I-tensors) are tensors that are inputs to some nodes, and will
// never be outputs. Output tensors (O-tensors) are tensors that are outputs from some nodes, but never be inputs to any nodes. I-tensors can be outputs
// from some nodes that outside of WL sub-graph. O-tensors can be inputs to some nodes that outside of WL sub-graph. Internal tensors (IN-tensors) are
// not visible outside of WL sub-graph, therefore, they can be both inputs and outputs of some nodes inside the sub-graph. Some tensors can be feedback
// into the WL sub-graph, given either O-tensors or IN-tensors. A parameter map can be given in these cases to describe which maps to what.
//
// The way to drive a WL sub-graph like this: the WL sub-graph runs until all CFO-nodes are reached. At this point, the while_f condition is checked.
// If true, we continue until all the end-nodes are reached. At this point, we increase the counter, reconfigure the WL sub-graph with parameter map,
// and run from I-nodes all over again. When reached all CFO-nodes, the condition is checked again, if false, WL sub-graph terminates, and the graph
// continues from the nodes that are pointed by CFO-nodes.
//
// Given these constraints, doing automatic differentiation is not that hard any more. A WL sub-graph, from the whole graph's point of view, is just
// a giant command supports both forward / backward operations, with some extra information passed around in the form of userdata (tape).
//
// For WL sub-graph, we can continue to leverage the compile / backward function that already written for symbolic graph as well.
// For compile function, we just need to take care of parameter maps (these need to be converted into binded tensors).
// For backward function, we need to convert parameter maps from assigner (thus, y = x) to accumulator (x += y).
//
// This function will replace the nodes that it affects to one sub-graph node.
// Thus, how to drive this sub-graph is opaque. Its backward form is opaque as well.
//
// There are no connection between its nodes and the outside graph nodes other than the
// three sets:
// 1). Incoming nodes, the set of nodes that contains the incoming edges from outside, they cannot have edges points by inside nodes. The sub-graph computation starts from these incoming nodes;
// 2). Condition false output nodes, when condition is false, we will break out of this while loop, these nodes pointing to the outside nodes, but no inside nodes;
// 3). End nodes, the set of nodes that marks the end of the while body, and after these nodes are executed, we will return to the incoming nodes. These end nodes shouldn't have any edges pointing to inside nodes (OK if end nodes are condition true output nodes as well);
//
// Since these will become a sub-graph (which, to its owner graph, just simple "node"), it will have inputs and outputs. Besides that, the loop body needs to be parameterized to be SSA compliant (see: https://www.cs.cmu.edu/~fp/courses/15411-f13/lectures/06-ssa.pdf). Thus, a list of body parameters need to be provided.

// The given tensors contains all the common / input / output tensors specified in the sub-graph.
// Currently, the special_size should always be 1, and contains only the loop counter.
typedef int(*ccv_nnc_graph_while_f)(ccv_nnc_tensor_t* const* const commons, const int common_size, ccv_nnc_tensor_t* const* const inputs, const int input_size, ccv_nnc_tensor_t* const* const outputs, const int output_size, const void* const data);
// Opaque pointer to the tape of tensors. The tape are used by the while loop.
typedef struct ccv_nnc_tensor_tape_s ccv_nnc_tensor_tape_t;
// Augmented function to run a graph with while loop (An obvious example is dynamic RNN).
typedef struct ccv_nnc_tensor_multiview_s {
	// This is an augmented ccv_nnc_tensor_view_t
	// Namely, it can point to multiple versions of tensors.
	int type; // This type is CCV_NNC_TENSOR_MULTI_VIEW
	// kind specified how the multi-version tensors stored.
	// See the comment on the follow up enums.
	int kind;
	intptr_t anchor; // on which graph this multi-view tensor is wrapped. This helps to determine on which level the multi-view tensor should be unwrapped.
	// If this tensor points to a tensor view, data.u8 - offset is the real pointer start.
	off_t offset;
	struct ccv_nnc_tensor_multiview_s* p; // If this is wrapped with another multiview tensor. Get to the parent one.
	ccv_numeric_data_t dc; // The updated pointer based on data.
	// Since we only support 2 or 3 ways multi-view tensor, therefore, fixed allocation of 3.
	ccv_numeric_data_t data[3];
	ccv_nnc_tensor_t* tv; // Current tensor (tensor in use), this is updated along with the graph computation.
	// This is useful because by just traverse tv, I can get the latest up-to-date reference to this multi-view tensor.
	ccv_array_t* rtvs; // Referenced tensor view array. This corresponds to ccv_nnc_tensor_reference_to_multiview method, that records all the tensors registered for updates.
} ccv_nnc_tensor_multiview_t;

enum {
	CCV_NNC_MULTIVIEW_K11, // The first one is the first, the second one is the rest. (0111111...)
	CCV_NNC_MULTIVIEW_K02, // The first one is the first, the second one is the second, the third is the first one again (0101010101...)
	CCV_NNC_MULTIVIEW_K12, // The first one is the first, the second one is the second, the third one is the third, and 4th is the second again. (012121212...)
};
// Setup a tensor multiview with a given set of tensors.
void ccv_nnc_tensor_multiview(ccv_nnc_tensor_t* const tv, ccv_numeric_data_t data[], const int kind, const ccv_nnc_graph_t* const graph, ccv_nnc_tensor_multiview_t* const tensor_multiview);
// Since tensor_multiview will never be allocated with *_new method, the *_free method simply frees anything that is dynamically allocated afterwards (such as the reference items).
void ccv_nnc_tensor_multiview_free(const ccv_nnc_tensor_multiview_t tensor_multiview);
// Setup a tensor as a reference to a tensor multiview, thus, when tensor multiview's tu (current tensor) updates, the tensor reference's data.u8 will get update as well (point to the same memory region as the tu).
void ccv_nnc_tensor_reference_to_multiview(ccv_nnc_tensor_multiview_t* const tensor_multiview, const off_t offset, ccv_nnc_tensor_t* const tensor);
// Constructing looped concrete graph. Note that this interface is a little bit simpler than the one for symbolic graph. The reason
// is that a concrete graph operates on allocated tensors, thus, there is no mapping of tensor symbols between the parent graph
// and the while graph. (The reason to have a mapping in symbolic graphs is to constraint the variable leaking between the sub graph
// and parent graph).
CCV_WARN_UNUSED(ccv_nnc_graph_exec_t) ccv_nnc_graph_while(ccv_nnc_graph_t* const graph, uint32_t cmd, ccv_nnc_graph_t* const while_graph);
void ccv_nnc_graph_set_while_expr(ccv_nnc_graph_t* const while_graph, const ccv_nnc_graph_while_f while_expr, const void* const while_data, const ccv_nnc_graph_exec_t* const cond_evals, const int cond_eval_size);
// In that case, the computation graph still has no loops or cycles, but you can run it multiple times against different
// versions of the tensors until the condition not met (thus, the tensor is versioned, so you can "backpropagate through time").
int ccv_nnc_graph_while_run(const ccv_nnc_graph_t* const graph, ccv_nnc_tensor_tape_t* const tensor_tape, const int flags, const ccv_nnc_graph_exec_t* const sources, const int source_size, const ccv_nnc_graph_exec_t* const destinations, const int destination_size);

// The API to operate on the symbolic graph is more involved than the concrete graph for while loops.
// The reason is because symbolic graph operates in SSA form (static single assignment), therefore, the while loops
// for the symbolic graph has to be parameterized.

// Return a while exec symbol (backed by a sub-graph) of the giving graph. The exec nodes on the way from sources to destinations will be moved from the giving graph to the sub-graph.
ccv_nnc_graph_exec_symbol_t ccv_nnc_symbolic_graph_while(ccv_nnc_symbolic_graph_t* const graph, ccv_nnc_symbolic_graph_t* const while_graph, const char* const name);
// Set the expression to be evaluated, and at which nodes to be evaluated.
void ccv_nnc_symbolic_graph_set_while_expr(ccv_nnc_symbolic_graph_t* const while_graph, const ccv_nnc_graph_while_f while_expr, const void* const while_data, const ccv_nnc_graph_exec_symbol_t* const cond_evals, const int cond_eval_size);

typedef struct {
	ccv_nnc_tensor_symbol_t source;
	ccv_nnc_tensor_symbol_t destination;
} ccv_nnc_tensor_symbol_map_t;

// Set the loop parameters when reuse. (parameterized loop).
void ccv_nnc_symbolic_graph_set_while_params(ccv_nnc_symbolic_graph_t* const while_graph, const ccv_nnc_tensor_symbol_map_t* const symbol_map, const int symbol_map_size);
// Retrieve the special (magical) tensor symbol that retains the while loop counter (thus, dimension of 1x1x1, CCV_64S type).
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_tensor_symbol_for_while_count(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t while_symbol);
CCV_WARN_UNUSED(ccv_nnc_tensor_symbol_t) ccv_nnc_find_tensor_symbol_from_graph(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_tensor_symbol_t symbol);
// Extract the sub-graph of the while loop from a symbol.
CCV_WARN_UNUSED(ccv_nnc_symbolic_graph_t*) ccv_nnc_symbolic_graph_from_while_symbol(const ccv_nnc_symbolic_graph_t* const graph, const ccv_nnc_graph_exec_symbol_t while_symbol);

#endif
