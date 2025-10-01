#include <lua.h>
#include <lauxlib.h>
#include <string.h>

#include "sokol/sokol_gfx.h"

static int
lbindings_set_vb(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	int index = luaL_checkinteger(L, 2);
	if (index < 0 || index >= SG_MAX_VERTEXBUFFER_BINDSLOTS)
		return luaL_error(L, "Invalid vbuffer slot %d", index);
	luaL_checkudata(L, 3, "SOKOL_BUFFER");
	lua_settop(L, 3);
	lua_pushlightuserdata(L, &b->vertex_buffers[index]);
	lua_call(L, 1, 0);
	return 0;
}

static int
lbindings_set_voff(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	int index = luaL_checkinteger(L, 2);
	if (index < 0 || index >= SG_MAX_VERTEXBUFFER_BINDSLOTS)
		return luaL_error(L, "Invalid vbuffer slot %d", index);
	b->vertex_buffer_offsets[index] = luaL_checkinteger(L, 3);
	return 0;
}

static int
lbindings_set_ib(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	luaL_checkudata(L, 2, "SOKOL_BUFFER");
	lua_settop(L, 2);
	lua_pushlightuserdata(L, &b->index_buffer);
	lua_call(L, 1, 0);
	return 0;
}

static int
lbindings_set_ioff(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	b->index_buffer_offset = luaL_checkinteger(L, 2);
	return 0;
}

struct view {
	sg_view view;
	int type;
};

static int
lbindings_set_view(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	int index = luaL_checkinteger(L, 2);
	if (index < 0 || index >= SG_MAX_VIEW_BINDSLOTS)
		return luaL_error(L, "Invalid view slot %d", index);
	struct view *v = luaL_checkudata(L, 3, "SOKOL_VIEW");
	b->views[index] = v->view;
	return 0;
}

static int
lbindings_set_sampler(lua_State *L) {
	sg_bindings *b = luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	int index = luaL_checkinteger(L, 2);
	if (index < 0 || index >= SG_MAX_SAMPLER_BINDSLOTS)
		return luaL_error(L, "Invalid sampler slot %d", index);
	luaL_checkudata(L, 3, "SOKOL_SAMPLER");
	lua_settop(L, 3);
	lua_pushlightuserdata(L, &b->samplers[index]);
	lua_call(L, 1, 0);
	return 0;
}

static int
lbindings_apply(lua_State *L) {
	sg_bindings *b = (sg_bindings *)luaL_checkudata(L, 1, "SOKOL_BINDINGS");
	sg_apply_bindings(b);
	return 0;
}

#define VIEW_TYPE_INVALID 0
#define VIEW_TYPE_TEXTURE 1
#define VIEW_TYPE_STORAGE 2

static inline const char *
view_type_string(int type) {
	switch (type) {
	case VIEW_TYPE_TEXTURE : return "texture";
	case VIEW_TYPE_STORAGE : return "storage";
	default : return "invalid";
	}
}

static inline void
check_view_type(lua_State *L, struct view *v) {
	if (v->type != VIEW_TYPE_INVALID)
		luaL_error(L, "Invalid multi type set : %s", view_type_string(v->type));
}

static int
lview_tostring(lua_State *L) {
	struct view *v = (struct view *)lua_touserdata(L, 1);
	const char *s = view_type_string(v->type);
	lua_pushexternalstring(L, s, strlen(s), NULL, NULL);
	return 1;
}

static int
lview_release(lua_State *L) {
	struct view *v = (struct view *)lua_touserdata(L, 1);
	if (v->type != VIEW_TYPE_INVALID) {
		v->type = VIEW_TYPE_INVALID;
		sg_destroy_view(v->view);
	}
	return 0;
}

int
lview_new(lua_State *L) {
	luaL_checktype(L, 1, LUA_TTABLE);
	struct sg_view_desc desc;
	memset(&desc, 0 , sizeof(desc));
	struct view *v = NULL;
	if (lua_getfield(L, 1, "label") == LUA_TSTRING) {
		v = (struct view *)lua_newuserdatauv(L, sizeof(*v), 1);
		lua_insert(L, -2);
		desc.label = lua_tostring(L, -1);
		lua_setiuservalue(L, 1, 1);
	} else {
		lua_pop(L, 1);
		v = (struct view *)lua_newuserdatauv(L, sizeof(*v), 0);
	}
	v->type = VIEW_TYPE_INVALID;
	if (lua_getfield(L, 1, "texture") == LUA_TUSERDATA) {
		check_view_type(L, v);
		v->type = VIEW_TYPE_TEXTURE;
		// image
		luaL_checkudata(L, -1, "SOKOL_IMAGE");
		lua_pushlightuserdata(L, &desc.texture.image);
		lua_call(L, 1, 0);
	} else {
		lua_pop(L, 1);
	}
	if (lua_getfield(L, 1, "storage") == LUA_TUSERDATA) {
		check_view_type(L, v);
		v->type = VIEW_TYPE_STORAGE;
		luaL_checkudata(L, -1, "SOKOL_BUFFER");
		lua_pushlightuserdata(L, &desc.storage_buffer.buffer);
		lua_call(L, 1, 0);
	} else {
		lua_pop(L, 1);
	}
	if (v->type == VIEW_TYPE_INVALID)
		return luaL_error(L, "No view type");
	v->view = sg_make_view(&desc);
	if (luaL_newmetatable(L, "SOKOL_VIEW")) {
		luaL_Reg l[] = {
			{ "__index", NULL },
			{ "__tostring", lview_tostring },
			{ "__gc", lview_release },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, l, 0);

		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	return 1;
}

int
lbindings_new(lua_State *L) {
	sg_bindings * b = (sg_bindings *)lua_newuserdatauv(L, sizeof(*b), 0);
	memset(b, 0, sizeof(*b));
	if (luaL_newmetatable(L, "SOKOL_BINDINGS")) {
		luaL_Reg l[] = {
			{ "__index", NULL },
			{ "vbuffer", lbindings_set_vb },
			{ "voffset", lbindings_set_voff },
			{ "ibuffer", lbindings_set_ib },
			{ "ioffset", lbindings_set_ioff },
			{ "view", lbindings_set_view },
			{ "sampler", lbindings_set_sampler },
			{ "apply", lbindings_apply },
			{ NULL, NULL },
		};
		luaL_setfuncs(L, l, 0);

		lua_pushvalue(L, -1);
		lua_setfield(L, -2, "__index");
	}
	lua_setmetatable(L, -2);
	return 1;
}
