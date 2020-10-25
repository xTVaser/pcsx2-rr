
#include "PrecompiledHeader.h"

#include "Debug.h"

bool SysTraceLog_EE_Disasm::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.EE.m_EnableDisasm;
}

bool SysTraceLog_EE_Registers::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.EE.m_EnableRegisters;
}

bool SysTraceLog_EE_Events::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.EE.m_EnableEvents;
}

bool SysTraceLog_IOP_Disasm::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.IOP.m_EnableDisasm;
}

bool SysTraceLog_IOP_Registers::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.IOP.m_EnableRegisters;
}

bool SysTraceLog_IOP_Events::IsActive() const
{
	return _parent::IsActive() && g_Conf->emulator->Trace.IOP.m_EnableEvents;
}