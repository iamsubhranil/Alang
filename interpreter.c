#include <float.h>
#include <inttypes.h>
#include <math.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#include "allocator.h"
//#include "callframe.h"
#include "datastack.h"
#include "display.h"
//#include "env.h"
//#include "fman.h"
#include "interpreter.h"
#include "io.h"
#include "native.h"
#include "parser.h"
#include "routines.h"
#include "scanner.h"
#include "strings.h"
#include "values.h"

static uint8_t *instructions = NULL;
static size_t   lastins = 0, fileInfoPointer = 0;
static size_t   ip = 0, baseptr = 0;

static FileInfo *fileInfos = NULL;

int info_onSameFile(Token t) {
	uint32_t nname = str_insert(strdup(t.fileName), 1);
	FileInfo l     = fileInfos[fileInfoPointer - 1];
	return ((l.fileName == nname) && ((int)l.line == t.line));
}

size_t ins_add(uint8_t ins) {
	instructions         = (uint8_t *)reallocate(instructions, 8 * ++ip);
	instructions[ip - 1] = ins;
	lastins              = ip - 1;
	Token t              = presentToken();
	// dbg("IP : %" PRIu32 " FileName : %s Line : %" PRIu32, ip - 1, t.fileName,
	// t.line);
	if(fileInfoPointer == 0 || !info_onSameFile(t)) {
		if(fileInfoPointer > 0) {
			//  dbg("New block created : %" PRId32 " to %" PRId32 " Line : %"
			//  PRId32, fileInfos[fileInfoPointer - 1].from, ip - 2,
			//          fileInfos[fileInfoPointer - 1].line);
			fileInfos[fileInfoPointer - 1].to = ip - 2;
		}
		fileInfos = (FileInfo *)reallocate(fileInfos, sizeof(FileInfo) *
		                                                  ++fileInfoPointer);
		fileInfos[fileInfoPointer - 1].fileName =
		    str_insert(strdup(t.fileName), 1);
		fileInfos[fileInfoPointer - 1].line = t.line;
		fileInfos[fileInfoPointer - 1].from = ip - 1;
		fileInfos[fileInfoPointer - 1].to   = INT32_MAX;
	}
	return ip - 1;
}

void ins_set(size_t mem, uint8_t ins) {
	instructions[mem] = ins;
}

uint8_t ins_last() {
	return instructions[lastins];
}

size_t ins_add_val(uint32_t store) {
	//    printf("\nStoring %lu at %lu : 0x", store, ip);
	instructions = (uint8_t *)reallocate(instructions, 8 * (ip + 4));
	uint8_t i    = 0;
	while(i < 4) {
		instructions[ip + i] = (store >> (i * 8)) & 0xff;
		//        printf("%x", instructions[ip + i]);
		i++;
	}
	ip += 4;

	//    ins_print();
	return ip - 4;
}

size_t ins_add_double(double d) {
	uint8_t *bits = (uint8_t *)&d;
	instructions =
	    (uint8_t *)reallocate(instructions, 8 * (ip + sizeof(double)));
	uint32_t i = 0;
	while(i < sizeof(double)) {
		instructions[ip + i] = bits[i];
		i++;
	}
	ip += 8;

	//    ins_print();
	return ip - 8;
}

void ins_set_val(size_t mem, uint32_t store) {
	//    printf("\nReStoring %lu at %lu : 0x", store, mem);
	uint8_t i = 0;
	while(i < 4) {
		instructions[mem + i] = (store >> (i * 8)) & 0xff;
		//        printf("%x", instructions[mem + i]);
		i++;
	}
}

static inline uint32_t ins_get_val(size_t mem) {
	uint32_t *ret = (uint32_t *)&instructions[mem];
	return *ret;
}

static inline double ins_get_double(size_t mem) {
	double *ret = (double *)&instructions[mem];
	return *ret;
}

uint8_t ins_get(size_t mem) {
	return instructions[mem];
}

size_t ip_get() {
	return ip;
}

static const char *opString[] = {
#define INSTRUCTION(X) #X,
#include "instruction.h"
#undef INSTRUCTION
};

void print_stack_trace() {
	size_t bakptr = baseptr;
	size_t bip    = ip;
	while(bakptr > 0) {
		bip         = tint(dataStack[bakptr - 2]);
		FileInfo fi = fileInfo_of(bip);
		info("Called from %s:%" PRIu32, str_get(fi.fileName), fi.line);
		bakptr = tint(dataStack[bakptr - 1]);
	}
}

void ins_print() {
	uint32_t i = 0;
	printf("\n\nPrinting memory[ip : %zu]..\n", ip);
	while(i < ip) {
		printf("%x ", instructions[i]);
		i++;
		if(i > 0 && (i % 8 == 0))
			printf("] [");
	}
	//
	i = 0;
	printf("\nPrinting instructions..\n");
	while(i < ip) {
		printf("ip %4" PRIu32 " : ", i);
		printf("%-20s ", opString[instructions[i]]);
		switch(instructions[i]) {
			case PUSHF:
				printf("%g", ins_get_double(++i));
				i += 7;
				break;
			case PUSHI:
				printf("%" PRId32, ins_get_val(++i));
				i += 3;
				break;
			case PUSHL:
				printf("%s", ins_get_val(++i) == 0 ? "false" : "true");
				i += 3;
				break;
			case PUSHS:
			case PUSHID:
			case MEMREF:
			case NEW_CONTAINER:
				printf("%s", str_get(ins_get_val(++i)));
				i += 3;
				break;
			case INPUTF:
			case INPUTI:
			case INPUTS:
			case JUMP:
			case JUMP_IF_TRUE:
			case JUMP_IF_FALSE:
			case LOAD_SLOT:
			case LOAD_SLOT_GLOBAL:
			case MAKE_ARRAY:
			case RESERVE_SLOT:
			case SAVE_STORE_SLOT:
				printf("%" PRIu32, ins_get_val(++i));
				i += 3;
				break;
			case CALLVAR:
			case CALL:
				printf("%" PRIu32, ins_get_val(++i));
				i += 3;
				printf(" numarg=%" PRIu32, ins_get_val(++i));
				i += 3;
				printf(" arity=%" PRIu32, ins_get_val(++i));
				i += 3;
				break;
			case CALLNATIVE:
				printf("%s", str_get(ins_get_val(++i)));
				i += 3;
				printf(" arity=%" PRIu32, ins_get_val(++i));
				i += 3;
				break;
		}
		i++;
		printf("\n");
	}
}

static clock_t tmStart, tmEnd;
#ifdef COUNT_INS
#define LAST_OP SAVE_STORE_SLOT
static uint64_t insExec = 0, counter[LAST_OP] = {0};

void print_stat() {
	uint8_t i = 0;
	printf("\n     Instruction        Execution Count");
	printf("\n====================    ===============");
	while(i < LAST_OP) {
		if(counter[i] > 0) {
			printf("\n%20s", opString[i]);
			printf("\t%" PRIu64, counter[i]);
		}
		i++;
	}
	printf("\n");
}
#endif
// static CallFrame callFrame;

void stop() {
	tmEnd     = clock() - tmStart;
	double tm = (double)tmEnd / CLOCKS_PER_SEC;
	// printf("\nRealloc called : %d times\n", get_realloc_count());
	printf("\n");
	dbg("[Interpreter] Execution time : %gs", tm);
#ifdef COUNT_INS
	dbg("[Interpreter] Instructions executed : %" PRIu64, insExec);
	dbg("[Interpreter] Average execution speed : %gs", tm / insExec);
	print_stat();
	printf("\n");
#endif

	dStackFree();
	obj_free();
	// env_free(callFrame.env);
	str_free();
	memfree(instructions);
	memfree(fileInfos);
	// cs_free();
	unload_all();
	routine_free();
	// free_all();
	printf("\n");
	exit(0);
}

static void printString(const char *s) {
	int i = 0, len = strlen(s);
	// printf("\nPrinting : %s", s);
	while(i < len) {
		if(s[i] == '\\' && i < (len - 1)) {
			if(s[i + 1] == 'n') {
				putchar('\n');
				i++;
			} else if(s[i + 1] == 't') {
				putchar('\t');
				i++;
			} else if(s[i + 1] == '"') {
				putchar('"');
				i++;
			} else
				putchar('\\');
		} else
			putchar(s[i]);
		i++;
	}
}

static uint8_t init = 0;

void init_interpreter() {
#ifdef DYNAMIC_STACK
	dataStack = NULL;
#endif
	sp        = 0;
	stackSize = 0;
	dStackInit();
	init = 1;
}

FileInfo fileInfo_of(uint32_t ip) {
	uint32_t i = 0;
	// dbg("Searching for info of IP %" PRIu32, ip);
	while(i < fileInfoPointer) {
		if(fileInfos[i].from <= ip && fileInfos[i].to >= ip)
			return fileInfos[i];
		i++;
	}
	// err("No file information found for IP : %" PRIu32 "!", ip);
	return fileInfos[0];
}

void print_op_type(Data op) {
	printf("Data : 0x%lx ", op);
	if(isfloat(op)) {
		printf("Float[ %g ]", tfloat(op));
		return;
	}
	switch(ttype(op)) {
		// case INT:
		//    printf("Integer[ %" PRId32 " ]", tint(op));
		//    break;
		case STRING: printf("String[ %s ]", tstr(op)); break;
		case LOGICAL:
			printf("Logical[ %s ]", tlogical(op) == 0 ? "False" : "True");
			break;
		case IDENTIFIER: printf("Identifer[ %s ]", tstr(op)); break;
		case INSTANCE:
			printf("Instance[ Container %s ]", str_get(tins(op)->name));
			break;
		case NIL: printf("Null"); break;
		case NONE: printf("None"); break;
		case ARR: printf("Array[ Size %zu]", arr_size(tarr(op))); break;
		default: printf("Unknown type 0x%lx[Data %lx]!", ttype(op), op); break;
	}
}

void interpreter_push(Data d) {
	dpush(d);
}

#define check_limit(x)                                                \
	{                                                                 \
		if(x > INT32_MAX) {                                           \
			rerr("Integer overflow : %" PRId64 " > %" PRId32 "!", x,  \
			     INT32_MAX);                                          \
		} else if(x < INT32_MIN) {                                    \
			rerr("Integer underflow : %" PRId64 " < %" PRId32 "!", x, \
			     INT32_MIN);                                          \
		}                                                             \
	}

size_t stored_slots[16]    = {0};
size_t stored_slot_pointer = 0;

void interpret() {
	if(init == 0)
		init_interpreter();

#define push_store_slot(x) stored_slots[stored_slot_pointer++] = x
#define pop_store_slot() stored_slots[--stored_slot_pointer]

	ip = 0;
	load_natives();
	baseptr         = 0; // The base pointer for slot referencing
	Data *baseStack = &dataStack[0];
	// uint32_t           store_slot = 0;
	static const void *dispatchTable[] = {
#define INSTRUCTION(x) &&DO_##x,
#include "instruction.h"
#undef INSTRUCTION
	};

//#define COUNT_INS
#ifdef COUNT_INS
#define INC_COUNTER()                    \
	{                                    \
		insExec++;                       \
		++counter[instructions[ip + 1]]; \
	}
#define INC_COUNTER_WINC()           \
	{                                \
		insExec++;                   \
		++counter[instructions[ip]]; \
	}
#else
#define INC_COUNTER()
#define INC_COUNTER_WINC()
#endif
#define GO() \
	{ goto *dispatchTable[instructions[++ip]]; }
#define GO_WINC() \
	{ goto *dispatchTable[instructions[ip]]; }
#define SHOW_STEP()                                                       \
	{                                                                     \
		FileInfo fInfo = fileInfo_of(ip + 1);                             \
		dbg("[%s:%3" PRIu32 "][ip : %4" PRIu32 ", sp : %4" PRIu32 "] %s", \
		    str_get(fInfo.fileName), fInfo.line, ip, sp,                  \
		    opString[instructions[ip + 1]]);                              \
		fflush(stdin);                                                    \
		getchar();                                                        \
	}
#define SHOW_STEP2()                                                      \
	{                                                                     \
		FileInfo fInfo = fileInfo_of(ip);                                 \
		dbg("[%s:%3" PRIu32 "][ip : %4" PRIu32 ", sp : %4" PRIu32 "] %s", \
		    str_get(fInfo.fileName), fInfo.line, ip, sp,                  \
		    opString[instructions[ip]]);                                  \
		fflush(stdin);                                                    \
		getchar();                                                        \
	}

	//#define STEP

#ifdef STEP
#define DISPATCH()                             \
	{                                          \
		INC_COUNTER();                         \
		print_value("any", dataStack[sp - 1]); \
		SHOW_STEP();                           \
		GO();                                  \
	}
#define DISPATCH_WINC()                        \
	{                                          \
		INC_COUNTER_WINC();                    \
		print_value("any", dataStack[sp - 1]); \
		SHOW_STEP2();                          \
		GO_WINC();                             \
	}
#else
#define DISPATCH()     \
	{                  \
		INC_COUNTER(); \
		GO();          \
	}
#define DISPATCH_WINC()     \
	{                       \
		INC_COUNTER_WINC(); \
		GO_WINC();          \
	}
#endif

	tmStart = clock();
	DISPATCH_WINC();
	while(1) {
	DO_PUSHF:
		dpushf(ins_get_double(++ip));
		ip += 7;
		DISPATCH();
	DO_PUSHI:
		dpushi((int32_t)ins_get_val(++ip));
		ip += 3;
		DISPATCH();
	DO_PUSHL:
		dpushl((int32_t)ins_get_val(++ip));
		ip += 3;
		DISPATCH();
	DO_PUSHS:
		dpushsk(ins_get_val(++ip));
		ip += 3;
		DISPATCH();
	DO_PUSHID:
		dpushidk(ins_get_val(++ip));
		ip += 3;
		DISPATCH();
	DO_PUSHN:
		dpushn();
		DISPATCH();
	DO_ADD : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			double res = tfloat(d1) + tfloat(d2);
			dpushf(res);
			DISPATCH();
		}
		if(isstr(d1) && isstr(d2)) {
			size_t s1 = str_len(tstrk(d1)), s2 = str_len(tstrk(d2));
			char * res = (char *)mallocate(sizeof(char) * (s1 + s2 + 1));
			res[0]     = '\0';
			strcat(res, str_get(tstrk(d2)));
			strcat(res, str_get(tstrk(d1)));
			res[s1 + s2] = '\0';
			dpushs(res);
			DISPATCH();
		}
		rerr("Bad operands for operator '+'!");
	}
	DO_SUB : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			double res = tfloat(d2) - tfloat(d1);
			dpushf(res);
			DISPATCH();
		}
		rerr("Bad operands for operator '-'!");
	}
	DO_MUL : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			double res = tfloat(d2) * tfloat(d1);
			dpushf(res);
			DISPATCH();
		}
		rerr("Bad operands for operator '-'!");
	}
	DO_DIV : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) || isfloat(d2)) {
			if(fabs(tfloat(d1)) <= DBL_EPSILON) {
				rerr("Division by zero!");
			}
			double res = tfloat(d2) / tfloat(d1);
			dpushf(res);
			DISPATCH();
		}
		rerr("Bad operands for operator '-'!");
	}
	DO_POW : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			dpushf(pow(tfloat(d2), tfloat(d1)));
			DISPATCH();
		}
		rerr("Bad operands for operator '^'!");
	}
	DO_MOD : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			dpushi(fmod(tfloat(d2), tfloat(d1)));
			DISPATCH();
		}
		rerr("Bad operands for operator '%%'");
	}

#define STORE_SLOT_X(N)                            \
	DO_STORE_SLOT_##N : {                          \
		Data value;                                \
		dpop(value);                               \
		size_t store_slot = N;                     \
		Data   oldvalue   = baseStack[store_slot]; \
		ref_incr(value);                           \
		ref_decr(oldvalue);                        \
		baseStack[store_slot] = value;             \
		DISPATCH();                                \
	}

		STORE_SLOT_X(0)
		STORE_SLOT_X(1)
		STORE_SLOT_X(2)
		STORE_SLOT_X(3)
		STORE_SLOT_X(4)
		STORE_SLOT_X(5)
		STORE_SLOT_X(6)
		STORE_SLOT_X(7)

	DO_GT : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			dpushl(tfloat(d2) > tfloat(d1));
			DISPATCH();
		}
		if(isstr(d1) && isstr(d2)) {
			dpushl(str_len(tstrk(d2)) > str_len(tstrk(d1)));
			DISPATCH();
		}
		rerr("Bad operands for operator '>'!");
	}
	DO_GTE : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(isfloat(d1) && isfloat(d2)) {
			dpushl(tfloat(d2) >= tfloat(d1));
			DISPATCH();
		}

		if(isstr(d1) && isstr(d2)) {
			dpushl(str_len(tstrk(d2)) >= str_len(tstrk(d1)));
			DISPATCH();
		}
		rerr("Bad operands for operator '>='!");
	}
	DO_LT : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);

		if(isfloat(d1) && isfloat(d2)) {
			dpushl(tfloat(d2) < tfloat(d1));
			DISPATCH();
		}

		if(isstr(d1) && isstr(d2)) {
			dpushl(str_len(tstrk(d2)) < str_len(tstrk(d1)));
			DISPATCH();
		}
		rerr("Bad operands for operator '<'!");
	}
	DO_LTE : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);

		if(isfloat(d1) && isfloat(d2)) {
			dpushl(tfloat(d2) <= tfloat(d1));
			DISPATCH();
		}

		if(isstr(d1) && isstr(d2)) {
			dpushl(str_len(tstrk(d2)) <= str_len(tstrk(d1)));
			DISPATCH();
		}

		rerr("Bad operands for operator '<='!");
	}
	DO_EQ : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);

		if(isfloat(d1) && isfloat(d2)) {
			dpushl(fabs(tfloat(d2) - tfloat(d1)) <= DBL_EPSILON);
			DISPATCH();
		}

		if(isnull(d1) || isnull(d2)) {
			dpushl(isnull(d1) && isnull(d2));
			DISPATCH();
		}

		if(isstr(d1) && isstr(d2)) {
			dpushl(tstrk(d2) == tstrk(d1));
			DISPATCH();
		}
		rerr("Bad operands for operator '=='!");
	}
	DO_NEQ : {

		Data d1, d2;
		dpop(d1);
		dpop(d2);

		if(isfloat(d1) && isfloat(d2)) {
			dpushl(fabs(tfloat(d2) - tfloat(d1)) > DBL_EPSILON);
			DISPATCH();
		}

		if(isnull(d1) || isnull(d2)) {
			dpushl(!(isnull(d1) && isnull(d2)));
			DISPATCH();
		}

		if(isstr(d1) && isstr(d2)) {
			dpushl(tstrk(d1) != tstrk(d2));
			DISPATCH();
		}
		rerr("Bad operands for operator '!='!");
	}
	DO_AND : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(islogical(d1) && islogical(d2)) {
			dpushl(tlogical(d1) && tlogical(d2));
			DISPATCH();
		}

		rerr("Bad operands for operator 'And'!");
	}
	DO_OR : {
		Data d1, d2;
		dpop(d1);
		dpop(d2);
		if(islogical(d1) && islogical(d2)) {
			dpushl(tlogical(d1) || tlogical(d2));
			DISPATCH();
		}

		rerr("Bad operands for operator 'Or'!");
	}
	DO_INPUTI : {
		dpush(getInt());
		DISPATCH();
	}
	DO_INPUTS : {
		dpush(getString());
		DISPATCH();
	}
	DO_INPUTF : {
		dpush(getFloat());
		DISPATCH();
	}
	DO_PRINT : {
		Data value;
		dpop(value);
		// dbg("Value : %lx", value);
		if(isfloat(value)) {
			printf("%g", tfloat(value));
			DISPATCH();
		}
		switch(ttype(value)) {
			// case INT:
			//     printf("%" PRId32, tint(value));
			//     DISPATCH();
			case LOGICAL:
				printf("%s", tint(value) == 0 ? "False" : "True");
				DISPATCH();
			case NIL: printf("Null"); DISPATCH();
			case STRING: printString(str_get(tstrk(value))); DISPATCH();
			case INSTANCE:
				printf("<instance of %s>", str_get(tins(value)->name));
				DISPATCH();
			case IDENTIFIER:
				printf("<identifer %s>", str_get(tstrk(value)));
				DISPATCH();
			case ARR:
				printf("<array of %zu>", arr_size(tarr(value)));
				DISPATCH();
			case NONE: printf("<none>"); DISPATCH();
		}
	}
	DO_HALT:
		stop();
	DO_JUMP_IF_FALSE : {
		Data     c;
		uint32_t ja = ins_get_val(++ip);
		dpop(c);
		if(islogical(c)) {
			if(!tlogical(c)) {
				ip = ja;
				DISPATCH_WINC();
			}
			ip += 3;
			//        printf("\nCond is true!");
			DISPATCH();
		}

		rerr("Illogical jump!");
	}
	DO_JUMP_IF_TRUE : {
		Data     c;
		uint32_t ja = ins_get_val(++ip);
		dpop(c);
		if(islogical(c)) {
			if(tlogical(c)) {
				ip = ja;
				DISPATCH_WINC();
			}
			ip += 3;
			DISPATCH();
		}

		rerr("Illogical jump!");
	}
	DO_JUMP : {
		uint32_t ja = ins_get_val(++ip);
		ip          = ja;
		DISPATCH_WINC();
	}
#define PREPARE_CALL()                                       \
	uint32_t ja;                                             \
	ja = ins_get_val(++ip);                                  \
	ip += 3;                                                 \
	uint32_t numArg = ins_get_val(++ip), bak = numArg;       \
	ip += 3;                                                 \
	uint32_t arity = ins_get_val(++ip);                      \
	ip += 3;                                                 \
	/* Add two more slots to the top of the stack */         \
	dpushn();                                                \
	dpushn();                                                \
	/*  sp points to the next slot, so decrement it          \
	    to point to the first empty slot */                  \
	--sp;                                                    \
	/* Move all arguments to two two slot top */             \
	while(numArg--) {                                        \
		dataStack[sp] = dataStack[sp - 2];                   \
		/*  Since they are now stored in a new slot,         \
		    increment their reference count */               \
		ref_incr(dataStack[sp]);                             \
		--sp;                                                \
	}                                                        \
	numArg = bak;                                            \
	/*  sp now points to the first empty slot from top       \
	    point it to the second  */                           \
	--sp;                                                    \
	/*  Push return address and baseptr to the newly created \
	    two slots */                                         \
	dpushi(ip + 1);  /* Push the return address */           \
	dpushi(baseptr); /* Push the base pointer */             \
	baseptr   = sp;  /* Assign the new base pointer */       \
	baseStack = &dataStack[baseptr];                         \
	/* Move the stack pointer to the top of arguments */     \
	sp += numArg;
	DO_CALLVAR : {
		PREPARE_CALL()
		// Find the number of extra arguments
		uint32_t extra = numArg - (arity - 2);
		// Now create the array which will store the
		// variadic arguments
		Data array = new_array(extra);
		// Increment refCount of the array
		obj_ref_incr(tarr(array));
		// Now pop all 'extra' arguments from the stack,
		// and assign them to the array
		while(extra > 0) {
			dpop(arr_elements(tarr(array))[extra - 1]);
			extra--;
		}
		// Finally, push the array and the count as the last
		// arguments
		dpush(array);
		dpushi(numArg - (arity - 2));

		// Now complete the call
		ip = ja;
		DISPATCH_WINC();
	}
	DO_CALL : {
		PREPARE_CALL()
		// printf("\nbaseptr now %d", baseptr);

		ip = ja;
		DISPATCH_WINC();
	}
	DO_RESERVE_SLOT : { // Reserve slot for local variables
		uint32_t count = ins_get_val(++ip);
		ip += 3;
		while(count-- > 0) dpushn();
		DISPATCH();
	}
#define LOAD_SLOT_X(N)       \
	DO_LOAD_SLOT_##N : {     \
		dpush(baseStack[N]); \
		DISPATCH();          \
	}

		LOAD_SLOT_X(0)
		LOAD_SLOT_X(1)
		LOAD_SLOT_X(2)
		LOAD_SLOT_X(3)
		LOAD_SLOT_X(4)
		LOAD_SLOT_X(5)
		LOAD_SLOT_X(6)
		LOAD_SLOT_X(7)
	DO_CALLNATIVE : {
		PREPARE_CALL()

		// check if it is a variadic call
		if(arity > 0) {
			// Find the number of extra arguments
			uint32_t extra = bak - (arity - 2);
			// Now create the array which will store the
			// variadic arguments
			Data array = new_array(extra);
			// Increment refCount of the array
			obj_ref_incr(tarr(array));
			// Now pop all 'extra' arguments from the stack,
			// and assign them to the array
			while(extra > 0) {
				dpop(arr_elements(tarr(array))[extra - 1]);
				extra--;
			}
			// Finally, push the array and the count as the last
			// arguments
			dpush(array);
			dpushi(bak - (arity - 2));
		}
		dpush(handle_native(ja, bak, baseStack));
		goto DO_RETURN;
	}
	DO_NEW_CONTAINER : {
		uint32_t name = ins_get_val(++ip);
		ip += 3;
		// Retrieve the Routine
		Routine2 *r = routine_get(name);
		// Create the instance
		Data ins = new_ins(baseStack, r);
		// dbg("refcount at creation %u", tins(ins)->obj.refCount);
		// Pop all the slots
		sp -= r->slots;
		// Push the instance
		dpush(ins);
		// Finally, return from the
		// initializer
		// goto DO_RETURN;
	}
	DO_RETURN : {
		// Return value
		Data ret;
		dpop(ret);
		// dbg("Returning");
		// print_value("any", ret);
		// ref_incr(ret);

		// Decrement ref of locals
		while(sp > baseptr) {
			// printf("\nPopping %lu", sp);
			Data p;
			dpop(p);
			ref_decr(p);
		}

		// Pop the old baseptr
		dpopi(baseptr);
		// Repoint the baseStack
		baseStack = &dataStack[baseptr];

		// Pop the return address
		dpopi(ip);

		// Push the return value back
		dpush(ret);

		// dbg("Returning to %u\n", ip);

		/*
		   ip = callFrame.returnAddress;
		   cf_free(callFrame);
		   callFrame = cf_pop();
		// dbg("ReStoring address : %lu", callFrame.returnAddress);
		*/
		DISPATCH_WINC();
	}
	DO_POP : {
		Data d;
		dpop(d);
		// It doesn't need to decr the ref, since it has already
		// been done by ref or such
		// ref_decr(d);
		DISPATCH();
	}
	DO_ARRAYREF : {
		Data index;
		Data arr;
		dpop(index);
		dpop(arr);
		if(!isarray(arr) && !isstr(arr)) {
			rerr("Subscripted element is not an array or string!");
		}
		if(isint(index)) {
			if(isarray(arr)) {
				if(tint(index) < 1 || tint(index) > arr_size(tarr(arr))) {
					rerr("Array index out of range : %" PRId32, tint(index));
				}

				dpush(arr_elements(tarr(arr))[tint(index) - 1]);
				DISPATCH();
			}

			if(tint(index) < 1 ||
			   (size_t)tint(index) > (str_len(tstrk(arr)) + 1)) {
				rerr("String index out of range for '%s' : %" PRId32
				     " [Expected <= %" PRIu32 "]",
				     str_get(tstrk(arr)), tint(index), str_len(tstrk(arr)));
			}

			if((size_t)tint(index) == str_len(tstrk(arr)) + 1) {
				dpushn();
				DISPATCH();
			}

			char *c = (char *)mallocate(sizeof(char) * 2);
			c[0]    = tstr(arr)[tint(index) - 1];
			c[1]    = 0;
			dpushs(c);
			DISPATCH();
		}

		rerr("Array index must be an integer!");
	}
	DO_MEMREF : {
		Data     ins;
		uint32_t mem = ins_get_val(++ip);
		ip += 3;
		dpop(ins);
		if(isins(ins)) {
			dpush(get_member(tins(ins), mem));
			DISPATCH();
		}
		rerr("Dereferenced value is not a container instance!");
	}
	DO_MAKE_ARRAY : {
		Data size;
		dpop(size);
		uint32_t slot = ins_get_val(++ip);
		ip += 3;
		if(isint(size)) {
			if(tint(size) > 0) {
				Data array      = new_array(tint(size));
				baseStack[slot] = array;
				DISPATCH();
			}
			rerr("Array size must be > 0!");
		}

		rerr("Array size must be an integer!");
	}
	DO_NOOP : { DISPATCH(); }
	DO_MEMSET : {
		Data     ins, data;
		uint32_t mem;
		// ip += 3;
		dpop(data);
		dpopi(mem);
		dpop(ins);
		if(isins(ins)) {
			set_member(tins(ins), mem, data);
			DISPATCH();
		}
		rerr("Referenced value is not a container instance!");
	}
	DO_ARRAYSET : {
		Data index, arr, value;
		dpop(value);
		dpop(index);
		dpop(arr);
		if(isint(index)) {
			if(isarray(arr)) {
				if(tint(index) > 0 && tint(index) <= arr_size(tarr(arr))) {
					Data *pos = &arr_elements(tarr(arr))[tint(index) - 1];
					if(*pos != value) {
						ref_decr(*pos);
						ref_incr(value);
						*pos = value;
					}
					DISPATCH();
				}
				rerr("Array index out of range : %" PRId32, tint(index));
			}
			rerr("Subscripted item is not an array!");
		}
		rerr("Array index must be an integer!");
	}
	DO_STORE_SLOT : { // Set slot to a value
		Data value;
		dpop(value);
		// slot += baseptr;
		size_t store_slot = pop_store_slot();
		Data   oldvalue   = baseStack[store_slot];
		if(oldvalue != value) {
			// dbg("Incrementing newvalue local");
			// print_op_type(value);
			ref_incr(value);
			// dbg("Decrementing oldvalue local\n");
			// print_op_type(oldvalue);
			ref_decr(oldvalue);
			baseStack[store_slot] = value;
		}
		DISPATCH();
	}
	DO_LOAD_SLOT : { // Load value from slot
		uint32_t slot = ins_get_val(++ip);
		ip += 3;
		dpush(baseStack[slot]);
		// printf("\nslotnum : %d pointer : %p", baseptr + slot,
		// &dataStack[baseptr + slot]); print_value("value", dpeek());
		DISPATCH();
	}
	DO_STORE_SLOT_GLOBAL : {
		Data value;
		dpop(value);
		size_t store_slot = pop_store_slot();
		Data   oldValue   = dataStack[store_slot];
		if(oldValue != value) {
			// print_value("any", value);
			// dbg("Incrementing newvalue global");
			ref_incr(value);
			// dbg("Decrementing oldvalue global");
			ref_decr(oldValue);
			dataStack[store_slot] = value;
		}
		DISPATCH();
	}
	DO_LOAD_SLOT_GLOBAL : { // Load value from global slot
		uint32_t slot = ins_get_val(++ip);
		ip += 3;
		dpush(dataStack[slot]);
		DISPATCH();
	}
	DO_SAVE_STORE_SLOT : { // Store the store_slot for the next STORE_ call
		push_store_slot(ins_get_val(++ip));
		ip += 3;
		DISPATCH();
	}
	}
}
