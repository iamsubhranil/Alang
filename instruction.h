// Load
INSTRUCTION(PUSHF)  // String => push index
INSTRUCTION(PUSHI)  // Identifer
INSTRUCTION(PUSHL)  // Int => push constant
INSTRUCTION(PUSHS)  // Float => push constant
INSTRUCTION(PUSHID) // Logical => push 0/1
INSTRUCTION(PUSHN)  // Null => pushN, usually followed by return
// NEW_CONTAINER calls return, in turn
INSTRUCTION(NEW_CONTAINER) // Takes container name to resolve
INSTRUCTION(RETURN)
// Pop the top of the stack,
// decr the ref if needed,
// and simply discard it
INSTRUCTION(POP)
// Arithmetic, almost always followed by a store or another arithmetic
INSTRUCTION(ADD)
INSTRUCTION(SUB)
INSTRUCTION(MUL)
INSTRUCTION(DIV)
INSTRUCTION(POW)
INSTRUCTION(MOD)
// Specific codes for STOREing to first 8 slots
INSTRUCTION(STORE_SLOT_0)
INSTRUCTION(STORE_SLOT_1)
INSTRUCTION(STORE_SLOT_2)
INSTRUCTION(STORE_SLOT_3)
INSTRUCTION(STORE_SLOT_4)
INSTRUCTION(STORE_SLOT_5)
INSTRUCTION(STORE_SLOT_6)
INSTRUCTION(STORE_SLOT_7)
// Conditional, always followed by jumps
INSTRUCTION(GT)
INSTRUCTION(GTE)
INSTRUCTION(LT)
INSTRUCTION(LTE)
INSTRUCTION(EQ)
INSTRUCTION(NEQ)
INSTRUCTION(AND)
INSTRUCTION(OR)
// Jumps
INSTRUCTION(JUMP_IF_FALSE)
INSTRUCTION(JUMP_IF_TRUE)
INSTRUCTION(JUMP)
// Basic IO operations
INSTRUCTION(
    INPUTI) // Just reads an integer from stdin and pushes it to the stack
INSTRUCTION(INPUTS) //               string
INSTRUCTION(INPUTF) //               float
// Control flow and branching
INSTRUCTION(HALT)
// Data structures
INSTRUCTION(ARRAYREF)   // Array access
INSTRUCTION(MEMREF)     // Takes the ID to resolve
INSTRUCTION(MAKE_ARRAY) // Takes the slot number to store
// Noop
INSTRUCTION(NOOP)
// Container
INSTRUCTION(MEMSET) // top <- value, top - 1 <- member id, top - 2 <- ins
INSTRUCTION(ARRAYSET)
// Native Calls
INSTRUCTION(CALLNATIVE)
// Variadic Call
// A separate instruction will reduce
// an 'if' inside the original CALL
// instruction to check for an compact
// variadic arguments, thereby costing
// nothing for the routines that do not
// require it.
INSTRUCTION(CALLVAR)
// Call is always followed by a reserve slot,
// which in turn is followed by LOAD_
INSTRUCTION(CALL)
// Reserve specified number of slots on
// the stack
INSTRUCTION(RESERVE_SLOT)
// Specific codes for LOADing first 8 slots
INSTRUCTION(LOAD_SLOT_0)
INSTRUCTION(LOAD_SLOT_1)
INSTRUCTION(LOAD_SLOT_2)
INSTRUCTION(LOAD_SLOT_3)
INSTRUCTION(LOAD_SLOT_4)
INSTRUCTION(LOAD_SLOT_5)
INSTRUCTION(LOAD_SLOT_6)
INSTRUCTION(LOAD_SLOT_7)
// Set the specified slot in (top - 1)
// with the top of the stack
INSTRUCTION(STORE_SLOT)
// Load the value stored in the operand
// slot to the top of the stack
INSTRUCTION(LOAD_SLOT)
// Store into global slot N
// First operand is the slot number
// Second operand is the parent number <-- Parent number is not needed,
//                                          since global environment
//                                          is always the enclosing env
//                                          of everyone
INSTRUCTION(STORE_SLOT_GLOBAL)
// Load from global slot N
INSTRUCTION(LOAD_SLOT_GLOBAL)
// Save store slot
// Saves the future store slot for the
// store_ call in a variable, instead
// of pushing into the stack
INSTRUCTION(SAVE_STORE_SLOT)
