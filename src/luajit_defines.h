#pragma once

#define RAIDWW2

#if defined(RAIDWW2) && defined(BEFORE_RAID_15703546)
#define LUAJIT_DISABLE_FFI
#define LUAJIT_DISABLE_JIT
#define LUAJIT_ENABLE_GC64

#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"
#elif defined(RAIDWW2)
//#define LUAJIT_DISABLE_FFI
#define LUAJIT_ENABLE_GC64

#define LUA_VERSION	"Lua 5.1"
#define LUA_RELEASE	"Lua 5.1.4"
#define LUA_VERSION_NUM	501
#define LUA_COPYRIGHT	"Copyright (C) 1994-2008 Lua.org, PUC-Rio"
#define LUA_AUTHORS	"R. Ierusalimschy, L. H. de Figueiredo & W. Celes"
#endif

#ifdef LUAJIT_ENABLE_GC64
#define LJ_TARGET_GC64		1
#endif
#if defined(LUAJIT_DISABLE_FFI) || defined(LJ_ARCH_NOFFI)
#define LJ_HASFFI		0
#else
#define LJ_HASFFI		1
#endif
/* Disable or enable the JIT compiler. */
#if defined(LUAJIT_DISABLE_JIT) || defined(LJ_ARCH_NOJIT) || defined(LJ_OS_NOJIT)
#define LJ_HASJIT		0
#else
#define LJ_HASJIT		1
#endif
#if LJ_TARGET_GC64
#define LJ_GC64			1
#else
#define LJ_GC64			0
#endif

#if LJ_GC64
#define LJ_FR2			1
#else
#define LJ_FR2			0
#endif

/* Target endianess. */
#define LUAJIT_LE	0
#define LUAJIT_BE	1

/* Target architectures. */
#define LUAJIT_ARCH_X86		1
#define LUAJIT_ARCH_x86		1
#define LUAJIT_ARCH_X64		2
#define LUAJIT_ARCH_x64		2
#define LUAJIT_ARCH_ARM		3
#define LUAJIT_ARCH_arm		3
#define LUAJIT_ARCH_ARM64	4
#define LUAJIT_ARCH_arm64	4
#define LUAJIT_ARCH_PPC		5
#define LUAJIT_ARCH_ppc		5
#define LUAJIT_ARCH_MIPS	6
#define LUAJIT_ARCH_mips	6
#define LUAJIT_ARCH_MIPS32	6
#define LUAJIT_ARCH_mips32	6
#define LUAJIT_ARCH_MIPS64	7
#define LUAJIT_ARCH_mips64	7
#if defined(__i386) || defined(__i386__) || defined(_M_IX86)
#define LUAJIT_TARGET	LUAJIT_ARCH_X86
#elif defined(__x86_64__) || defined(__x86_64) || defined(_M_X64) || defined(_M_AMD64)
#define LUAJIT_TARGET	LUAJIT_ARCH_X64
#elif defined(__arm__) || defined(__arm) || defined(__ARM__) || defined(__ARM)
#define LUAJIT_TARGET	LUAJIT_ARCH_ARM
#elif defined(__aarch64__)
#define LUAJIT_TARGET	LUAJIT_ARCH_ARM64
#elif defined(__ppc__) || defined(__ppc) || defined(__PPC__) || defined(__PPC) || defined(__powerpc__) || defined(__powerpc) || defined(__POWERPC__) || defined(__POWERPC) || defined(_M_PPC)
#define LUAJIT_TARGET	LUAJIT_ARCH_PPC
#elif defined(__mips64__) || defined(__mips64) || defined(__MIPS64__) || defined(__MIPS64)
#define LUAJIT_TARGET	LUAJIT_ARCH_MIPS64
#elif defined(__mips__) || defined(__mips) || defined(__MIPS__) || defined(__MIPS)
#define LUAJIT_TARGET	LUAJIT_ARCH_MIPS32
#else
#error "No support for this architecture (yet)"
#endif

#if LUAJIT_TARGET == LUAJIT_ARCH_X86

#define LJ_ARCH_NAME		"x86"
#define LJ_ARCH_BITS		32
#define LJ_ARCH_ENDIAN		LUAJIT_LE
#if LJ_TARGET_WINDOWS || LJ_TARGET_CYGWIN
#define LJ_ABI_WIN		1
#else
#define LJ_ABI_WIN		0
#endif
#define LJ_TARGET_X86		1
#define LJ_TARGET_X86ORX64	1
#define LJ_TARGET_EHRETREG	0
#define LJ_TARGET_MASKSHIFT	1
#define LJ_TARGET_MASKROT	1
#define LJ_TARGET_UNALIGNED	1
#define LJ_ARCH_NUMMODE		LJ_NUMMODE_SINGLE_DUAL

#elif LUAJIT_TARGET == LUAJIT_ARCH_X64

#define LJ_ARCH_NAME		"x64"
#define LJ_ARCH_BITS		64
#define LJ_ARCH_ENDIAN		LUAJIT_LE
#if LJ_TARGET_WINDOWS || LJ_TARGET_CYGWIN
#define LJ_ABI_WIN		1
#else
#define LJ_ABI_WIN		0
#endif
#define LJ_TARGET_X64		1
#define LJ_TARGET_X86ORX64	1
#define LJ_TARGET_EHRETREG	0
#define LJ_TARGET_JUMPRANGE	31	/* +-2^31 = +-2GB */
#define LJ_TARGET_MASKSHIFT	1
#define LJ_TARGET_MASKROT	1
#define LJ_TARGET_UNALIGNED	1
#define LJ_ARCH_NUMMODE		LJ_NUMMODE_SINGLE_DUAL
#ifdef LUAJIT_ENABLE_GC64
#define LJ_TARGET_GC64		1
#endif

#endif

#define LJ_STACK_EXTRA	(5+3*LJ_FR2)
#define LUA_MINSTACK	20
#define LUAI_MAXSTACK	65500
#define LJ_STACK_MIN	LUA_MINSTACK	/* Min. stack size. */
#define LJ_STACK_MAX	LUAI_MAXSTACK	/* Max. stack size. */
#define LJ_STACK_START	(2*LJ_STACK_MIN)	/* Starting stack size. */
#define LJ_STACK_MAXEX	(LJ_STACK_MAX + 1 + LJ_STACK_EXTRA)

#define isluafunc(fn)	((fn)->c.ffid == FF_LUA)
#define iscfunc(fn)	((fn)->c.ffid == FF_C)
#define isffunc(fn)	((fn)->c.ffid > FF_C)
#if LJ_GC64
#define curr_func(L)		(&gcval(L->base-2)->fn)
#elif LJ_FR2
#define curr_func(L)		(&gcref((L->base-2)->gcr)->fn)
#else
#define curr_func(L)		(&gcref((L->base-1)->fr.func)->fn)
#endif
#define curr_funcisL(L)		(isluafunc(curr_func(L)))
#define curr_proto(L)		(funcproto(curr_func(L)))
#define curr_topL(L)		(L->base + curr_proto(L)->framesize)
#define curr_top(L)		(curr_funcisL(L) ? curr_topL(L) : L->top)

#if LJ_FR2

#define setframe_gc(f, p, tp)	(setgcVraw((f), (p), (tp)))
#else

#define setframe_gc(f, p, tp)	(setgcref((f)->fr.func, (p)), UNUSED(tp))
#endif