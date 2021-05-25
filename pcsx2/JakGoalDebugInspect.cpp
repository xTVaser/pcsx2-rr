#include "PrecompiledHeader.h"
#include "JakGoalDebugInspect.h"
#include "Memory.h"
#include "Counters.h"
#include <cstdio>

const char* goalMethodNames[9] = {
	"new",
	"delete",
	"print",
	"inspect",
	"length",
	"asize-of",
	"copy",
	"relocate",
	"memusage"
};
bool goalWriteP = false;

namespace
{
	std::stack<GoalFuncInfo*> goalFuncStack;
	std::unordered_map<const char*, std::array<GoalFuncInfo*, 128>*> goalTypeMethods;
}

const char* goalGetSymName(u16 sym)
{
	const u32 addr = cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0] + (s16)sym;

	return (const char*)memGetPtr(memRead32(addr + 0xff38) + 4);
}

u16 goalGetSymFromPtr(u32 ptr) noexcept(true)
{
	return (u16)(ptr - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
}

const char* goalGetSymNameFromPtr(u32 ptr)
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
		if (!strncmp(sym_name, name, strlen(name)+1))
		{
			return memGetPtr(memRead32(s));
		}
	}
	return NULL;
}


#include "App.h"
#include <wx/stdpaths.h>
 
void goalWriteAllInfo()
{
	wxDirName dir = (wxDirName)wxFileName(wxStandardPaths::Get().GetExecutablePath()).GetPath();
	wxString file_name = "method_info.txt";
	wxString file_path = (dir + file_name).GetFullPath();
	FILE* file = fopen(file_path, "w");
	if (file == NULL)
		return;
	for (auto& type : goalTypeMethods)
	{
		fprintf(file, "[TYPE] %s\n", type.first);
		for (int i = 0; i < 128; ++i)
		{
			auto func = type.second->at(i);
			if (func != nullptr)
			{
				auto fprintf_arg_func = [func, file](int arg) {
					if (func->arg_is_symbol[arg])
					{
						fprintf(file, "'%s", func->arg_types[arg]);
					}
					else if (!func->arg_is_value[arg])
					{
						fprintf(file, "%s", func->arg_types[arg]);
					}
					else
					{
						if (func->arg_data[arg].hi == 0)
						{
							fprintf(file, "#x%x", func->arg_data[arg].lo);
						}
						else
						{
							fprintf(file, "#x%llx%llx", func->arg_data[arg].hi, func->arg_data[arg].lo);
						}
					}
				};
				fprintf(file, "  [METHOD %3d] args est: %d (", i, func->arg_count_estimate);
				fprintf_arg_func(0); fprintf(file, ", ");
				fprintf_arg_func(1); fprintf(file, ", ");
				fprintf_arg_func(2); fprintf(file, ", ");
				fprintf_arg_func(3); fprintf(file, ", ");
				fprintf_arg_func(4); fprintf(file, ", ");
				fprintf_arg_func(5); fprintf(file, ", ");
				fprintf_arg_func(6); fprintf(file, ", ");
				fprintf_arg_func(7); fprintf(file, ")\n");
			}
		}
	}
	fclose(file);
}

void goalPushFunc(GoalFuncInfo* func)
{
	goalFuncStack.push(func);
}

void goalPopFunc() noexcept(true)
{
	goalFuncStack.pop();
}

GoalFuncInfo* goalTopFunc() noexcept(true)
{
	return goalFuncStack.top();
}

bool goalNoFuncsP() noexcept(true)
{
	return goalFuncStack.empty();
}

void goalAnalyzeFunc(GoalFuncInfo* func)
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
			if (i+1 > func->arg_count_estimate)
				func->arg_count_estimate = i+1;
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
				const char* symbol_name = goalGetSymName(ref - cpuRegs.GPR.r[GOAL_SYMTABLE_REG].UL[0]);
				func->arg_is_symbol[i] = true;
				func->arg_is_value[i] = false;
				func->arg_types[i] = strdup(symbol_name);
			}
			else if ((ref & 0b111) == 4 && ref >= symTableFull && ref < 0x2000000)
			{
				u32 type_ptr = memRead32(ref-4);
				if (typeLookupFunc(type_ptr))
				{
					func->arg_types[i] = goalGetSymNameFromPtr(type_ptr);
				}
				func->arg_is_value[i] = false;
			}
		}
	}

	if (goalWriteP)
	{
		goalWriteAllInfo();
		goalWriteP = false;
	}
}

void goalValidateTypeMethods(const char* name)
{
	if (goalTypeMethods.find(name) == goalTypeMethods.end())
	{
		goalTypeMethods.insert_or_assign(name, new std::array<GoalFuncInfo*, 128>{nullptr});
	}
}

GoalFuncInfo* goalGetTypeMethod(const char* name, u32 m)
{
	return goalTypeMethods.at(name)->at(m);
}

void goalSetTypeMethod(const char* name, u32 m, GoalFuncInfo* func)
{
	goalTypeMethods.at(name)->at(m) = func;
}
