#include "generated_game_information.h"

#include <string>
#include <vector>
#include <iostream>


void scan_for_all_lua_functions(const char* executable);
extern unsigned __int64 FindPattern(std::string module, const char* pattern, const char* mask);

class SigContainer {
public:
	const char* name;
	const char* pattern;
	const char* mask;
	unsigned __int64* destination;
public:
	SigContainer(const char* name, const char* pattern, const char* mask, unsigned __int64* destination) :
	  name(name), pattern(pattern), mask(mask), destination(destination) {}
};

class ScanForSig {
public:
	static std::vector<SigContainer> signatures;
	ScanForSig(const char* name, const char* pattern, const char* mask, unsigned __int64* destination) {
		*destination = 0;
		signatures.emplace_back(SigContainer(name, pattern, mask, destination));
	}
};

std::vector<SigContainer> ScanForSig::signatures;

#define DIESELLUAIMPL
#include "diesellua.h"
#undef DIESELLUAIMPL

#include "luajit_types.h"

// old luajit
#if defined(RAIDWW2) && RAID_VERSION < 15703546
typedef int(__cdecl* lua_getinfo_type)(lua_State* L, const char* what, lua_Debug* ar, int unknown);
lua_getinfo_type lua_getinfo_;
ScanForSig lua_getinfo_sigsearch("lua_getinfo", "\x48\x89\x5C\x24\x00\x44\x89\x4C\x24\x00\x55\x56\x57\x41\x54", "xxxx?xxxx?xxxxx", (unsigned long long*)&lua_getinfo_);

int __cdecl lua_getinfo(lua_State* L, const char* what, lua_Debug* ar) {
	return lua_getinfo_(L, what, ar, 0);
}
const void* lua_topointer(lua_State* L, int idx) {
	cTValue* o = index2adr(L, idx);
	if (tvisfunc(o)) {
		BCOp op = bc_op(*mref(funcV(o)->c.pc, BCIns));
		if (op == BC_FUNCC || op == BC_FUNCCW)
			return funcV(o)->c.f;
	}
	return NULL;
}

void lj_state_growstack1(lua_State* L) {
	lj_state_growstack(L, 1);
}
void lua_getfenv(lua_State* L, int idx) {
	cTValue* o = index2adr(L, idx);
	api_checkvalidindex(L, o);
	if (tvisfunc(o)) {
		settabV(L, L->top, tabref(funcV(o)->c.env));
	}
	else if (tvisudata(o)) {
		settabV(L, L->top, tabref(udataV(o)->env));
	}
	else if (tvisthread(o)) {
		settabV(L, L->top, tabref(threadV(o)->env));
	}
	else {
		setnilV(L->top);
	}
	incr_top(L);
}

enum {
	GCSpause, GCSpropagate, GCSatomic, GCSsweepstring, GCSsweep, GCSfinalize
};
void gc_mark(global_State* g, GCobj* o) {
	int gct = o->gch.gct;
	lua_assert(iswhite(o) && !isdead(g, o));
	white2gray(o);
	if (LJ_UNLIKELY(gct == ~LJ_TUDATA)) {
		GCtab* mt = tabref(gco2ud(o)->metatable);
		gray2black(o);  /* Userdata are never gray. */
		if (mt) gc_markobj(g, mt);
		gc_markobj(g, tabref(gco2ud(o)->env));
	}
	else if (LJ_UNLIKELY(gct == ~LJ_TUPVAL)) {
		GCupval* uv = gco2uv(o);
		gc_marktv(g, uvval(uv));
		if (uv->closed)
			gray2black(o);  /* Closed upvalues are never gray. */
	}
	else if (gct != ~LJ_TSTR && gct != ~LJ_TCDATA) {
		lua_assert(gct == ~LJ_TFUNC || gct == ~LJ_TTAB ||
			gct == ~LJ_TTHREAD || gct == ~LJ_TPROTO || gct == ~LJ_TTRACE);
		setgcrefr(o->gch.gclist, g->gc.gray);
		setgcref(g->gc.gray, o);
	}
}
void lj_gc_barrierf(global_State* g, GCobj* o, GCobj* v) {
	lua_assert(isblack(o) && iswhite(v) && !isdead(g, v) && !isdead(g, o));
	lua_assert(g->gc.state != GCSfinalize && g->gc.state != GCSpause);
	lua_assert(o->gch.gct != ~LJ_TTAB);
	/* Preserve invariant during propagation. Otherwise it doesn't matter. */
	if (g->gc.state == GCSpropagate || g->gc.state == GCSatomic)
		gc_mark(g, v);  /* Move frontier forward. */
	else
		makewhite(g, o);  /* Make it white to avoid the following barrier. */
}
int lua_setfenv(lua_State* L, int idx) {
	cTValue* o = index2adr(L, idx);
	GCtab* t;
	api_checknelems(L, 1);
	api_checkvalidindex(L, o);
	api_check(L, tvistab(L->top - 1));
	t = tabV(L->top - 1);
	if (tvisfunc(o)) {
		setgcref(funcV(o)->c.env, obj2gco(t));
	}
	else if (tvisudata(o)) {
		setgcref(udataV(o)->env, obj2gco(t));
	}
	else if (tvisthread(o)) {
		setgcref(threadV(o)->env, obj2gco(t));
	}
	else {
		L->top--;
		return 0;
	}
	lj_gc_objbarrier(L, gcV(o), t);
	L->top--;
	return 1;
}
#endif

// latest luajit
#if defined(RAIDWW2) && RAID_VERSION > 15703546
static LJ_AINLINE void copyTV(lua_State* L, TValue* o1, const TValue* o2) {
	*o1 = *o2;
}
#define lj_str_newz(L, s)	(lj_str_new(L, s, strlen(s)))
int luaL_getmetafield(lua_State* L, int obj, const char* e) {
  if(lua_getmetatable(L, obj))
  {
		cTValue* tv = lj_tab_getstr(tabV(L->top - 1), lj_str_newz(L, e));
		if(tv && !tvisnil(tv))
		{
			copyTV(L, L->top - 1, tv);
			return 1;
		}
		L->top--;
  }
	return 0;
}
int luaL_callmeta(lua_State* L, int obj, const char* e) {
  if(luaL_getmetafield(L, obj, e))
  {
		TValue* top = L->top--;
		if (LJ_FR2) setnilV(top++);
		copyTV(L, top++, index2adr(L, obj));
		L->top = top;
		lj_vm_call(L, top - 1, 1 + 1);
		return 1;
  }
	return 0;
}
int lua_gettop(lua_State* L) {
	return (int)(L->top - L->base);
}
const void* lua_topointer(lua_State* L, int idx) {
	return lj_obj_ptr(G(L), index2adr(L, idx));
}

int lua_getinfo(lua_State* L, const char* what, lua_Debug* ar) {
	return lj_debug_getinfo(L, what, (lj_Debug*)ar, 0);
}

// skip over error stuff
#define LJ_ERR_STKOV 0
void lj_err_stkov(lua_State* L) {}
void lj_err_throw(lua_State*L, int errcode) {}
GCstr* lj_err_str(lua_State* L, int str) { return nullptr; }

void lj_state_growstack(lua_State* L, MSize need) {
	MSize n = L->stacksize + need;
	if (LJ_LIKELY(n < LJ_STACK_MAX)) {  /* The stack can grow as requested. */
		if (n < 2 * L->stacksize) {  /* Try to double the size. */
			n = 2 * L->stacksize;
			if (n > LJ_STACK_MAX)
				n = LJ_STACK_MAX;
		}
		resizestack(L, n);
	}
	else {  /* Request would overflow. Raise a stack overflow error. */
		if (LJ_HASJIT) {
			TValue* base = tvref(G(L)->jit_base);
			if (base) L->base = base;
		}
		if (curr_funcisL(L)) {
			L->top = curr_topL(L);
			if (L->top > tvref(L->maxstack)) {
				/* The current Lua frame violates the stack, so replace it with a
				** dummy. This can happen when BC_IFUNCF is trying to grow the stack.
				*/
				L->top = L->base;
				setframe_gc(L->base - 1 - LJ_FR2, obj2gco(L), LJ_TTHREAD);
			}
		}
		if (L->stacksize <= LJ_STACK_MAXEX) {
			/* An error handler might want to inspect the stack overflow error, but
			** will need some stack space to run in. We give it a stack size beyond
			** the normal limit in order to do so, then rely on lj_state_relimitstack
			** calls during unwinding to bring us back to a convential stack size.
			** The + 1 is space for the error message, and 2 * LUA_MINSTACK is for
			** the lj_state_checkstack() call in lj_err_run().
			*/
			resizestack(L, LJ_STACK_MAX + 1 + 2 * LUA_MINSTACK);
			lj_err_stkov(L);  /* May invoke an error handler. */
		}
		else {
			/* If we're here, then the stack overflow error handler is requesting
			** to grow the stack even further. We have no choice but to abort the
			** error handler.
			*/
			GCstr* em = lj_err_str(L, LJ_ERR_STKOV);  /* Might OOM. */
			setstrV(L, L->top++, em);  /* There is always space to push an error. */
			lj_err_throw(L, LUA_ERRERR);  /* Does not invoke an error handler. */
		}
	}
}
#endif

void scan_for_all_lua_functions(const char* executable) {
	int success_count = 0;
  for(auto& signature : ScanForSig::signatures)
  {
		if(std::string(signature.pattern) == "" || std::string(signature.mask) == "")
		{
			std::cout << signature.name << " missing pattern or signature" << std::endl;
		}
		*signature.destination = FindPattern(executable, signature.pattern, signature.mask);
		success_count++;
		if((void*)(*signature.destination) == nullptr) {
			std::cout << "Function not found " << signature.name << std::endl;
			success_count--;
		}
  }
	std::cout << "Successfully found " << success_count << " functions in " << executable << std::endl;
}

__declspec(dllexport) std::uintptr_t get_lua_function_address(const char* name) {
	for (auto& signature : ScanForSig::signatures) {
		if (std::string(signature.name) == std::string(name)) {
			return *signature.destination;
		}
	}
	return reinterpret_cast<std::uintptr_t>(nullptr);
}