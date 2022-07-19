#pragma once

#include <stack>
#include <unordered_map>

#include "common/Pcsx2Types.h"
#include "R5900.h"

constexpr int GOAL_FUNCARG0_REG = 4;
constexpr int GOAL_FUNCARG1_REG = 5;
constexpr int GOAL_FUNCARG2_REG = 6;
constexpr int GOAL_FUNCARG3_REG = 7;
constexpr int GOAL_FUNCARG4_REG = 8;
constexpr int GOAL_FUNCARG5_REG = 9;
constexpr int GOAL_FUNCARG6_REG = 10;
constexpr int GOAL_FUNCARG7_REG = 11;
constexpr int GOAL_SYMTABLE_REG = 23; // s7/r23
constexpr int GOAL_FUNCTION_REG = 25; // t9/r25

std::string goalGetSymName(u16 sym);
std::string goalGetSymNameDirectly(u32 ptr);
u16 goalGetSymFromPtr(u32 ptr);
std::string goalGetSymNameFromPtr(u32 ptr);

enum class CallType {
	METHOD,
	FUNCTION
};

struct CallSignature
{
	std::string type_or_func_name;
	int method_id;
	u32 func_ptr;
	CallType call_type;

	u8 reg_changed = 0;
	u32 arg_count_estimate = 0;
	u128 reg_data[8];
	u128 arg_data[8];
	bool arg_is_symbol[8];
	bool arg_is_value[8];
	std::string arg_types[8] = {};

	CallSignature(u32 l_func_ptr, std::string& type_or_func_name, int method_id, CallType call_type)
		: func_ptr(l_func_ptr), type_or_func_name(type_or_func_name), method_id(method_id), call_type(call_type)
	{
		reg_data[0] = cpuRegs.GPR.r[GOAL_FUNCARG0_REG].UQ;
		reg_data[1] = cpuRegs.GPR.r[GOAL_FUNCARG1_REG].UQ;
		reg_data[2] = cpuRegs.GPR.r[GOAL_FUNCARG2_REG].UQ;
		reg_data[3] = cpuRegs.GPR.r[GOAL_FUNCARG3_REG].UQ;
		reg_data[4] = cpuRegs.GPR.r[GOAL_FUNCARG4_REG].UQ;
		reg_data[5] = cpuRegs.GPR.r[GOAL_FUNCARG5_REG].UQ;
		reg_data[6] = cpuRegs.GPR.r[GOAL_FUNCARG6_REG].UQ;
		reg_data[7] = cpuRegs.GPR.r[GOAL_FUNCARG7_REG].UQ;
	};
};

struct GOALMethodOrFunction
{
	std::vector<CallSignature> unique_signatures;
};


class GOALMethodInspector
{
public:
	GOALMethodInspector();
	~GOALMethodInspector();

	std::stack<CallSignature> m_func_call_stack;

	std::unordered_map<std::string, GOALMethodOrFunction> m_function_calls;
	std::unordered_map<std::string, std::unordered_map<int, GOALMethodOrFunction>> m_type_methods;
	std::mutex m_lock;

	// eliminates stutter on file write, not worried about accuracy / race conditions
	// anything missed will be caught the next time (append only maps above!)
	std::thread m_file_writer_thread;

	// jak 2 for some reason needs +1 when resolving the symbol->string for functions
	u32 m_function_name_addr_offset = 0;
	u32 m_symbol_table_offset = 0xff38;
	int m_symbol_width = 8;

	std::vector<std::string> goalMethodNames = {
		"new",
		"delete",
		"print",
		"inspect",
		"length",
		"asize-of",
		"copy",
		"relocate",
		"memusage"};

	uint64_t unique_method_calls = 0;
	uint64_t unique_function_calls = 0;

	void pushCall(const CallSignature& func);
	void popCall();
	CallSignature peekCall();
	bool callStackEmpty();

	void analyzeCall(CallSignature& func, GOALMethodOrFunction& associated_type);

	void writeAllInfo();
};

extern GOALMethodInspector g_goal_method_inspector;
