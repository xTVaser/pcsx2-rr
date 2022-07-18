#include "PrecompiledHeader.h"
#include "JakGoalDebugInspect.h"
#include "Memory.h"
#include "Counters.h"
#include <cstdio>
#include <common/FileSystem.h>
#include "fmt/core.h"
#include "fmt/ranges.h"
#include <pcsx2/Config.h>

/*
In Jak 1, the "true" symbol was located at s7 + 8, but in Jak 2 it's s7 + 4. 
So there's some hard-coded 8's to clean up in places.  
The s7 register always points to #f, and #t is always the symbol after that. 

In Jak 2, symbols are only 4 bytes, so the offset is 4 now.
*/

/*
Jak 1
---- SYMBOL TABLE
| ...
| s7 - Address to the middle of the table
| ...
----
---- SYMBOL NAME TABLE (with no additional symbols = s7 + 0xFF38)
| ...
|------ Entry (ie. profile-bar) = BASE_OF_TABLE + Symbol Index [ie. for a method call ((t9 - (MethodId * 16)) - s7)]
| 4 bytes - string length
| 4 bytes - string value ptr
|------
| ...
|---- 147d24
*/
std::string goalGetSymName(u16 sym)
{
	// TODO - use different offset for jak 2
	const u32 symbol_name_table_addr = cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0] + 0xff38;
	auto symbol_name_addr = memRead32(symbol_name_table_addr + (s16)sym);
	// TODO - this might be skipping some stuff
	if (symbol_name_addr == 0 || symbol_name_addr > 0x7A12000) // out of bounds!
	{
		return "";
	}
	auto name = (const char*)memGetPtr(symbol_name_addr + 4);
	if (name == nullptr)
	{
		return "";
	}
	// Detect bad data
	if (name[0] < 33 || name[0] > 126)
	{
		return "";
	}
	return std::string(name);
}

std::string goalGetSymNameDirectly(u32 ptr)
{
	if (ptr > 0x7A12000)
	{ // beyond 128mb!
		return "";
	}
	// TODO - use different offset for jak 2
	const u32 symbol_name_table_addr = ptr + 0xff38;
	auto symbol_name_addr = memRead32(symbol_name_table_addr);
	// TODO - this might be skipping some stuff
	if (symbol_name_addr == 0 || symbol_name_addr > 0x7A12000) // out of bounds!
	{
		return "";
	}
	auto name = (const char*)memGetPtr(symbol_name_addr + 4);
	if (name == nullptr)
	{
		return "";
	}
	// Detect bad data
	if (name[0] < 33 || name[0] > 126)
	{
		return "";
	}
	return std::string(name);
}

u16 goalGetSymFromPtr(u32 ptr)
{
	return (u16)(ptr - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
}

std::string goalGetSymNameFromPtr(u32 ptr)
{
	if (ptr > 0x7A12000)
	{ // beyond 128mb!
		return "";
	}
	return goalGetSymName(goalGetSymFromPtr(memRead32(ptr)));
}

uptr goalGetSymValFromName(const char* name)
{
	uptr symTableFull = cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0] - 0x8000;

	for (auto s = symTableFull; s < symTableFull + 0xff38; s += 8)
	{
		u32 sym_name_ptr = memRead32(s + 0xff38);
		if (sym_name_ptr == 0)
			continue;
		const char* sym_name = (const char*)memGetPtr(sym_name_ptr);
		if (!strncmp(sym_name, name, strlen(name) + 1))
		{
			return memGetPtr(memRead32(s));
		}
	}
	return NULL;
}

GOALMethodInspector g_goal_method_inspector;

GOALMethodInspector::GOALMethodInspector()
{
}

void GOALMethodInspector::pushCall(CallSignature* call)
{
	m_func_call_stack.push(call);
}

void GOALMethodInspector::popCall()
{
	m_func_call_stack.pop();
}

CallSignature* GOALMethodInspector::peekCall()
{
	return m_func_call_stack.top();
}

bool GOALMethodInspector::callStackEmpty()
{
	return m_func_call_stack.empty();
}

// TODO - can probably speed up a lot of this by initializing a map of all symbols
// and their addresses, instead of doing a lookup everytime we find a call

void GOALMethodInspector::analyzeCall(CallSignature* call, GOALMethodOrFunction* associated_type)
{
	call->arg_count_estimate = 0;
	call->reg_changed = 0;
	call->arg_data[0] = cpuRegs.GPR.r[GOAL_FUNCARG0_REG].UQ;
	call->arg_data[1] = cpuRegs.GPR.r[GOAL_FUNCARG1_REG].UQ;
	call->arg_data[2] = cpuRegs.GPR.r[GOAL_FUNCARG2_REG].UQ;
	call->arg_data[3] = cpuRegs.GPR.r[GOAL_FUNCARG3_REG].UQ;
	call->arg_data[4] = cpuRegs.GPR.r[GOAL_FUNCARG4_REG].UQ;
	call->arg_data[5] = cpuRegs.GPR.r[GOAL_FUNCARG5_REG].UQ;
	call->arg_data[6] = cpuRegs.GPR.r[GOAL_FUNCARG6_REG].UQ;
	call->arg_data[7] = cpuRegs.GPR.r[GOAL_FUNCARG7_REG].UQ;

	for (int i = 0; i < 8; ++i)
	{
		if (call->arg_data[i] != call->reg_data[i])
		{
			call->reg_changed |= 1 << i;
			if (i + 1 > call->arg_count_estimate)
			{
				call->arg_count_estimate = i + 1;
			}
		}
	}

	uptr symTableFull = cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0] - 0x8000;
	uptr symTableEnd = symTableFull + 0xff38;
	auto typeLookupFunc = [symTableFull](u32 ptr) {
		if ((ptr & 0b111) != 4)
			return false;
		uptr s = symTableFull;
		while (s < symTableFull + 0xff38)
		{
			if (memRead32(s) == ptr)
				return true;
			s += 8;
		}
		return false;
	};

	for (int i = 0; i < 8; ++i)
	{
		call->arg_types[i] = "(not-basic)";
		call->arg_is_symbol[i] = false;
		call->arg_is_value[i] = true;
		if (call->arg_data[i].hi == 0)
		{
			u64 ref = i == 0 ? call->arg_data[i]._u32[0] : call->arg_data[i].lo;
			if (ref >= symTableFull && ref < symTableEnd)
			{
				auto symbol_name = goalGetSymName(ref - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
				if (!symbol_name.empty())
				{
					call->arg_is_symbol[i] = true;
					call->arg_is_value[i] = false;
					call->arg_types[i] = symbol_name;
				}
			}
			else if ((ref & 0b111) == 4 && ref >= symTableFull && ref < 0x2000000)
			{
				u32 type_ptr = memRead32(ref - 4);
				if (typeLookupFunc(type_ptr))
				{
					auto symbol_name = goalGetSymNameFromPtr(type_ptr);
					if (!symbol_name.empty())
					{
						call->arg_types[i] = symbol_name;
					}
				}
				call->arg_is_value[i] = false;
			}
		}
	}
	// Check to see if we've already found this signature before
	for (const auto& unique_sig : associated_type->unique_signatures)
	{
		bool sameSig = true;
		for (int i = 0; i < 8; i++)
		{
			if (call->arg_types[i] != unique_sig->arg_types[i])
			{
				sameSig = false;
				break;
			}
		}
		if (sameSig)
		{
			return;
		}
	}

	associated_type->unique_signatures.push_back(call);
	if (call->call_type == CallType::METHOD)
	{
		unique_method_calls++;
		if (call->method_id < 9)
		{
			auto msg = fmt::format("JAK: method {} registered for '{}'", goalMethodNames[call->method_id], call->type_or_func_name);
			Console.WriteLn(msg);
		}
		else
		{
			auto msg = fmt::format("JAK: method {} registered for '{}'", call->method_id, call->type_or_func_name);
			Console.WriteLn(msg);
		}
	}
	else
	{
		unique_function_calls++;
		auto msg = fmt::format("JAK: function '{}' registered", call->type_or_func_name);
		Console.WriteLn(msg);
	}
}

void GOALMethodInspector::writeAllInfo()
{
	/*auto exec_path = EmuFolders::AppRoot;
	auto file = FileSystem::OpenCFile(fmt::format("{}/goal_method_info.json", exec_path).c_str(), "a");
	if (file == NULL)
	{
		return;
	}
	for (auto& type : m_type_methods)
	{
		for (int i = 0; i < 512; i++)
		{
			auto func = type.second->at(i);
			if (func != nullptr)
			{
				std::vector<std::string> arg_types;
				for (int arg = 0; arg < 8; arg++)
				{
					if (func->arg_is_symbol[arg])
					{
						arg_types.push_back(fmt::format("\"'{}\"", func->arg_types[arg]));
					}
					else if (!func->arg_is_value[arg])
					{
						arg_types.push_back(fmt::format("\"{}\"", func->arg_types[arg]));
					}
					else
					{
						if (func->arg_data[arg].hi == 0)
						{
							arg_types.push_back(fmt::format("\"#x{:x}\"", func->arg_data[arg].lo));
						}
						else
						{
							arg_types.push_back(fmt::format("\"#x{:x}x{:x}x\"", func->arg_data[arg].hi, func->arg_data[arg].lo));
						}
					}
				}
				std::string entry = fmt::format("{{\"type_name\": \"{}\", \"method_id\": {}, \"arg_estimate\": {}, \"arg_types\": [{}]}},\n",
					type.first, i, func->arg_count_estimate, fmt::join(arg_types, ","));
				fprintf(file, entry.c_str());
			}
		}
	}
	fclose(file);*/
}
