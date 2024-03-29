#include "al2o3_platform/platform.h"
#include "utils_misccpp/compiletimehash.hpp"
#include "al2o3_vfile/vfile.hpp"
#include "gfx_imageio/io.h"
#include "gfx_image/create.h"
#include "gfx_image/utils.h"
#include "gfx_imagecompress/imagecompress.h"
#include "lua_base5.3/lua.hpp"
#include "lua_base5.3/utils.h"

static char const MetaName[] = "Al2o3.Image";

// create the null image user data return on the lua state
static Image_ImageHeader const** imageud_create(lua_State *L) {
	// allocate a pointer and push it onto the stack
	auto ud = (Image_ImageHeader const**)lua_newuserdata(L, sizeof(Image_ImageHeader*));
	if(ud == nullptr) return nullptr;

	*ud = nullptr;
	luaL_getmetatable(L, MetaName);
	lua_setmetatable(L, -2);
	return ud;
}

static int imageud_gc (lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	if (image) Image_Destroy(image);

	return 0;
}

static int width(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->width);
	return 1;
}

static int height(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->height);
	return 1;
}
static int depth(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->depth);
	return 1;
}
static int slices(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, image->slices);
	return 1;
}

static int format(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushstring(L, TinyImageFormat_Name(image->format));
	return 1;
}

static int flags(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_createtable(L, 0, 2);
	lua_pushboolean(L, image->flags & Image_Flag_Cubemap);
	lua_setfield(L, -2, "Cubemap");
	lua_pushboolean(L, image->flags & Image_Flag_HeaderOnly);
	lua_setfield(L, -2, "HeaderOnly");
	return 1;
}
static int dimensions(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");

	lua_pushinteger(L, image->width);
	lua_pushinteger(L, image->height);
	lua_pushinteger(L, image->depth);
	lua_pushinteger(L, image->slices);
	return 4;
}

static int getPixelAt(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	double pixel[4];
	Image_GetPixelAtD(image, (double*)&pixel, index);

	lua_pushnumber(L, pixel[0]); // r
	lua_pushnumber(L, pixel[1]); // g
	lua_pushnumber(L, pixel[2]); // b
	lua_pushnumber(L, pixel[3]); // a

	return 4;
}

static int setPixelAt(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);

	double pixel[4];
	pixel[0] = luaL_checknumber(L, 3); // r
	pixel[1] = luaL_checknumber(L, 4); // g
	pixel[2] = luaL_checknumber(L, 5); // b
	pixel[3] = luaL_checknumber(L, 6); // a

	Image_SetPixelAtD(image, pixel, index);

	return 0;
}

static int is1D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is1D(image));
	return 1;
}

static int is2D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is2D(image));
	return 1;
}

static int is3D(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_Is3D(image));
	return 1;
}

static int isArray(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_IsArray(image));
	return 1;
}

static int isCubemap(lua_State *L) {
	Image_ImageHeader* image = *(Image_ImageHeader**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushboolean(L, Image_IsCubemap(image));
	return 1;
}

static int pixelCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountOf(image));
	return 1;
}
static int pixelCountPerSlice(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerSliceOf(image));
	return 1;
}

static int pixelCountPerPage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerPageOf(image));
	return 1;
}

static int pixelCountPerRow(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_PixelCountPerRowOf(image));
	return 1;
}

static int byteCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountOf(image));
	return 1;
}

static int byteCountPerSlice(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerSliceOf(image));
	return 1;
}


static int byteCountPerPage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerPageOf(image));
	return 1;
}


static int byteCountPerRow(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountPerRowOf(image));
	return 1;
}

static int byteCountOfImageChain(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_ByteCountOfImageChainOf(image));
	return 1;
}

static int bytesRequiredForMipMaps(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_BytesRequiredForMipMapsOf(image));
	return 1;
}

static int calculateIndex(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t x = luaL_checkinteger(L, 2);
	int64_t y = luaL_checkinteger(L, 3);
	int64_t z = luaL_checkinteger(L, 4);
	int64_t s = luaL_checkinteger(L, 5);

	lua_pushinteger(L, Image_CalculateIndex(image, (uint32_t)x, (uint32_t)y, (uint32_t)z, (uint32_t)s));
	return 1;
}

static int copy(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	auto dst = *(Image_ImageHeader const**)luaL_checkudata(L, 2, MetaName);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyImage(src, dst);
	return 0;
}

static int copySlice(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sw = luaL_checkinteger(L, 2);
	auto dst = *(Image_ImageHeader const**)luaL_checkudata(L, 3, MetaName);
	int64_t dw = luaL_checkinteger(L, 4);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopySlice(src, (uint32_t)sw, dst, (uint32_t)dw);
	return 0;
}

static int copyPage(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sz = luaL_checkinteger(L, 2);
	int64_t sw = luaL_checkinteger(L, 3);
	auto dst = *(Image_ImageHeader const**)luaL_checkudata(L, 4, MetaName);
	int64_t dz = luaL_checkinteger(L, 5);
	int64_t dw = luaL_checkinteger(L, 6);
	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyPage(src, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int copyRow(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sy = luaL_checkinteger(L, 2);
	int64_t sz = luaL_checkinteger(L, 3);
	int64_t sw = luaL_checkinteger(L, 4);
	auto dst = *(Image_ImageHeader const**)luaL_checkudata(L, 5, MetaName);
	int64_t dy = luaL_checkinteger(L, 6);
	int64_t dz = luaL_checkinteger(L, 7);
	int64_t dw = luaL_checkinteger(L, 8);

	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyRow(src, (uint32_t)sy, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dy, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int copyPixel(lua_State *L) {
	auto src = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	int64_t sx = luaL_checkinteger(L, 2);
	int64_t sy = luaL_checkinteger(L, 3);
	int64_t sz = luaL_checkinteger(L, 4);
	int64_t sw = luaL_checkinteger(L, 5);
	auto dst = *(Image_ImageHeader const**)luaL_checkudata(L, 6, MetaName);
	int64_t dx = luaL_checkinteger(L, 7);
	int64_t dy = luaL_checkinteger(L, 8);
	int64_t dz = luaL_checkinteger(L, 9);
	int64_t dw = luaL_checkinteger(L, 10);

	LUA_ASSERT(src, L, "image is NIL");
	LUA_ASSERT(dst, L, "image is NIL");

	Image_CopyPixel(src, (uint32_t)sx, (uint32_t)sy, (uint32_t)sz, (uint32_t)sw, dst, (uint32_t)dx, (uint32_t)dy, (uint32_t)dz, (uint32_t)dw);
	return 0;
}

static int linkedImageCount(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	lua_pushinteger(L, Image_LinkedImageCountOf(image));
	return 1;
}

static int linkedImage(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	int64_t index = luaL_checkinteger(L, 2);
	auto ud = imageud_create(L);
	*ud = Image_LinkedImageOf(image, index);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_CreateNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	char const* fmt = luaL_checkstring(L, 2);

	auto ud = imageud_create(L);
	*ud = Image_Create1D((uint32_t)w,TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	char const* fmt = luaL_checkstring(L, 2);

	auto ud = imageud_create(L);
	*ud = Image_Create1DNoClear((uint32_t)w, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t s = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create1DArray((uint32_t)w, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create1DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t s = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create1DArrayNoClear((uint32_t)w, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create2D((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int create2DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_Create2DNoClear((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create2DArray((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create2DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create2DArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3D(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create3D((uint32_t)w, (uint32_t)h, (uint32_t)d, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3DNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_Create3DNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int create3DArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create3DArray((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int create3DArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t d = luaL_checkinteger(L, 3);
	int64_t s = luaL_checkinteger(L, 4);
	char const* fmt = luaL_checkstring(L, 5);

	auto ud = imageud_create(L);
	*ud = Image_Create3DArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)d, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}
static int createCubemap(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemap((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	char const* fmt = luaL_checkstring(L, 3);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapNoClear((uint32_t)w, (uint32_t)h, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapArray(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapArray((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createCubemapArrayNoClear(lua_State *L) {
	int64_t w = luaL_checkinteger(L, 1);
	int64_t h = luaL_checkinteger(L, 2);
	int64_t s = luaL_checkinteger(L, 3);
	char const* fmt = luaL_checkstring(L, 4);

	auto ud = imageud_create(L);
	*ud = Image_CreateCubemapArrayNoClear((uint32_t)w, (uint32_t)h, (uint32_t)s, TinyImageFormat_FromName(fmt));
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int createMipMapChain(lua_State * L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool generateFromImage = lua_isnil(L, 2) ? true : (bool)lua_toboolean(L, 2);
	Image_CreateMipMapChain(image,generateFromImage);
	return 0;
}

static int clone(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	*ud = Image_Clone(image);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int cloneStructure(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	*ud = Image_CloneStructure(image);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int preciseConvert(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	auto ud = imageud_create(L);
	*ud = Image_PreciseConvert(image, TinyImageFormat_FromName(luaL_checkstring(L,2)) );
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int fastConvert(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_FastConvert(image, TinyImageFormat_FromName(luaL_checkstring(L,2)), allowInPlace);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC1(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC1(image, nullptr, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC2(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC2(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC3(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC3(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC4(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC4(image, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC5(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC5(image, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC6H(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC6H(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}

static int compressAMDBC7(lua_State *L) {
	auto image = *(Image_ImageHeader const**)luaL_checkudata(L, 1, MetaName);
	LUA_ASSERT(image, L, "image is NIL");
	bool allowInPlace = lua_isnil(L, 2) ? false : (bool)lua_toboolean(L, 3);
	auto ud = imageud_create(L);
	*ud = Image_CompressAMDBC7(image, nullptr, nullptr, nullptr);
	lua_pushboolean(L, *ud != nullptr);
	return 2;
}


static int load(lua_State * L) {
	char const* filename = luaL_checkstring(L, 1);

	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_ReadBinary);
	if(!file) {
		lua_pushnil(L);
		lua_pushboolean(L, false);
		return 2;
	}

	auto ud = imageud_create(L);
	*ud = Image_Load(file);
	lua_pushboolean(L, *ud != nullptr);

	return 2;
}

static int saveAsDDS(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = Image_SaveAsDDS(image, file);

	lua_pushboolean(L, ret);
	return 1;
}

static int saveAsTGA(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = Image_SaveAsTGA(image, file);
	lua_pushboolean(L, ret);
	return 1;
}
static int saveAsBMP(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_SaveAsBMP(image, file);
	lua_pushboolean(L, ret);
	return 1;
}
static int saveAsPNG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_SaveAsPNG(image, file);
	lua_pushboolean(L, ret);
	return 1;
}

static int saveAsJPG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_SaveAsJPG(image, file);
	lua_pushboolean(L, ret);
	return 1;
}

static int saveAsKTX(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_SaveAsKTX(image, file);
	lua_pushboolean(L, ret);
	return 1;
}

static int saveAsHDR(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	char const* filename = luaL_checkstring(L, 2);
	VFile::ScopedFile file = VFile::File::FromFile(filename, Os_FM_WriteBinary);
	if(!file) {
		return 0;
	}
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_SaveAsHDR(image, file);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsDDS(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = Image_CanSaveAsDDS(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsTGA(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	Image_ImageHeader* image = *(Image_ImageHeader**)ud;
	bool ret = Image_CanSaveAsTGA(image);
	lua_pushboolean(L, ret);
	return 1;
}
static int canSaveAsBMP(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_CanSaveAsBMP(image);
	lua_pushboolean(L, ret);
	return 1;
}
static int canSaveAsPNG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_CanSaveAsPNG(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsJPG(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);

	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_CanSaveAsJPG(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsKTX(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_CanSaveAsKTX(image);
	lua_pushboolean(L, ret);
	return 1;
}

static int canSaveAsHDR(lua_State * L) {
	void* ud = luaL_checkudata(L, 1, MetaName);
	auto image = *(Image_ImageHeader const**)ud;
	bool ret = Image_CanSaveAsHDR(image);
	lua_pushboolean(L, ret);
	return 1;
}

AL2O3_EXTERN_C int LuaImage_Open(lua_State* L) {
	static const struct luaL_Reg imageObj [] = {
			{"width", &width},
			{"height", &height},
			{"depth", &depth},
			{"slices", &slices},
			{"dimensions", &dimensions},

			{"format", &format},
			{"flags", &flags},

			{"getPixelAt", &getPixelAt},
			{"setPixelAt", &setPixelAt},

			{"copy", &copy},
			{"copySlice", &copySlice},
			{"copyPage", &copyPage},
			{"copyRow", &copyRow},
			{"copyPixel", &copyPixel},

			{"is1D", &is1D},
			{"is2D", &is2D},
			{"is3D", &is3D},
			{"isArray", &isArray},
			{"isCubemap", &isCubemap},

			{"calculateIndex", &calculateIndex },

			{"pixelCount", &pixelCount },
			{"pixelCountPerSlice", &pixelCountPerSlice },
			{"pixelCountPerPage", &pixelCountPerPage },
			{"pixelCountPerRow", &pixelCountPerRow },

			{"byteCount", &byteCount },
			{"byteCountPerSlice", &byteCountPerSlice },
			{"byteCountPerPage", &byteCountPerPage },
			{"byteCountPerRow", &byteCountPerRow },

			{"linkedImageCount", &linkedImageCount},
			{"linkedImage", &linkedImage},

			{"byteCountOfImageChain", &byteCountOfImageChain},
			{"bytesRequiredForMipMaps", &bytesRequiredForMipMaps},

			{"createMipMapChain", &createMipMapChain},
			{"clone", &clone},
			{"cloneStructure", &cloneStructure},
			{"preciseConvert", &preciseConvert},
			{"fastConvert", &fastConvert},

			{"compressAMDBC1", &compressAMDBC1},
			{"compressAMDBC2", &compressAMDBC2},
			{"compressAMDBC3", &compressAMDBC3},
			{"compressAMDBC4", &compressAMDBC4},
			{"compressAMDBC5", &compressAMDBC5},
			{"compressAMDBC6H", &compressAMDBC6H},
			{"compressAMDBC7", &compressAMDBC7},

			{"saveAsTGA", &saveAsTGA},
			{"saveAsBMP", &saveAsBMP},
			{"saveAsPNG", &saveAsPNG},
			{"saveAsJPG", &saveAsJPG},
			{"saveAsHDR", &saveAsHDR},
			{"saveAsKTX", &saveAsKTX},
			{"saveAsDDS", &saveAsDDS},

			{"canSaveAsTGA", &canSaveAsTGA},
			{"canSaveAsBMP", &canSaveAsBMP},
			{"canSaveAsPNG", &canSaveAsPNG},
			{"canSaveAsJPG", &canSaveAsJPG},
			{"canSaveAsHDR", &canSaveAsHDR},
			{"canSaveAsKTX", &canSaveAsKTX},
			{"canSaveAsDDS", &canSaveAsDDS},
			{"__gc", &imageud_gc },
			{nullptr, nullptr}  /* sentinel */
	};

	static const struct luaL_Reg imageLib [] = {
			{"create", &create},
			{"createNoClear", &createNoClear},

			{"create1D", &create1D},
			{"create1DNoClear", &create1DNoClear},
			{"create1DArray", &create1DArray},
			{"create1DArrayNoClear", &create1DArrayNoClear},

			{"create2D", &create2D},
			{"create2DNoClear", &create2DNoClear},
			{"create2DArray", &create2DArray},
			{"create2DArrayNoClear", &create2DArrayNoClear},

			{"create3D", &create3D},
			{"create3DNoClear", &create3DNoClear},
			{"create3DArray", &create3DArray},
			{"create3DArrayNoClear", &create3DArrayNoClear},

			{"createCubemap", &createCubemap},
			{"createCubemapNoClear", &createCubemapNoClear},
			{"createCubemapArray", &createCubemapArray},
			{"createCubemapArrayNoClear", &createCubemapArrayNoClear},

			{"load", &load},
			{nullptr, nullptr}  /* sentinel */
	};

	luaL_newmetatable(L, MetaName);
	/* metatable.__index = metatable */
	lua_pushvalue(L, -1);
	lua_setfield(L, -2, "__index");

	/* register methods */
	luaL_setfuncs(L, imageObj, 0);

	luaL_newlib(L, imageLib);
	return 1;
}