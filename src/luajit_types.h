#pragma once

#include "luajit_defines.h"

/* Types for handling bytecodes. Need this here, details in lj_bc.h. */
typedef uint32_t BCIns;  /* Bytecode instruction. */
typedef uint32_t BCPos;  /* Bytecode position. */
typedef uint32_t BCReg;  /* Bytecode register. */
typedef int32_t BCLine;  /* Bytecode line number. */


#if LJ_ARCH_ENDIAN == LUAJIT_BE
#define LJ_LE			0
#define LJ_BE			1
#define LJ_ENDIAN_SELECT(le, be)	be
#define LJ_ENDIAN_LOHI(lo, hi)		hi lo
#else
#define LJ_LE			1
#define LJ_BE			0
#define LJ_ENDIAN_SELECT(le, be)	le
#define LJ_ENDIAN_LOHI(lo, hi)		lo hi
#endif
#define LJ_NORET	__declspec(noreturn)
#define LJ_ALIGN(n)	__declspec(align(n))
#define LJ_INLINE	__inline
#define LJ_AINLINE	__forceinline
#define LJ_NOINLINE	__declspec(noinline)
/* Memory and GC object sizes. */
typedef uint32_t MSize;
#if LJ_GC64
typedef uint64_t GCSize;
#else
typedef uint32_t GCSize;
#endif
/* Memory reference */
typedef struct MRef {
#if LJ_GC64
  uint64_t ptr64;	/* True 64 bit pointer. */
#else
  uint32_t ptr32;	/* Pseudo 32 bit pointer. */
#endif
} MRef;
/* GCobj reference */
typedef struct GCRef {
#if LJ_GC64
  uint64_t gcptr64;	/* True 64 bit pointer. */
#else
  uint32_t gcptr32;	/* Pseudo 32 bit pointer. */
#endif
} GCRef;
/* Frame link. */
typedef union {
  int32_t ftsz;		/* Frame type and size of previous frame. */
  MRef pcr;		/* Or PC for Lua frames. */
} FrameLink;
/* Tagged value. */
typedef LJ_ALIGN(8) union TValue {
  uint64_t u64;		/* 64 bit pattern overlaps number. */
  lua_Number n;		/* Number object overlaps split tag/value object. */
#if LJ_GC64
  GCRef gcr;		/* GCobj reference with tag. */
  int64_t it64;
  struct {
    LJ_ENDIAN_LOHI(
      int32_t i;	/* Integer value. */
    , uint32_t it;	/* Internal object tag. Must overlap MSW of number. */
      )
  };
#else
  struct {
    LJ_ENDIAN_LOHI(
      union {
      GCRef gcr;	/* GCobj reference (if any). */
      int32_t i;	/* Integer value. */
    };
    , uint32_t it;	/* Internal object tag. Must overlap MSW of number. */
      )
  };
#endif
#if LJ_FR2
  int64_t ftsz;		/* Frame type and size of previous frame, or PC. */
#else
  struct {
    LJ_ENDIAN_LOHI(
      GCRef func;	/* Function for next frame (or dummy L). */
    , FrameLink tp;	/* Link to previous frame. */
      )
  } fr;
#endif
  struct {
    LJ_ENDIAN_LOHI(
      uint32_t lo;	/* Lower 32 bits of number. */
    , uint32_t hi;	/* Upper 32 bits of number. */
      )
  } u32;
} TValue;
#define GCHeader	GCRef nextgc; uint8_t marked; uint8_t gct

typedef const TValue cTValue;

#define LJ_TNIL			(~0u)
#define LJ_TFALSE		(~1u)
#define LJ_TTRUE		(~2u)
#define LJ_TLIGHTUD		(~3u)
#define LJ_TSTR			(~4u)
#define LJ_TUPVAL		(~5u)
#define LJ_TTHREAD		(~6u)
#define LJ_TPROTO		(~7u)
#define LJ_TFUNC		(~8u)
#define LJ_TTRACE		(~9u)
#define LJ_TCDATA		(~10u)
#define LJ_TTAB			(~11u)
#define LJ_TUDATA		(~12u)
/* This is just the canonical number type used in some places. */
#define LJ_TNUMX		(~13u)

#if LJ_64 && !LJ_GC64
#define LJ_TISNUM		0xfffeffffu
#else
#define LJ_TISNUM		LJ_TNUMX
#endif
#define LJ_TISTRUECOND		LJ_TFALSE
#define LJ_TISPRI		LJ_TTRUE
#define LJ_TISGCV		(LJ_TSTR+1)
#define LJ_TISTABUD		LJ_TTAB

#if LJ_GC64
#define LJ_GCVMASK		(((uint64_t)1 << 47) - 1)
#endif

#if LJ_GC64
#define itype(o)	((uint32_t)((o)->it64 >> 47))
#define tvisnil(o)	((o)->it64 == -1)
#else
#define itype(o)	((o)->it)
#define tvisnil(o)	(itype(o) == LJ_TNIL)
#endif
#define tvisfalse(o)	(itype(o) == LJ_TFALSE)
#define tvistrue(o)	(itype(o) == LJ_TTRUE)
#define tvisbool(o)	(tvisfalse(o) || tvistrue(o))
#if LJ_64 && !LJ_GC64
#define tvislightud(o)	(((int32_t)itype(o) >> 15) == -2)
#else
#define tvislightud(o)	(itype(o) == LJ_TLIGHTUD)
#endif
#define tvisstr(o)	(itype(o) == LJ_TSTR)
#define tvisfunc(o)	(itype(o) == LJ_TFUNC)
#define tvisthread(o)	(itype(o) == LJ_TTHREAD)
#define tvisproto(o)	(itype(o) == LJ_TPROTO)
#define tviscdata(o)	(itype(o) == LJ_TCDATA)
#define tvistab(o)	(itype(o) == LJ_TTAB)
#define tvisudata(o)	(itype(o) == LJ_TUDATA)
#define tvisnumber(o)	(itype(o) <= LJ_TISNUM)
#define tvisint(o)	(LJ_DUALNUM && itype(o) == LJ_TISNUM)
#define tvisnum(o)	(itype(o) < LJ_TISNUM)

#define BCDEF(_) \
  /* Comparison ops. ORDER OPR. */ \
  _(ISLT,	var,	___,	var,	lt) \
  _(ISGE,	var,	___,	var,	lt) \
  _(ISLE,	var,	___,	var,	le) \
  _(ISGT,	var,	___,	var,	le) \
  \
  _(ISEQV,	var,	___,	var,	eq) \
  _(ISNEV,	var,	___,	var,	eq) \
  _(ISEQS,	var,	___,	str,	eq) \
  _(ISNES,	var,	___,	str,	eq) \
  _(ISEQN,	var,	___,	num,	eq) \
  _(ISNEN,	var,	___,	num,	eq) \
  _(ISEQP,	var,	___,	pri,	eq) \
  _(ISNEP,	var,	___,	pri,	eq) \
  \
  /* Unary test and copy ops. */ \
  _(ISTC,	dst,	___,	var,	___) \
  _(ISFC,	dst,	___,	var,	___) \
  _(IST,	___,	___,	var,	___) \
  _(ISF,	___,	___,	var,	___) \
  _(ISTYPE,	var,	___,	lit,	___) \
  _(ISNUM,	var,	___,	lit,	___) \
  \
  /* Unary ops. */ \
  _(MOV,	dst,	___,	var,	___) \
  _(NOT,	dst,	___,	var,	___) \
  _(UNM,	dst,	___,	var,	unm) \
  _(LEN,	dst,	___,	var,	len) \
  \
  /* Binary ops. ORDER OPR. VV last, POW must be next. */ \
  _(ADDVN,	dst,	var,	num,	add) \
  _(SUBVN,	dst,	var,	num,	sub) \
  _(MULVN,	dst,	var,	num,	mul) \
  _(DIVVN,	dst,	var,	num,	div) \
  _(MODVN,	dst,	var,	num,	mod) \
  \
  _(ADDNV,	dst,	var,	num,	add) \
  _(SUBNV,	dst,	var,	num,	sub) \
  _(MULNV,	dst,	var,	num,	mul) \
  _(DIVNV,	dst,	var,	num,	div) \
  _(MODNV,	dst,	var,	num,	mod) \
  \
  _(ADDVV,	dst,	var,	var,	add) \
  _(SUBVV,	dst,	var,	var,	sub) \
  _(MULVV,	dst,	var,	var,	mul) \
  _(DIVVV,	dst,	var,	var,	div) \
  _(MODVV,	dst,	var,	var,	mod) \
  \
  _(POW,	dst,	var,	var,	pow) \
  _(CAT,	dst,	rbase,	rbase,	concat) \
  \
  /* Constant ops. */ \
  _(KSTR,	dst,	___,	str,	___) \
  _(KCDATA,	dst,	___,	cdata,	___) \
  _(KSHORT,	dst,	___,	lits,	___) \
  _(KNUM,	dst,	___,	num,	___) \
  _(KPRI,	dst,	___,	pri,	___) \
  _(KNIL,	base,	___,	base,	___) \
  \
  /* Upvalue and function ops. */ \
  _(UGET,	dst,	___,	uv,	___) \
  _(USETV,	uv,	___,	var,	___) \
  _(USETS,	uv,	___,	str,	___) \
  _(USETN,	uv,	___,	num,	___) \
  _(USETP,	uv,	___,	pri,	___) \
  _(UCLO,	rbase,	___,	jump,	___) \
  _(FNEW,	dst,	___,	func,	gc) \
  \
  /* Table ops. */ \
  _(TNEW,	dst,	___,	lit,	gc) \
  _(TDUP,	dst,	___,	tab,	gc) \
  _(GGET,	dst,	___,	str,	index) \
  _(GSET,	var,	___,	str,	newindex) \
  _(TGETV,	dst,	var,	var,	index) \
  _(TGETS,	dst,	var,	str,	index) \
  _(TGETB,	dst,	var,	lit,	index) \
  _(TGETR,	dst,	var,	var,	index) \
  _(TSETV,	var,	var,	var,	newindex) \
  _(TSETS,	var,	var,	str,	newindex) \
  _(TSETB,	var,	var,	lit,	newindex) \
  _(TSETM,	base,	___,	num,	newindex) \
  _(TSETR,	var,	var,	var,	newindex) \
  \
  /* Calls and vararg handling. T = tail call. */ \
  _(CALLM,	base,	lit,	lit,	call) \
  _(CALL,	base,	lit,	lit,	call) \
  _(CALLMT,	base,	___,	lit,	call) \
  _(CALLT,	base,	___,	lit,	call) \
  _(ITERC,	base,	lit,	lit,	call) \
  _(ITERN,	base,	lit,	lit,	call) \
  _(VARG,	base,	lit,	lit,	___) \
  _(ISNEXT,	base,	___,	jump,	___) \
  \
  /* Returns. */ \
  _(RETM,	base,	___,	lit,	___) \
  _(RET,	rbase,	___,	lit,	___) \
  _(RET0,	rbase,	___,	lit,	___) \
  _(RET1,	rbase,	___,	lit,	___) \
  \
  /* Loops and branches. I/J = interp/JIT, I/C/L = init/call/loop. */ \
  _(FORI,	base,	___,	jump,	___) \
  _(JFORI,	base,	___,	jump,	___) \
  \
  _(FORL,	base,	___,	jump,	___) \
  _(IFORL,	base,	___,	jump,	___) \
  _(JFORL,	base,	___,	lit,	___) \
  \
  _(ITERL,	base,	___,	jump,	___) \
  _(IITERL,	base,	___,	jump,	___) \
  _(JITERL,	base,	___,	lit,	___) \
  \
  _(LOOP,	rbase,	___,	jump,	___) \
  _(ILOOP,	rbase,	___,	jump,	___) \
  _(JLOOP,	rbase,	___,	lit,	___) \
  \
  _(JMP,	rbase,	___,	jump,	___) \
  \
  /* Function headers. I/J = interp/JIT, F/V/C = fixarg/vararg/C func. */ \
  _(FUNCF,	rbase,	___,	___,	___) \
  _(IFUNCF,	rbase,	___,	___,	___) \
  _(JFUNCF,	rbase,	___,	lit,	___) \
  _(FUNCV,	rbase,	___,	___,	___) \
  _(IFUNCV,	rbase,	___,	___,	___) \
  _(JFUNCV,	rbase,	___,	lit,	___) \
  _(FUNCC,	rbase,	___,	___,	___) \
  _(FUNCCW,	rbase,	___,	___,	___)

/* Bytecode opcode numbers. */
typedef enum {
#define BCENUM(name, ma, mb, mc, mt)	BC_##name,
  BCDEF(BCENUM)
#undef BCENUM
  BC__MAX
} BCOp;

typedef struct GChead {
  GCHeader;
  uint8_t unused1;
  uint8_t unused2;
  GCRef env;
  GCRef gclist;
  GCRef metatable;
} GChead;

typedef struct GCstr {
  GCHeader;
  uint8_t reserved;	/* Used by lexer for fast lookup of reserved words. */
  uint8_t unused;
  MSize hash;		/* Hash of string. */
  MSize len;		/* Size of string. */
} GCstr;

#define strref(r)	(&gcref((r))->str)
#define strdata(s)	((const char *)((s)+1))
#define strdatawr(s)	((char *)((s)+1))
#define strVdata(o)	strdata(strV(o))
#define sizestring(s)	(sizeof(struct GCstr)+(s)->len+1)

/* -- Userdata object ----------------------------------------------------- */

/* Userdata object. Payload follows. */
typedef struct GCudata {
  GCHeader;
  uint8_t udtype;	/* Userdata type. */
  uint8_t unused2;
  GCRef env;		/* Should be at same offset in GCfunc. */
  MSize len;		/* Size of payload. */
  GCRef metatable;	/* Must be at same offset in GCtab. */
  uint32_t align1;	/* To force 8 byte alignment of the payload. */
} GCudata;

/* Userdata types. */
enum {
  UDTYPE_USERDATA,	/* Regular userdata. */
  UDTYPE_IO_FILE,	/* I/O library FILE. */
  UDTYPE_FFI_CLIB,	/* FFI C library namespace. */
  UDTYPE__MAX
};

#define uddata(u)	((void *)((u)+1))
#define sizeudata(u)	(sizeof(struct GCudata)+(u)->len)

/* -- C data object ------------------------------------------------------- */

/* C data object. Payload follows. */
typedef struct GCcdata {
  GCHeader;
  uint16_t ctypeid;	/* C type ID. */
} GCcdata;


typedef struct GCupval {
  GCHeader;
  uint8_t closed;	/* Set if closed (i.e. uv->v == &uv->u.value). */
  uint8_t immutable;	/* Immutable value. */
  union {
    TValue tv;		/* If closed: the value itself. */
    struct {		/* If open: double linked list, anchored at thread. */
      GCRef prev;
      GCRef next;
    };
  };
  MRef v;		/* Points to stack slot (open) or above (closed). */
  uint32_t dhash;	/* Disambiguation hash: dh1 != dh2 => cannot alias. */
} GCupval;


/* -- Function object (closures) ------------------------------------------ */

/* Common header for functions. env should be at same offset in GCudata. */
#define GCfuncHeader \
  GCHeader; uint8_t ffid; uint8_t nupvalues; \
  GCRef env; GCRef gclist; MRef pc

typedef struct GCfuncC {
  GCfuncHeader;
  lua_CFunction f;	/* C function to be called. */
  TValue upvalue[1];	/* Array of upvalues (TValue). */
} GCfuncC;

typedef struct GCfuncL {
  GCfuncHeader;
  GCRef uvptr[1];	/* Array of _pointers_ to upvalue objects (GCupval). */
} GCfuncL;

typedef union GCfunc {
  GCfuncC c;
  GCfuncL l;
} GCfunc;

#define FF_LUA		0
#define FF_C		1
#define isluafunc(fn)	((fn)->c.ffid == FF_LUA)
#define iscfunc(fn)	((fn)->c.ffid == FF_C)
#define isffunc(fn)	((fn)->c.ffid > FF_C)
#define funcproto(fn) \
  check_exp(isluafunc(fn), (GCproto *)(mref((fn)->l.pc, char)-sizeof(GCproto)))
#define sizeCfunc(n)	(sizeof(GCfuncC)-sizeof(TValue)+sizeof(TValue)*(n))
#define sizeLfunc(n)	(sizeof(GCfuncL)-sizeof(GCRef)+sizeof(GCRef)*(n))

typedef struct GCtab {
  GCHeader;
  uint8_t nomm;		/* Negative cache for fast metamethods. */
  int8_t colo;		/* Array colocation. */
  MRef array;		/* Array part. */
  GCRef gclist;
  GCRef metatable;	/* Must be at same offset in GCudata. */
  MRef node;		/* Hash part. */
  uint32_t asize;	/* Size of array part (keys [0, asize-1]). */
  uint32_t hmask;	/* Hash part mask (size of hash part - 1). */
#if LJ_GC64
  MRef freetop;		/* Top of free elements. */
#endif
} GCtab;


typedef struct GCproto {
  GCHeader;
  uint8_t numparams;	/* Number of parameters. */
  uint8_t framesize;	/* Fixed frame size. */
  MSize sizebc;		/* Number of bytecode instructions. */
#if LJ_GC64
  uint32_t unused_gc64;
#endif
  GCRef gclist;
  MRef k;		/* Split constant array (points to the middle). */
  MRef uv;		/* Upvalue list. local slot|0x8000 or parent uv idx. */
  MSize sizekgc;	/* Number of collectable constants. */
  MSize sizekn;		/* Number of lua_Number constants. */
  MSize sizept;		/* Total size including colocated arrays. */
  uint8_t sizeuv;	/* Number of upvalues. */
  uint8_t flags;	/* Miscellaneous flags (see below). */
  uint16_t trace;	/* Anchor for chain of root traces. */
  /* ------ The following fields are for debugging/tracebacks only ------ */
  GCRef chunkname;	/* Name of the chunk this function was defined in. */
  BCLine firstline;	/* First line of the function definition. */
  BCLine numline;	/* Number of lines for the function definition. */
  MRef lineinfo;	/* Compressed map from bytecode ins. to source line. */
  MRef uvinfo;		/* Upvalue names. */
  MRef varinfo;		/* Names and compressed extents of local variables. */
} GCproto;


struct lua_State {
  GCHeader;
  uint8_t dummy_ffid;
  uint8_t status;
  MRef glref;
  GCRef gclist;
  TValue* base;
  TValue* top;
  MRef maxstack;
  MRef stack;
  GCRef openupval;
  GCRef env;
  void* cframe;
  MSize stacksize;
};

typedef union GCobj {
  GChead gch;
  GCstr str;
  GCupval uv;
  lua_State th;
  GCproto pt;
  GCfunc fn;
  GCcdata cd;
  GCtab tab;
  GCudata ud;
} GCobj;

#if LJ_GC64
#define gcref(r)	((GCobj *)(r).gcptr64)
#define gcrefp(r, t)	((t *)(void *)(r).gcptr64)
#define gcrefu(r)	((r).gcptr64)
#define gcrefeq(r1, r2)	((r1).gcptr64 == (r2).gcptr64)

#define setgcref(r, gc)	((r).gcptr64 = (uint64_t)&(gc)->gch)
#define setgcreft(r, gc, it) \
  (r).gcptr64 = (uint64_t)&(gc)->gch | (((uint64_t)(it)) << 47)
#define setgcrefp(r, p)	((r).gcptr64 = (uint64_t)(p))
#define setgcrefnull(r)	((r).gcptr64 = 0)
#define setgcrefr(r, v)	((r).gcptr64 = (v).gcptr64)
#else
#define gcref(r)	((GCobj *)(uintptr_t)(r).gcptr32)
#define gcrefp(r, t)	((t *)(void *)(uintptr_t)(r).gcptr32)
#define gcrefu(r)	((r).gcptr32)
#define gcrefeq(r1, r2)	((r1).gcptr32 == (r2).gcptr32)

#define setgcref(r, gc)	((r).gcptr32 = (uint32_t)(uintptr_t)&(gc)->gch)
#define setgcrefp(r, p)	((r).gcptr32 = (uint32_t)(uintptr_t)(p))
#define setgcrefnull(r)	((r).gcptr32 = 0)
#define setgcrefr(r, v)	((r).gcptr32 = (v).gcptr32)
#endif

#if LJ_GC64
#define gcval(o)	((GCobj *)(gcrefu((o)->gcr) & LJ_GCVMASK))
#else
#define gcval(o)	(gcref((o)->gcr))
#endif

/* Macros to get instruction fields. */
#define bc_op(i)	((BCOp)((i)&0xff))
#define bc_a(i)		((BCReg)(((i)>>8)&0xff))
#define bc_b(i)		((BCReg)((i)>>24))
#define bc_c(i)		((BCReg)(((i)>>16)&0xff))
#define bc_d(i)		((BCReg)((i)>>16))
#define bc_j(i)		((ptrdiff_t)bc_d(i)-BCBIAS_J)


#if LJ_GC64
#define mref(r, t)	((t *)(void *)(r).ptr64)

#define setmref(r, p)	((r).ptr64 = (uint64_t)(void *)(p))
#define setmrefr(r, v)	((r).ptr64 = (v).ptr64)
#else
#define mref(r, t)	((t *)(void *)(uintptr_t)(r).ptr32)

#define setmref(r, p)	((r).ptr32 = (uint32_t)(uintptr_t)(void *)(p))
#define setmrefr(r, v)	((r).ptr32 = (v).ptr32)
#endif


#define gcV(o)		check_exp(tvisgcv(o), gcval(o))
#define strV(o)		check_exp(tvisstr(o), &gcval(o)->str)
#define funcV(o)	check_exp(tvisfunc(o), &gcval(o)->fn)
#define threadV(o)	check_exp(tvisthread(o), &gcval(o)->th)
#define protoV(o)	check_exp(tvisproto(o), &gcval(o)->pt)
#define cdataV(o)	check_exp(tviscdata(o), &gcval(o)->cd)
#define tabV(o)		check_exp(tvistab(o), &gcval(o)->tab)
#define udataV(o)	check_exp(tvisudata(o), &gcval(o)->ud)
#define numV(o)		check_exp(tvisnum(o), (o)->n)
#define intV(o)		check_exp(tvisint(o), (int32_t)(o)->i)

#define luai_apicheck(L, o)	{ (void)L; }
#define lua_assert(c)		((void)0)
#define check_exp(c, e)		(e)
#define api_check		luai_apicheck


#define api_checkvalidindex(L, i)	api_check(L, (i) != niltv(L))


#define sizetabcolo(n)	((n)*sizeof(TValue) + sizeof(GCtab))
#define tabref(r)	(&gcref((r))->tab)
#define noderef(r)	(mref((r), Node))
#define nextnode(n)	(mref((n)->next, Node))


#define setitype(o, i)		((o)->it = (i))
static LJ_AINLINE void setgcVraw(TValue* o, GCobj* v, uint32_t itype) {
#if LJ_GC64
  setgcreft(o->gcr, v, itype);
#else
  setgcref(o->gcr, v); setitype(o, itype);
#endif
}

#define LJ_TISGCV		(LJ_TSTR+1)
#define tvisgcv(o)	((itype(o) - LJ_TISGCV) > (LJ_TNUMX - LJ_TISGCV))
#define UNUSED(x)	((void)(x))	/* to avoid warnings */
#define tvchecklive(L, o) \
  UNUSED(L), lua_assert(!tvisgcv(o) || \
  ((~itype(o) == gcval(o)->gch.gct) && !isdead(G(L), gcval(o))))
static LJ_AINLINE void setgcV(lua_State* L, TValue* o, GCobj* v, uint32_t it) {
  setgcVraw(o, v, it); tvchecklive(L, o);
}


#define tvref(r)	(mref(r, TValue))

#define incr_top(L) \
  (++L->top >= tvref(L->maxstack) && (lj_state_growstack1(L), 0))

#define obj2gco(v)	((GCobj *)(v))
#define api_checknelems(L, n)		api_check(L, (n) <= (L->top - L->base))


#define LJ_GC_WHITE0	0x01
#define LJ_GC_WHITE1	0x02
#define LJ_GC_BLACK	0x04
#define LJ_GC_FINALIZED	0x08
#define LJ_GC_WEAKKEY	0x08
#define LJ_GC_WEAKVAL	0x10
#define LJ_GC_CDATA_FIN	0x10
#define LJ_GC_FIXED	0x20
#define LJ_GC_SFIXED	0x40

#define LJ_GC_WHITES	(LJ_GC_WHITE0 | LJ_GC_WHITE1)
#define LJ_GC_COLORS	(LJ_GC_WHITES | LJ_GC_BLACK)
#define LJ_GC_WEAK	(LJ_GC_WEAKKEY | LJ_GC_WEAKVAL)

#define G(L)			(mref(L->glref, global_State))
#define registry(L)		(&G(L)->registrytv)

#define iswhite(x)	((x)->gch.marked & LJ_GC_WHITES)
#define isblack(x)	((x)->gch.marked & LJ_GC_BLACK)
#define lj_gc_objbarrier(L, p, o) \
  { if (iswhite(obj2gco(o)) && isblack(obj2gco(p))) \
      lj_gc_barrierf(G(L), obj2gco(p), obj2gco(o)); }


#define define_setV(name, type, tag) \
static LJ_AINLINE void name(lua_State *L, TValue *o, type *v) \
{ \
  setgcV(L, o, obj2gco(v), tag); \
}
define_setV(setstrV, GCstr, LJ_TSTR)
define_setV(setthreadV, lua_State, LJ_TTHREAD)
define_setV(setprotoV, GCproto, LJ_TPROTO)
define_setV(setfuncV, GCfunc, LJ_TFUNC)
define_setV(setcdataV, GCcdata, LJ_TCDATA)
define_setV(settabV, GCtab, LJ_TTAB)
define_setV(setudataV, GCudata, LJ_TUDATA)

#if LJ_GC64
#define setitype(o, i)		((o)->it = ((i) << 15))
#define setnilV(o)		((o)->it64 = -1)
#define setpriV(o, x)		((o)->it64 = (int64_t)~((uint64_t)~(x)<<47))
#define setboolV(o, x)		((o)->it64 = (int64_t)~((uint64_t)((x)+1)<<47))
#else
#define setitype(o, i)		((o)->it = (i))
#define setnilV(o)		((o)->it = LJ_TNIL)
#define setboolV(o, x)		((o)->it = LJ_TFALSE-(uint32_t)(x))
#define setpriV(o, i)		(setitype((o), (i)))
#endif

typedef struct GCState {
  GCSize total;		/* Memory currently allocated. */
  GCSize threshold;	/* Memory threshold. */
  uint8_t currentwhite;	/* Current white color. */
  uint8_t state;	/* GC state. */
  uint8_t nocdatafin;	/* No cdata finalizer called. */
  uint8_t unused2;
  MSize sweepstr;	/* Sweep position in string table. */
  GCRef root;		/* List of all collectable objects. */
  MRef sweep;		/* Sweep position in root list. */
  GCRef gray;		/* List of gray objects. */
  GCRef grayagain;	/* List of objects for atomic traversal. */
  GCRef weak;		/* List of weak tables (to be cleared). */
  GCRef mmudata;	/* List of userdata (to be finalized). */
  GCSize debt;		/* Debt (how much GC is behind schedule). */
  GCSize estimate;	/* Estimate of memory actually in use. */
  MSize stepmul;	/* Incremental GC step granularity. */
  MSize pause;		/* Pause between successive GC cycles. */
} GCState;
/* Resizable string buffer. Need this here, details in lj_buf.h. */
typedef struct SBuf {
  MRef p;		/* String buffer pointer. */
  MRef e;		/* String buffer end pointer. */
  MRef b;		/* String buffer base. */
  MRef L;		/* lua_State, used for buffer resizing. */
} SBuf;
typedef struct Node {
  TValue val;		/* Value object. Must be first field. */
  TValue key;		/* Key object. */
  MRef next;		/* Hash chain. */
#if !LJ_GC64
  MRef freetop;		/* Top of free elements (stored in t->node[0]). */
#endif
} Node;


#ifdef LJ_HASFFI
#define MMDEF_FFI(_) _(new)
#else
#define MMDEF_FFI(_)
#endif

#if LJ_52 || LJ_HASFFI
#define MMDEF_PAIRS(_) _(pairs) _(ipairs)
#else
#define MMDEF_PAIRS(_)
#define MM_pairs	255
#define MM_ipairs	255
#endif
#define MMDEF(_) \
  _(index) _(newindex) _(gc) _(mode) _(eq) _(len) \
  /* Only the above (fast) metamethods are negative cached (max. 8). */ \
  _(lt) _(le) _(concat) _(call) \
  /* The following must be in ORDER ARITH. */ \
  _(add) _(sub) _(mul) _(div) _(mod) _(pow) _(unm) \
  /* The following are used in the standard libraries. */ \
  _(metatable) _(tostring) MMDEF_FFI(_) MMDEF_PAIRS(_)

typedef enum {
#define MMENUM(name)	MM_##name,
  MMDEF(MMENUM)
#undef MMENUM
  MM__MAX,
  MM____ = MM__MAX,
  MM_FAST = MM_len
} MMS;

typedef enum {
  GCROOT_MMNAME,	/* Metamethod names. */
  GCROOT_MMNAME_LAST = GCROOT_MMNAME + MM__MAX - 1,
  GCROOT_BASEMT,	/* Metatables for base types. */
  GCROOT_BASEMT_NUM = GCROOT_BASEMT + ~LJ_TNUMX,
  GCROOT_IO_INPUT,	/* Userdata for default I/O input file. */
  GCROOT_IO_OUTPUT,	/* Userdata for default I/O output file. */
  GCROOT_MAX
} GCRootID;
typedef struct global_State {
  GCRef* strhash;	/* String hash table (hash chain anchors). */
  MSize strmask;	/* String hash mask (size of hash table - 1). */
  MSize strnum;		/* Number of strings in hash table. */
  lua_Alloc allocf;	/* Memory allocator. */
  void* allocd;		/* Memory allocator data. */
  GCState gc;		/* Garbage collector. */
  volatile int32_t vmstate;  /* VM state or current JIT code trace number. */
  SBuf tmpbuf;		/* Temporary string buffer. */
  GCstr strempty;	/* Empty string. */
  uint8_t stremptyz;	/* Zero terminator of empty string. */
  uint8_t hookmask;	/* Hook mask. */
  uint8_t dispatchmode;	/* Dispatch mode. */
  uint8_t vmevmask;	/* VM event mask. */
  GCRef mainthref;	/* Link to main thread. */
  TValue registrytv;	/* Anchor for registry. */
  TValue tmptv, tmptv2;	/* Temporary TValues. */
  Node nilnode;		/* Fallback 1-element hash part (nil key and value). */
  GCupval uvhead;	/* Head of double-linked list of all open upvalues. */
  int32_t hookcount;	/* Instruction hook countdown. */
  int32_t hookcstart;	/* Start count for instruction hook counter. */
  lua_Hook hookf;	/* Hook function. */
  lua_CFunction wrapf;	/* Wrapper for C function calls. */
  lua_CFunction panic;	/* Called as a last resort for errors. */
  BCIns bc_cfunc_int;	/* Bytecode for internal C function calls. */
  BCIns bc_cfunc_ext;	/* Bytecode for external C function calls. */
  GCRef cur_L;		/* Currently executing lua_State. */
  MRef jit_base;	/* Current JIT code L->base or NULL. */
  MRef ctype_state;	/* Pointer to C type state. */
  GCRef gcroot[GCROOT_MAX];  /* GC roots. */
} global_State;

#define LJ_LIKELY(x)	(x)
#define LJ_UNLIKELY(x)	(x)

#define white2gray(x)		((x)->gch.marked &= (uint8_t)~LJ_GC_WHITES)
#define gray2black(x)		((x)->gch.marked |= LJ_GC_BLACK)
#define isfinalized(u)		((u)->marked & LJ_GC_FINALIZED)
#define curwhite(g)	((g)->gc.currentwhite & LJ_GC_WHITES)
#define makewhite(g, x) \
  ((x)->gch.marked = ((x)->gch.marked & (uint8_t)~LJ_GC_COLORS) | curwhite(g))


#define gco2str(o)	check_exp((o)->gch.gct == ~LJ_TSTR, &(o)->str)
#define gco2uv(o)	check_exp((o)->gch.gct == ~LJ_TUPVAL, &(o)->uv)
#define gco2th(o)	check_exp((o)->gch.gct == ~LJ_TTHREAD, &(o)->th)
#define gco2pt(o)	check_exp((o)->gch.gct == ~LJ_TPROTO, &(o)->pt)
#define gco2func(o)	check_exp((o)->gch.gct == ~LJ_TFUNC, &(o)->fn)
#define gco2cd(o)	check_exp((o)->gch.gct == ~LJ_TCDATA, &(o)->cd)
#define gco2tab(o)	check_exp((o)->gch.gct == ~LJ_TTAB, &(o)->tab)
#define gco2ud(o)	check_exp((o)->gch.gct == ~LJ_TUDATA, &(o)->ud)

extern void gc_mark(global_State* g, GCobj* o);


#define tviswhite(x)	(tvisgcv(x) && iswhite(gcV(x)))
#define uvval(uv_)	(mref((uv_)->v, TValue))
#define gc_markobj(g, o) \
  { if (iswhite(obj2gco(o))) gc_mark(g, obj2gco(o)); }
#define gc_marktv(g, tv) \
  { lua_assert(!tvisgcv(tv) || (~itype(tv) == gcval(tv)->gch.gct)); \
    if (tviswhite(tv)) gc_mark(g, gcV(tv)); }


#define LJ_STATIC_ASSERT(cond) static_assert(cond);


typedef struct lj_Debug {};
typedef const TValue cTValue;

/* The env field SHOULD be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, env) == offsetof(GCfuncL, env));
LJ_STATIC_ASSERT(offsetof(GChead, env) == offsetof(GCudata, env));

/* The metatable field MUST be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, metatable) == offsetof(GCtab, metatable));
LJ_STATIC_ASSERT(offsetof(GChead, metatable) == offsetof(GCudata, metatable));

/* The gclist field MUST be at the same offset for all GC objects. */
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(lua_State, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCproto, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCfuncL, gclist));
LJ_STATIC_ASSERT(offsetof(GChead, gclist) == offsetof(GCtab, gclist));