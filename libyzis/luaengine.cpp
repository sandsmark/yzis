/* This file is part of the Yzis libraries
*  Copyright (C) 2004-2005 Mickael Marchand <mikmak@yzis.org>
*
*  This library is free software; you can redistribute it and/or
*  modify it under the terms of the GNU Library General Public
*  License as published by the Free Software Foundation; either
*  version 2 of the License, or (at your option) any later version.
*
*  This library is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
*  Library General Public License for more details.
*
*  You should have received a copy of the GNU Library General Public License
*  along with this library; see the file COPYING.LIB.  If not, write to
*  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
*  Boston, MA 02110-1301, USA.
**/

#include "luaengine.h"
#include "luafuncs.h"
#include "luaregexp.h"
#include "debug.h"
#include "buffer.h"
#include "session.h"
#include "yzis.h"
#include "portability.h"

class YView;

/*
 * TODO:
 * - invert line/col arguments
 * - test every argument of the functions
 * - find how to add file:line info to error messages
 * - override print() in lua for yzis
 * - clear the lua stack properly
 * - arguments to :source must be passed as argv
 * - add missing function from vim
 * - clean up all the lua calling functions
 * - yzpcall does not need to take nresult argument
 * - use correct printf api
 * - yzpcall should display the type of lua error when error occurs
 * - understand and document the highlight command
 *
 */

extern "C" {
#include <lauxlib.h>
#include <lualib.h>
}

#define deepdbg() yzDeepDebug("YLuaEngine")
#define dbg() yzDebug("YLuaEngine")
#define err() yzError("YLuaEngine")

using namespace yzis;

QString YLuaEngine::lua_value_to_string(lua_State *L, int index, int depth, bool type_only)
{
    // dbg().SPrintf( "lua_value_to_string(index=%d, depth=%d, type_only=%d", index, depth, type_only );
    QString s(depth * 2, ' ');

    switch (lua_type(L, index)) {
    case LUA_TNIL:
        s += "nil";
        break;

    case LUA_TNUMBER:
        s += QString("number: %1").arg(lua_tonumber(L, index));
        break;

    case LUA_TBOOLEAN:
        s += QString("boolean: %1").arg(lua_toboolean(L, index));
        break;

    case LUA_TSTRING:
        s += QString("string: '%1'").arg(lua_tostring(L, index));
        break;

    case LUA_TTABLE:
        if (type_only) {
            s += "table";
        } else {
            s += lua_table_to_string(L, index, depth + 1);
        }

        break;

    case LUA_TFUNCTION:
        s += "function";
        break;

    case LUA_TUSERDATA:
        s += "userdata";
        break;

    case LUA_TTHREAD:
        s += "thread";
        break;

    case LUA_TLIGHTUSERDATA:
        s += "light user data:";
        break;

    default:
        err().SPrintf("Unknown lua type: %d\n", lua_type(L, index));
        s += "Unknown lua type";
    }

    return s;
}

QString YLuaEngine::lua_table_to_string(lua_State *L, int index, int depth)
{
    // dbg().SPrintf( "lua_table_to_string( index=%d depth=%d )", index, depth );
    QString prefix(depth * 2, ' ');
    QString s;
    QMap<QString, QString> content;

    if (depth > 5) {
        err().SPrintf("lua_table_to_string - recursion error");
        return s;
    }

    s += prefix + "table: {\n";
    // put table on top of stack
    lua_pushvalue(L, index);
    lua_pushnil(L); // first key

    while (lua_next(L, -2) != 0) {
        bool isG;
        QString key, value;
        // key is at index -2, value at index -1
        key = lua_value_to_string(L, -2, depth + 1);
        // dbg() << "key='" << key << "'" << endl;
        isG = lua_isstring(L, -2) && QString("_G") == lua_tostring(L, -2);
        value = lua_value_to_string(L, -1, depth + 1, isG);
        // dbg() << "value='" << value << "'" << endl;
        content[key] = value;
        lua_pop(L, 1); // remove value, keep key for next iteration
    }

    // on the last call, nothing is put on the stack, so nothing to clean
    // remove table from stack
    lua_pop(L, 1);
    // dbg() << HERE() << "step 5" << endl;
    QList<QString> keys = content.keys();
    std::sort(keys.begin(), keys.end());

    for (QString key : keys) {
        s += prefix + "  " + key + " -> " + content[key] + '\n';
    }

    s += prefix + "} ";
    // dbg().SPrintf( "lua_table_to_string done - depth=%d stack_size=%d\n",  depth, lua_gettop(L) );
    return s;
}

void YLuaEngine::print_lua_stack_value(lua_State *L, int index, bool type_only)
{
    deepdbg().SPrintf("print_lua_stack_value(index=%d, type_only=%d)\n", index, type_only);
    deepdbg().SPrintf("stack value %d: %s", index, qp(lua_value_to_string(L, index, 0, type_only)));
}

void YLuaEngine::print_lua_stack(lua_State *L, const char *msg, bool type_only)
{
    deepdbg().SPrintf("print_lua_stack(msg=%s, type_only=%d)\n", msg, type_only);
    deepdbg().SPrintf("Stack (type_only=%d) - '%s' \n", type_only, msg);

    for (int i = 1; i <= lua_gettop(L); i++) {
        print_lua_stack_value(L, i, type_only);
    }
}

YLuaEngine *YLuaEngine::me = nullptr;

void YLuaEngine::destroy()
{
    delete YLuaEngine::me;
    YLuaEngine::me = nullptr;
}

YLuaEngine *YLuaEngine::self()
{
    if (YLuaEngine::me == 0) {
        YLuaEngine::me = new YLuaEngine();
        YLuaEngine::self()->init();
    }

    return YLuaEngine::me;
}

YLuaEngine::YLuaEngine()
{
}

void YLuaEngine::init()
{
    L = luaL_newstate();
    luaL_openlibs(L);
    dbg() << LUA_VERSION << " loaded" << endl;
    // luaopen leaves some garbage on the stack
    cleanLuaStack(L);
    YLuaFuncs::registerLuaFuncs(L);
    YLuaRegexp::registerLuaRegexp(L);
    cleanLuaStack(L);
    dbg() << HERE() << " done" << endl;
}

YLuaEngine::~YLuaEngine()
{
    lua_close(L);
    dbg() << HERE() << " done" << endl;
}

void YLuaEngine::cleanLuaStack(lua_State *L)
{
    lua_pop(L, lua_gettop(L));
    dbg() << HERE() << " done" << endl;
}

QString YLuaEngine::lua(YView *, const QString &args)
{
    dbg().SPrintf("lua( view, args=%s )", qp(args));
    execInLua(args);
    return QString();
}

//see Lua's PIL chapter 25.3 for how to use this :)
void YLuaEngine::exe(const QString &function, const char *sig, ...)
{
    dbg() << "YLuaEngine::exe( " << function << " ) sig : " << sig << endl;
    va_list vl;
    int narg, nres;
    va_start(vl, sig);
    lua_getglobal(L, function.toUtf8());
    narg = 0;

    while (*sig) {
        switch (*sig++) {
        case 'd':
            lua_pushnumber(L, va_arg(vl, double));
            break;

        case 'i':
            lua_pushnumber(L, va_arg(vl, int));
            break;

        case 's':
            lua_pushstring(L, va_arg(vl, char *));
            break;

        case '>':
            goto endwhile;
            break;

        default:
            //error
            break;
        }

        narg++;
        luaL_checkstack(L, 1, "too many arguments");
    }

endwhile:
    nres = strlen(sig);

    if (!yzpcall(narg, nres, QObject::tr("Executing function %1").arg(function))) {
        va_end(vl);
        return;
    }

    nres = -nres;

    while (*sig) {
        switch (*sig++) {
        case 'd':
            if (!lua_isnumber(L, nres)) {
                (void)0; //error
            }

            *va_arg(vl, double *) = lua_tonumber(L, nres);
            break;

        case 'i':
            if (!lua_isnumber(L, nres)) {
                (void)0; //error
            }

            *va_arg(vl, int *) = (int)lua_tonumber(L, nres);
            break;

        case 's':
            if (!lua_isstring(L, nres)) {
                (void)0; //error
            }

            *va_arg(vl, const char **) = lua_tostring(L, nres);
            break;

        default:
            //error
            break;
        }

        nres++;
    }

    va_end(vl);
}

void YLuaEngine::execute(const QString &function, int nbArgs, int nbResults)
{
    dbg().SPrintf("execute( function=%s, nbArgs=%d, nbResults=%d", qp(function), nbArgs, nbResults);
    lua_getglobal(L, function.toUtf8());
    yzpcall(nbArgs, nbResults, QObject::tr("YLuaEngine::execute function %1").arg(function));
}

QString YLuaEngine::source(const QString &filename)
{
    dbg().SPrintf("source( '%s' )\n", qp(filename));
    luaReturnValue = "";
    lua_pushstring(L, "return");
    //    lua_pushstring(L, "dofile");
    //    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_getglobal(L, "dofile");
    lua_pushstring(L, filename.toUtf8());
    /*bool success = */ yzpcall(1, 1, QObject::tr("Lua error when running file %1:\n").arg(filename));
    cleanLuaStack(L); // in case sourcing the file left something on the stack
    return luaReturnValue;
}

int YLuaEngine::execInLua(const QString &luacode)
{
    deepdbg().SPrintf("execInLua( %s )", qp(luacode));
    //    lua_pushstring(L, "loadstring" );
    //    lua_gettable(L, LUA_GLOBALSINDEX);
    lua_getglobal(L, "loadstring");
    lua_pushstring(L, luacode.toUtf8());

    // print_lua_stack(L, "loadstring step 0");
    if (!yzpcall(1, 2, QObject::tr("Executing code in lua\n"))) {
        // Error in the call
        return 1;
    }

    // now, we have the result of lua_string on the stack:
    // should be either function to call + nil or nil + error message
    // print_lua_stack(L, "loadstring step 1");
    if (lua_isfunction(L, -2) && lua_isnil(L, -1)) {
        lua_pop(L, 1);

        if (!yzpcall(0, 0, "loadstring step 2")) {
            // Error in the call
            // print_lua_stack(L, "loadstring step 3");
            return 2;
        }

        return 0;
    }

    if (lua_isnil(L, -2) && lua_isstring(L, -1)) {
        // there was an error in loadstring
        err() << "Error during loadstring(): " << lua_tostring(L, -1) << endl;
        YSession::self()->guiPopupMessage(
            QString("Error when executing lua code:\n%1\n\nCode was:\n%2")
                .arg(lua_tostring(L, -1))
                .arg(luacode));
        lua_pop(L, 2);
        // print_lua_stack(L, "loadstring step 4");
        return 3;
    }

    err() << "Unknown return types after loadstring" << endl;
    return 4;
}

bool YLuaEngine::yzpcall(int nbArg, int nbReturn, const QString &context)
{
    dbg().SPrintf("yzpcall( %d, %d, %s )", nbArg, nbReturn, qp(context));
    int lua_err = lua_pcall(L, nbArg, nbReturn, 0);
    QString luaErrorMsg;

    if (!lua_err) {
        dbg().SPrintf("yzpcall() done successfully");
        return true; // call is successful
    }

    err().SPrintf("yzpcall() error");

    if (lua_isstring(L, -1)) {
        // an error message on the stack
        luaErrorMsg = QString::fromUtf8((char *)lua_tostring(L, lua_gettop(L)));
    } else if (lua_isfunction(L, -2)) {
        // error handler function on the stack at position -2
        lua_pop(L, 1);
        yzpcall(0, 0, QObject::tr("error handling function called from within yzpcall"));
    } else {
        // big error, we do not grok what happend
        print_lua_stack(L, "loadstring returns strange things");
        luaErrorMsg = "Unknown lua return type after loadstring";
    }

    QByteArray err = luaErrorMsg.toLatin1();
    err().SPrintf("pCall error: %s\n", err.data());
    YSession::self()->guiPopupMessage(context + '\n' + luaErrorMsg);
    return false;
}

void YLuaEngine::yzisprint(const QString &text)
{
    dbg().SPrintf("yzisprint( %s )\n", qp(text));
    // XXX to be implemented
}

bool YLuaEngine::checkFunctionArguments(lua_State *L, int argNbMin, int argNbMax, const char *functionName, const char *functionArgDesc)
{
    int n = lua_gettop(L);

    if (n >= argNbMin && n <= argNbMax) {
        return true;
    }

    QString errorMsg = QString("%1() called with %2 arguments but %3-%4 expected: %5").arg(functionName).arg(n).arg(argNbMin).arg(argNbMax).arg(functionArgDesc);
    err() << errorMsg << endl;
#if 1
    QByteArray e = errorMsg.toUtf8();
    lua_pushstring(L, e.data());
    lua_error(L);
#else
    YLuaEngine::self()->execInLua(QString("error(%1)").arg(errorMsg));
#endif
    return false;
}

QStringList YLuaEngine::getLastResult(int nb) const
{
    dbg() << HERE() << " nb=" << nb << endl;
    int n = lua_gettop(L);
    dbg() << "LUA: Stack has " << n << " entries" << endl;
    QStringList list;

    for (int i = -nb; i < 0; ++i) {
        int type = lua_type(L, i);
        dbg() << "Type for index " << i << ": " << type << endl;

        switch (type) {
        case LUA_TNUMBER:
            list << QString::number(lua_tonumber(L, i));
            break;

        case LUA_TBOOLEAN:
            list << QString((lua_toboolean(L, i) != 0) ? "true" : "false");
            break;

        case LUA_TSTRING:
            list << QString::fromUtf8((char *)lua_tostring(L, i));
            break;

        default:
            break;
        }

        //cleanup
        lua_pop(L, 1);
    }

    dbg() << "LUA: Result " << list << endl;
    return list;
}

void YLuaEngine::setLuaReturnValue(const QString &value)
{
    dbg() << "setLuaReturnValue( " << value << ")" << endl;
    luaReturnValue = value;
}
