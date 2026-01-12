/* © 2022 David Given.
 * WordGrinder is licensed under the MIT open source license. See the COPYING
 * file in this distribution for the full text.
 */

#include "globals.h"

#include <cassert>
#include <map>
#include <vector>

namespace clip {

typedef size_t format;
enum formats : format {
    FORMAT_TEXT = 1,
    FORMAT_WG = 100,
};
typedef std::vector<char> Buffer;
typedef std::map<format, Buffer> Map;

static Map g_data;

static bool clear() {
  g_data.clear();
  return true;
}

static bool is_convertible(format f) {
  return (g_data.find(f) != g_data.end());
}

static bool set_data(format f, const char* buf, size_t len) {
  Buffer& dst = g_data[f];

  dst.resize(len);
  if (buf && len > 0)
    std::copy(buf, buf+len, dst.begin());

  if (f == clip::FORMAT_TEXT &&
      len > 0 && dst.back() != 0) {
    dst.push_back(0);
  }

  return true;
}

static bool get_data(format f, char* buf, size_t len) {
  assert(buf);

  if (!buf || !is_convertible(f))
    return false;

  const Buffer& src = g_data[f];
  std::copy(src.begin(), src.end(), buf);
  return true;
}

static size_t get_data_length(format f) {
  if (is_convertible(f))
    return g_data[f].size();
  else
    return 0;
}

} // namespace clip

static int clipboard_clear_cb(lua_State* L)
{
    clip::clear();
    return 0;
}

static void getdata(lua_State* L, clip::format format)
{
    if (clip::is_convertible(format))
    {
        size_t len = clip::get_data_length(format);

        std::vector<char> buf(len);
        clip::get_data(format, &buf[0], len);
        if (buf.back() == 0)
            len--;
        lua_pushlstring(L, &buf[0], len);
    }
    else
        lua_pushnil(L);
}

static int clipboard_get_cb(lua_State* L)
{
    getdata(L, clip::FORMAT_TEXT);
    getdata(L, clip::FORMAT_WG);

    return 2;
}

static void setdata(lua_State* L, int index, clip::format format)
{
    size_t len;
    const char* ptr = lua_tolstring(L, index, &len);

    if (ptr)
        clip::set_data(format, ptr, len);
}

static int clipboard_set_cb(lua_State* L)
{
    clip::clear();
    setdata(L, 1, clip::FORMAT_TEXT);
    setdata(L, 2, clip::FORMAT_WG);

    return 0;
}

void clipboard_init()
{
    const static luaL_Reg funcs[] = {
        {"clipboard_clear", clipboard_clear_cb},
        {"clipboard_get",   clipboard_get_cb  },
        {"clipboard_set",   clipboard_set_cb  },
        {NULL,              NULL              }
    };

    luaL_register(L, "wg", funcs);
}

// vim: ts=4 sw=4 et
