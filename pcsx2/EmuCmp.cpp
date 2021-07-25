/*  PCSX2 - PS2 Emulator for PCs
 *  Copyright (C) 2020  PCSX2 Dev Team
 *
 *  PCSX2 is free software: you can redistribute it and/or modify it under the terms
 *  of the GNU Lesser General Public License as published by the Free Software Found-
 *  ation, either version 3 of the License, or (at your option) any later version.
 *
 *  PCSX2 is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 *  without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 *  PURPOSE.  See the GNU General Public License for more details.
 *
 *  You should have received a copy of the GNU General Public License along with PCSX2.
 *  If not, see <http://www.gnu.org/licenses/>.
 */

#include "PrecompiledHeader.h"
#include "EmuCmp.h"
#include "R5900.h"
#include "VU.h"
#include "DebugTools/Debug.h"

#ifdef __linux
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/tcp.h>
#include <unistd.h>
#elif _WIN32
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include <WS2tcpip.h>
#endif
#include <errno.h>
#include <stdio.h>
#include <string.h>

#ifdef __linux
const int TCP_SOCKET_LEVEL = SOL_TCP;
#elif _WIN32
const int TCP_SOCKET_LEVEL = IPPROTO_TCP;
#endif

int open_socket(int af, int type, int protocol)
{
#ifdef __linux
	return socket(af, type, protocol);
#elif _WIN32
	WSADATA wsaData = {0};
	int iResult = 0;
	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0)
	{
		printf("WSAStartup failed: %d\n", iResult);
		return 1;
	}
	return socket(af, type, protocol);
#endif
}

void close_socket(int sock)
{
	if (sock < 0)
	{
		return;
	}
#ifdef __linux
	close(sock);
#elif _WIN32
	closesocket(sock);
	WSACleanup();
#endif
}

int set_socket_option(int socket, int level, int optname, const void* optval, int optlen)
{
	int ret = setsockopt(socket, level, optname, (const char*)optval, optlen);
	if (ret < 0)
	{
		printf("Failed to setsockopt(%d, %d, %d, _, _) - Error: %s\n", socket, level, optname,
			strerror(errno));
	}
#ifdef _WIN32
	if (ret < 0)
	{
		int err = WSAGetLastError();
		printf("WSAGetLastError: %d\n", err);
	}
#endif
	return ret;
}

int set_socket_timeout(int socket, long microSeconds)
{
#ifdef __linux
	struct timeval timeout = {};
	timeout.tv_sec = 0;
	timeout.tv_usec = microSeconds;
	int ret = setsockopt(socket, SOL_SOCKET, SO_RCVTIMEO, (struct timeval*)&timeout, sizeof(timeout));
	if (ret < 0)
	{
		printf("Failed to setsockopt(%d, %d, %d, _, _) - Error: %s\n", socket, SOL_SOCKET, SO_RCVTIMEO,
			strerror(errno));
	}
	return ret;
#elif _WIN32
	DWORD timeout = microSeconds / 1000; // milliseconds
	return set_socket_option(socket, SOL_SOCKET, SO_RCVTIMEO, &timeout, sizeof(timeout));
#endif
}

int write_to_socket(int socket, const char* buf, int len)
{
#ifdef __linux
	return write(socket, buf, len);
#elif _WIN32
	return send(socket, buf, len, 0);
#endif
}

int read_from_socket(int socket, char* buf, int len)
{
#ifdef __linux
	return read(socket, buf, len);
#elif _WIN32
	return recv(socket, buf, len, 0);
#endif
}

using namespace EmuCmp;

Config::Mode EmuCmp::mode = Config::Mode::Server;

// Note: communications are one-way so we can use buffered IO

static FILE* comms = nullptr;

void EmuCmp::init()
{
	if (comms)
	{
		return;
	} // Already initted

	mode = Config::Mode::Off;
	if (EmuConfig.Debugger.EmuCmpHost.IsEmpty())
	{
		DevCon.Error("EmuCmp: EmuCmpHost in .ini file is blank!");
		return;
	}
	wxString host;
	Config::Mode attemptMode;
	auto port = EmuConfig.Debugger.EmuCmpHost.AfterLast(':');
	if (port == EmuConfig.Debugger.EmuCmpHost)
	{
		attemptMode = Config::Mode::Server;
	}
	else
	{
		host = EmuConfig.Debugger.EmuCmpHost.BeforeLast(':');
		attemptMode = Config::Mode::Client;
	}
	long portnum;
	if (!port.ToLong(&portnum) && portnum == (u16)portnum)
	{
		DevCon.Error("EmuCmp: Invalid port %s", WX_STR(port));
		return;
	}
	DevCon.WriteLn("EmuCmp: Port Selected: %d", portnum);

	if (attemptMode == Config::Mode::Server)
	{
		cmp_socket = open_socket(AF_INET, SOCK_STREAM, 0);
		if (cmp_socket < 0)
		{
			cmp_socket = -1;
		}

#ifdef __linux
		int server_socket_opt = SO_REUSEADDR | SO_REUSEPORT;
#elif _WIN32
		int server_socket_opt = SO_EXCLUSIVEADDRUSE;
#endif

		int opt = 1;
		if (set_socket_option(cmp_socket, SOL_SOCKET, server_socket_opt, &opt, sizeof(opt)) < 0)
		{
			return;
		};

		if (set_socket_option(cmp_socket, TCP_SOCKET_LEVEL, TCP_NODELAY, &opt, sizeof(opt)) < 0)
		{
			return;
		}

		if (set_socket_timeout(cmp_socket, 100000) < 0)
		{
			return;
		}

		sockaddr_in server_address = {};
		server_address.sin_family = AF_INET;
		server_address.sin_addr.s_addr = INADDR_ANY;
		server_address.sin_port = htons(portnum);

		if (bind(cmp_socket, (sockaddr*)&server_address, sizeof(server_address)) < 0)
		{
			printf("[EmuCmp] Failed to bind\n");
			return;
		}

		if (listen(cmp_socket, 0) < 0)
		{
			printf("[EmuCmp] Failed to listen\n");
			return;
		}
	}
	else
	{
		// construct socket
		cmp_socket = open_socket(AF_INET, SOCK_STREAM, 0);
		if (cmp_socket < 0)
		{
			DevCon.WriteLn("[EmuCMP] Failed to create socket.");
			return;
		}

		if (set_socket_timeout(cmp_socket, 500000) < 0)
		{
			close_socket(cmp_socket);
			return;
		}

		// set nodelay, which makes small rapid messages faster, but large messages slower
		int one = 1;
		if (set_socket_option(cmp_socket, TCP_SOCKET_LEVEL, TCP_NODELAY, &one, sizeof(one)))
		{
			close_socket(cmp_socket);
			return;
		}

		// setup address
		sockaddr_in server_address = {};
		server_address.sin_family = AF_INET;
		server_address.sin_port = htons(portnum);
		if (inet_pton(AF_INET, "127.0.0.1", &server_address.sin_addr) <= 0)
		{
			DevCon.WriteLn("[EmuCMP] Invalid IP address.");
			close_socket(cmp_socket);
		}

		// connect!
		int rv, i;
		for (i = 0; i < 100; i++)
		{
			rv = connect(cmp_socket, (sockaddr*)&server_address, sizeof(server_address));
			if (rv >= 0)
			{
				break;
			}
			std::this_thread::sleep_for(std::chrono::microseconds(100000));
		}
		if (rv < 0)
		{
			DevCon.WriteLn("[EmuCmp] Failed to connect");
			close_socket(cmp_socket);
		}
		else
		{
			DevCon.WriteLn("[EmuCmp] Socket connected established! (took %d tries).", i);
		}
	}
}

void EmuCmp::shutdown()
{
	if (comms)
	{
		fclose(comms);
		comms = nullptr;
	}
	mode = Config::Mode::Off;
}

static void send(void* buffer, std::size_t size)
{
	if (mode != Config::Mode::Server)
	{
		return;
	}
	write_to_socket(cmp_socket, static_cast<char*>(buffer), size);
}

static void receive(void* buffer, std::size_t size)
{
	if (mode != Config::Mode::Client)
	{
		return;
	}
	read_from_socket(cmp_socket, static_cast<char*>(buffer), size);
}

template <typename T, typename = typename std::enable_if<std::is_trivial<T>::value>::type>
void send(const T& item)
{
	send(&item, sizeof(T));
}

template <typename T, typename = typename std::enable_if<std::is_trivial<T>::value>::type>
T receive()
{
	T out;
	receive(&out, sizeof(T));
	return out;
}

void EmuCmp::detail::syncData(void* data, std::size_t size)
{
	if (mode == Config::Mode::Server)
	{
		send(data, size);
	}
	else if (mode == Config::Mode::Client)
	{
		receive(data, size);
	}
}

#include <sstream>

template <typename T>
static void Compare(const T& server, T& client, const char* name, bool& found, u32 pc)
{
	if (server != client)
	{
		std::stringstream out;
		out << "Server client mismatch of " << name << " at " << std::hex << pc << " (server was " << std::hex << server << " but client was " << std::hex << client << ")";
		pxAssertRel(0, out.str().c_str());
		if (Config::corrections)
		{
			client = server;
		}
		found = true;
	}
}

static void Compare(const GPR_reg& server, GPR_reg& client, const char* name, bool& found, u32 pc)
{
	Compare(server.SD[0], client.SD[0], name, found, pc);
	char hiname[16];
	sprintf(hiname, "%s High", name);
	Compare(server.SD[1], client.SD[1], hiname, found, pc);
}

static void Compare(const VECTOR& server, VECTOR& client, const char* name, bool& found, u32 pc)
{
	char name_[256];
	for (int i = 0; i < 4; i++)
	{
		sprintf(name_, "%s[%d]", name, i);
		Compare(server.UL[i], client.UL[i], name_, found, pc);
	}
}

static void PrintMismatchUnknown(const char* name, u32 pc)
{
	std::stringstream out;
	out << "Server client mismatch of unknown " << name << " at " << std::hex << pc;
	pxAssertRel(0, out.str().c_str());
}

/// If you're having trouble with load instructions loading incorrect data from memory, use this to compare the section of memory containing the incorrect data during the appropriate cmpR#### routine to catch the bad store in action!
static void CompareBuffer(void* buffer, int startOffset, int length, const char* name, u32 pc)
{
	u8* local = (u8*)buffer + startOffset;
	if (mode == Config::Mode::Server)
	{
		send(local, length);
	}
	else if (mode == Config::Mode::Client)
	{
		const int maxStackBuffer = _64kb;
		u8* srv = length > maxStackBuffer ? (u8*)malloc(length) : (u8*)alloca(length);
		receive(srv, length);

		if (0 != memcmp(srv, local, length))
		{
			bool ignore;
			if (length % 4 == 0)
			{ // Compare ints
				u32* srv32 = (u32*)srv;
				u32* local32 = (u32*)local;
				for (int i = 0; i < length / 4; i++)
				{
					char nm[128];
					sprintf(nm, "%s[0x%x]", name, i * 4 + startOffset);
					Compare(srv32[i], local32[i], nm, ignore, pc);
				}
			}
			else
			{
				for (int i = 0; i < length; i++)
				{
					char nm[128];
					sprintf(nm, "%s[0x%x]", name, i + startOffset);
					Compare(srv[i], local[i], nm, ignore, pc);
				}
			}
		}

		if (length > maxStackBuffer)
		{
			free(srv);
		}
	}
}

#define COMPARE(reg) Compare(servCPU.reg, cpuRegs.reg, #reg, found, pc)

void __fastcall EmuCmp::cmpR5900(u32 pc)
{
	static u32 lastPC = 0;
	// Example use of CompareBuffer to find bad memory around 0x70001000
	// CompareBuffer(eeMem->Scratch, 0x1000, 0x500, "Scratch", lastPC);
	if (mode == Config::Mode::Server)
	{
		send(cpuRegs);
		send(fpuRegs);
	}
	else if (mode == Config::Mode::Client)
	{
		const cpuRegisters servCPU = receive<cpuRegisters>();
		const fpuRegisters servFPU = receive<fpuRegisters>();
		if (0 != memcmp(&servCPU, &cpuRegs, sizeof(cpuRegisters)))
		{
			bool found = false;
			for (int i = 0; i < 32; i++)
			{
				Compare(servCPU.GPR.r[i], cpuRegs.GPR.r[i], R5900::GPR_REG[i], found, lastPC);
				Compare(servCPU.CP0.r[i], cpuRegs.CP0.r[i], R5900::COP0_REG[i], found, lastPC);
				Compare(servCPU.eCycle[i], cpuRegs.eCycle[i], "eCycle", found, lastPC);
				Compare(servCPU.sCycle[i], cpuRegs.sCycle[i], "sCycle", found, lastPC);
			}
			COMPARE(HI);
			COMPARE(LO);
			COMPARE(sa);
			COMPARE(IsDelaySlot);
			COMPARE(pc);
			COMPARE(cycle);
			COMPARE(interrupt);
			COMPARE(branch);
			COMPARE(opmode);
			COMPARE(tempcycles);
			Compare(servCPU.PERF.n.pccr.val, cpuRegs.PERF.n.pccr.val, "pccr", found, lastPC);
			Compare(servCPU.PERF.n.pad, cpuRegs.PERF.n.pad, "pccr", found, lastPC);
			Compare(servCPU.PERF.n.pcr0, cpuRegs.PERF.n.pcr0, "pcr0", found, lastPC);
			Compare(servCPU.PERF.n.pcr1, cpuRegs.PERF.n.pcr1, "pcr1", found, lastPC);
			if (!found)
			{
				PrintMismatchUnknown("CPU register", lastPC);
			}
		}
		if (0 != memcmp(&servFPU, &fpuRegs, sizeof(fpuRegisters)))
		{
			bool found = false;
			for (int i = 0; i < 32; i++)
			{
				Compare(servFPU.fpr[i].f, fpuRegs.fpr[i].f, R5900::COP1_REG_FP[i], found, lastPC);
				Compare(servFPU.fprc[i], fpuRegs.fprc[i], R5900::COP1_REG_FCR[i], found, lastPC);
			}
			Compare(servFPU.ACC.f, fpuRegs.ACC.f, "FP ACC", found, lastPC);
			if (!found)
			{
				PrintMismatchUnknown("FPU register", lastPC);
			}
		}
	}
	lastPC = pc;
}

void __fastcall EmuCmp::cmpVU(u32 idx, u32 pc)
{
	if (!pxAssert(idx == 0 || idx == 1))
	{
		return;
	}
	static u32 lastPC[2] = {0};
	VURegs& regs = vuRegs[idx];
	if (mode == Config::Mode::Server)
	{
		send(regs.VF);
		send(regs.VI);
		send(regs.ACC);
		send(regs.q);
		send(regs.p);
	}
	else if (mode == Config::Mode::Client)
	{
		VURegs srv;
		receive(&srv.VF, sizeof(srv.VF));
		receive(&srv.VI, sizeof(srv.VI));
		receive(&srv.ACC, sizeof(srv.ACC));
		receive(&srv.q, sizeof(srv.q));
		receive(&srv.p, sizeof(srv.p));

		if (0 != memcmp(&regs, &srv, offsetof(VURegs, idx)))
		{
			bool found = false;
			char name[16] = "VUX.";
			name[2] = idx ? '1' : '0';
			for (int i = 0; i < 32; i++)
			{
				sprintf(name + 4, "VI%d", i);
				Compare(srv.VI[i].UL, regs.VI[i].UL, name, found, lastPC[idx]);
				name[5] = 'F';
				Compare(srv.VF[i], regs.VF[i], name, found, lastPC[idx]);
			}
			sprintf(name + 4, "ACC");
			Compare(srv.ACC, regs.ACC, name, found, lastPC[idx]);
			sprintf(name + 4, "q");
			Compare(srv.q.UL, regs.q.UL, name, found, lastPC[idx]);
			sprintf(name + 4, "p");
			Compare(srv.p.UL, regs.p.UL, name, found, lastPC[idx]);

			if (!found)
			{
				PrintMismatchUnknown(idx ? "VU1 Register" : "VU0 Register", lastPC[idx]);
			}
		}
	}
	lastPC[idx] = pc;
}

void EmuCmp::detail::cmpMem(void* mem, int length, const char* description)
{
	if (mode == Config::Mode::Server)
	{
		send(mem, length);
	}
	else
	{
		const int maxStackBuffer = _64kb;
		u8* srv = length > maxStackBuffer ? (u8*)malloc(length) : (u8*)alloca(length);

		if (0 != memcmp(srv, mem, length))
		{
			char err[1024];
			sprintf(err, "Mismatch in %s", description);
			pxAssertRel(0, err);
		}

		if (length > maxStackBuffer)
		{
			free(srv);
		}
	}
}

void EmuCmp::detail::verifySync(u16 syncID)
{
	u32 val = 0xaaaa0000 | syncID;
	syncValue(val);
	pxAssertRel(val == 0xaaaa0000 | syncID, "Emulators desynced!");
}
