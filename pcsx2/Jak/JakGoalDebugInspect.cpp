#include "PrecompiledHeader.h"
#include "JakGoalDebugInspect.h"
#include "Memory.h"
#include "Counters.h"
#include <cstdio>
#include <common/FileSystem.h>
#include "fmt/core.h"
#include "fmt/ranges.h"
#include <pcsx2/Config.h>

std::string goalGetSymName(u16 sym)
{
	const u32 addr = cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0] + (s16)sym;
	auto name_addr = memRead32(addr + 0xff38);
	if (name_addr == 0)
	{
		return "";
	}
	auto name = (const char*)memGetPtr(name_addr + 4);
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

u16 goalGetSymFromPtr(u32 ptr) noexcept(true)
{
	return (u16)(ptr - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
}

std::string goalGetSymNameFromPtr(u32 ptr)
{
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

void GOALMethodInspector::pushFunc(GoalFuncInfo* func)
{
	m_func_stack.push(func);
}

void GOALMethodInspector::popFunc()
{
	m_func_stack.pop();
}

GoalFuncInfo* GOALMethodInspector::peekFunc()
{
	return m_func_stack.top();
}

bool GOALMethodInspector::funcStackEmpty()
{
	return m_func_stack.empty();
}

void GOALMethodInspector::analyzeFunc(GoalFuncInfo* func)
{
	func->arg_count_estimate = 0;
	func->reg_changed = 0;
	func->arg_data[0] = cpuRegs.GPR.r[GOAL_FUNCARG0_REG].UQ;
	func->arg_data[1] = cpuRegs.GPR.r[GOAL_FUNCARG1_REG].UQ;
	func->arg_data[2] = cpuRegs.GPR.r[GOAL_FUNCARG2_REG].UQ;
	func->arg_data[3] = cpuRegs.GPR.r[GOAL_FUNCARG3_REG].UQ;
	func->arg_data[4] = cpuRegs.GPR.r[GOAL_FUNCARG4_REG].UQ;
	func->arg_data[5] = cpuRegs.GPR.r[GOAL_FUNCARG5_REG].UQ;
	func->arg_data[6] = cpuRegs.GPR.r[GOAL_FUNCARG6_REG].UQ;
	func->arg_data[7] = cpuRegs.GPR.r[GOAL_FUNCARG7_REG].UQ;

	for (int i = 0; i < 8; ++i)
	{
		if (func->arg_data[i] != func->reg_data[i])
		{
			func->reg_changed |= 1 << i;
			if (i + 1 > func->arg_count_estimate)
				func->arg_count_estimate = i + 1;
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
		func->arg_types[i] = "(not-basic)";
		func->arg_is_symbol[i] = false;
		func->arg_is_value[i] = true;
		if (func->arg_data[i].hi == 0)
		{
			u64 ref = i == 0 ? func->arg_data[i]._u32[0] : func->arg_data[i].lo;
			if (ref >= symTableFull && ref < symTableEnd)
			{
				auto symbol_name = goalGetSymName(ref - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
				func->arg_is_symbol[i] = true;
				func->arg_is_value[i] = false;
				func->arg_types[i] = symbol_name;
			}
			else if ((ref & 0b111) == 4 && ref >= symTableFull && ref < 0x2000000)
			{
				u32 type_ptr = memRead32(ref - 4);
				if (typeLookupFunc(type_ptr))
				{
					func->arg_types[i] = goalGetSymNameFromPtr(type_ptr);
				}
				func->arg_is_value[i] = false;
			}
		}
	}
}

void GOALMethodInspector::validateTypeMethods(std::string name)
{
	if (m_type_methods.find(name) == m_type_methods.end())
	{
		m_type_methods.insert_or_assign(name, new std::array<GoalFuncInfo*, 512>{nullptr});
	}
}

GoalFuncInfo* GOALMethodInspector::getTypeMethod(std::string name, u32 m)
{
	return m_type_methods.at(name)->at(m);
}

void GOALMethodInspector::setTypeMethod(std::string name, u32 m, GoalFuncInfo* func)
{
	m_type_methods.at(name)->at(m) = func;
}

void GOALMethodInspector::writeAllInfo()
{
	auto exec_path = EmuFolders::AppRoot;
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
	fclose(file);
}
