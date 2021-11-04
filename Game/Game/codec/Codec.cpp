#include "Codec.h"

#include <type_traits>
#include <memory>
#include "../log/Log.h"
#include "../Libs/lua/lua.hpp"



//using namespace google::protobuf;
//using namespace google::protobuf::compiler;

class ProtoErrorCollector : public google::protobuf::compiler::MultiFileErrorCollector
{
	virtual void AddError(const std::string& filename, int line, int column, const std::string& message)
	{
		printf("%s line %d, column %d : %s", filename.c_str(), line, column, message.c_str());
	}
};



ProtoErrorCollector        g_errorCollector;
google::protobuf::compiler::DiskSourceTree             g_sourceTree;
google::protobuf::compiler::Importer                   g_importer(&g_sourceTree, &g_errorCollector);
google::protobuf::DynamicMessageFactory      g_factory;

struct LuaMsg
{
	char* data_ = nullptr;
	size_t lens_ = 0;
	const char* name_;
	bool ok_ = false;
};


void Codec::ps(lua_State* L)
{
	int i;	int top = lua_gettop(L);
	printf("stackDump(num=%d):\n", top);
	for (i = 1; i <= top; i++)
	{  /* repeat for each level */
		int t = lua_type(L, i);
		switch (t)
		{
		case LUA_TSTRING:  /* strings */
			printf("`%s'", lua_tostring(L, i));
			break;
		case LUA_TBOOLEAN:  /* booleans */
			printf(lua_toboolean(L, i) ? "true" : "false");
		break;		case LUA_TNUMBER:  /* numbers */
			printf("%g", lua_tonumber(L, i));
			break;
		default:  /* other values */
			printf("%s", lua_typename(L, t));
			break;
		}
		printf("  ");
		/* put a separator */
	}
	printf("\n");
	/* end the listing */
}

bool Codec::decodeMessage(const google::protobuf::Message* mess, const google::protobuf::Descriptor* desc, lua_State* l)
{
	ps(l);
	int cnt = desc->field_count();
	lua_createtable(l, 0, cnt);
	for (int i = 0; i < cnt; ++i)
	{
		const google::protobuf::FieldDescriptor* fieledDesc = desc->field(i);
		bool ok = decodeField(mess, fieledDesc, l);
		if (!ok)
		{
			logDebug("decodeMessage err");
			return false;
		}
		lua_setfield(l, -2, fieledDesc->name().c_str());
	}
	return true;
}

bool Codec::decode(const char* name, char* data, size_t lens, lua_State* l)
{
	const google::protobuf::Descriptor* desc = g_importer.pool()->FindMessageTypeByName(name);
	if (!desc)
	{
		logDebug("decode no find name = %s", name);
		return false;
	}

	const google::protobuf::Message* mess = g_factory.GetPrototype(desc);
	if (!mess)
	{
		logDebug("decode no find mess");
		return false;
	}

	std::unique_ptr<google::protobuf::Message> message(mess->New());
	message->ParseFromArray(data, (int)lens);
	decodeMessage(message.get(), desc, l);
	return true;
}

bool Codec::decodeField(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l)
{
	if (fieldDesc->is_map())
	{
		return decodeMap(mess, fieldDesc, l);
	}
	else if (fieldDesc->is_repeated())
	{
		return decodeRepeated(mess, fieldDesc, l);
	}
	else if (fieldDesc->is_optional() || fieldDesc->is_required())
	{
		return decodeOptional(mess, fieldDesc, l);
	}

	return false;
}

bool Codec::decodeMap(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l)
{
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	const google::protobuf::Descriptor* desc = fieldDesc->message_type();
	if (desc->field_count() != 2)
	{
		logDebug("decodeMap err desc->field_count() != 2, name = %s", desc->full_name().c_str());
		return false;
	}

	const google::protobuf::FieldDescriptor* key = desc->field(0);
	const google::protobuf::FieldDescriptor* val = desc->field(1);
	int cnt = reftion->FieldSize(*mess, fieldDesc);
	lua_createtable(l, 0, cnt);

	for (int i = 0; i < cnt; ++i)
	{
		const google::protobuf::Message* subMess = &reftion->GetRepeatedMessage(*mess, fieldDesc, i);
		if (!decodeField(subMess, key, l))
		{
			return false;
		}
		if (!decodeField(subMess, val, l))
		{
			return false;
		}
		lua_rawset(l, -3);
	}
	return true;
}

bool Codec::decodeRepeated(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l)
{
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	int cnt = reftion->FieldSize(*mess, fieldDesc);
	lua_createtable(l, cnt, 0);
	for (int i = 0; i < cnt; ++i)
	{
		if (!decoderRepeatedField(mess, fieldDesc, l, i))
		{
			return false;
		}
		lua_seti(l, -2, (int)(i + 1));
	}
	return true;
}

bool Codec::decodeOptional(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l)
{
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	if (!reftion->HasField(*mess, fieldDesc))
	{
		logDebug("decodeOptional no find field, fieldname = %s", fieldDesc->full_name().c_str());
		//return false;
	}

	switch (fieldDesc->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		lua_pushnumber(l, reftion->GetDouble(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		lua_pushnumber(l, reftion->GetFloat(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		printf("%d\n", reftion->GetInt32(*mess, fieldDesc));
		lua_pushinteger(l, reftion->GetInt32(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		lua_pushinteger(l, reftion->GetUInt32(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		lua_pushinteger(l, reftion->GetInt64(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		lua_pushinteger(l, reftion->GetUInt64(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		lua_pushinteger(l, reftion->GetEnumValue(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		lua_pushboolean(l, reftion->GetBool(*mess, fieldDesc));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
	{
		const std::string& str = reftion->GetString(*mess, fieldDesc);
		lua_pushlstring(l, str.c_str(), str.size());
		return true;
	}
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		const google::protobuf::Message* subMess = &reftion->GetMessage(*mess, fieldDesc);
		return decodeMessage(subMess, fieldDesc->message_type(), l);
	}
	default:
		logDebug("decodeOptional no find type, fieldname = %s", fieldDesc->full_name().c_str());
		return false;
	}
}

bool Codec::decoderRepeatedField(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int index)
{
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	switch (fieldDesc->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		lua_pushnumber(l, (double)reftion->GetRepeatedDouble(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		lua_pushnumber(l, (float)reftion->GetRepeatedFloat(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		lua_pushinteger(l, (int)reftion->GetRepeatedInt32(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		lua_pushinteger(l, (google::protobuf::int64)reftion->GetRepeatedInt64(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		lua_pushinteger(l, (google::protobuf::uint32)reftion->GetRepeatedUInt32(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		lua_pushinteger(l, (google::protobuf::uint64)reftion->GetRepeatedUInt64(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		lua_pushinteger(l, reftion->GetRepeatedEnumValue(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		lua_pushboolean(l, reftion->GetRepeatedBool(*mess, fieldDesc, index));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
	{
		const std::string& str = reftion->GetRepeatedString(*mess, fieldDesc, index);
		lua_pushlstring(l, str.c_str(), str.length());
		return true;
	}
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		const google::protobuf::Message* subMess = &reftion->GetRepeatedMessage(*mess, fieldDesc, index);
		return decodeMessage(subMess, fieldDesc->message_type(), l);
	}
	default:
		logDebug("decoderRepeatedField err, no find type, name = %s", fieldDesc->full_name().c_str());
		return false;
	}
}

bool Codec::encoderMessage(google::protobuf::Message* mess, const google::protobuf::Descriptor* desc, lua_State* l, int idx)
{
	int fieldCnt = desc->field_count();
	for (int i = 0; i < fieldCnt; ++i)
	{
		const google::protobuf::FieldDescriptor* fieldDesc = desc->field(i);
		lua_getfield(l, idx, fieldDesc->name().c_str());// 字段= fieldDesc->name()的值压入栈顶	
		bool ok = encoderField(mess, fieldDesc, l, lua_absindex(l, -1));
		lua_pop(l, 1);
		if (!ok)
		{
			return false;
		}
	}
	return true;
}

bool Codec::encoderField(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx)
{
	//cout << "fieldname = " << fieldDesc->name() << endl;
	//cout << " is_repeated " << fieldDesc->is_repeated() << " is_map " << fieldDesc->is_map() << " is_optional " << fieldDesc->is_optional() << " is_required " << fieldDesc->is_required() << endl;
	//cout << fieldDesc->type() << endl;
	if (fieldDesc->is_map())
	{
		return encoderMap(mess, fieldDesc, l, idx);
	}
	else if (fieldDesc->is_repeated())
	{
		return encoderRepeated(mess, fieldDesc, l, idx);
	}
	else if (fieldDesc->is_optional())
	{
		return encoderOptional(mess, fieldDesc, l, idx);
	}
	else
	{
		logDebug("encoderField no reg field");
		return false;
	}
	return true;
}

bool Codec::encoderRepeatedField(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx)
{
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	switch (fieldDesc->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		reftion->AddDouble(mess, fieldDesc, (double)lua_tonumber(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		reftion->AddFloat(mess, fieldDesc, (float)lua_tonumber(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		reftion->AddInt32(mess, fieldDesc, (google::protobuf::int32)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		reftion->AddUInt32(mess, fieldDesc, (google::protobuf::uint32)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		reftion->AddInt64(mess, fieldDesc, (google::protobuf::int64)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		reftion->AddUInt64(mess, fieldDesc, (google::protobuf::uint64)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		reftion->AddEnumValue(mess, fieldDesc, (int)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		reftion->AddBool(mess, fieldDesc, lua_toboolean(l, idx) != 0);
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
	{
		size_t dataLen = 0;
		const char* data = lua_tolstring(l, idx, &dataLen);
		reftion->AddString(mess, fieldDesc, std::string(data, dataLen));
		return true;
	}
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		google::protobuf::Message* submessage = reftion->AddMessage(mess, fieldDesc);
		encoderMessage(submessage, fieldDesc->message_type(), l, idx);
		return true;
	}
	default:
		logDebug("encoderRepeatedField err, field = %s", fieldDesc->full_name().c_str());
		return false;
	}
}

bool Codec::encoderMap(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx)
{
	if (lua_isnil(l, idx))
	{
		logDebug("encoderMap err nil val, field name = %s", fieldDesc->full_name().c_str());
		return false;
	}
	if (!lua_istable(l, idx))
	{
		logDebug("encoderMap err nil val, field name = %s", fieldDesc->full_name().c_str());
		return false;
	}
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	const google::protobuf::Descriptor* desc = fieldDesc->message_type();
	if (desc->field_count() != 2)
	{
		logDebug("encoderMap err desc->field_count() != 2, name = %s, cnt = %d", fieldDesc->full_name().c_str(), desc->field_count());
		return false;
	}
	const google::protobuf::FieldDescriptor* key = desc->field(0);
	const google::protobuf::FieldDescriptor* val = desc->field(1);
	lua_pushnil(l);
	while (lua_next(l, idx))
	{
		google::protobuf::Message* submess = reftion->AddMessage(mess, fieldDesc);

		bool ok = encoderField(submess, key, l, lua_absindex(l, -2));
		if (!ok)
		{
			lua_pop(l, 2);
			return false;
		}
		ok = encoderField(submess, val, l, lua_absindex(l, -1));
		if (!ok)
		{
			lua_pop(l, 2);
			return false;
		}
		lua_pop(l, 1);
	}
	return true;
}

bool Codec::encoderRepeated(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx)
{
	if (lua_isnil(l, idx))
	{
		logDebug("encoderRepeated err nil val, field name = %s", fieldDesc->full_name().c_str());
		return false;
	}
	if (!lua_istable(l, idx))
	{
		logDebug("encoderRepeated err nil val, field name = %s", fieldDesc->full_name().c_str());
		return false;
	}

	int cnt = (int)luaL_len(l, idx);
	for (int i = 1; i <= cnt; ++i)
	{
		lua_geti(l, idx, i); // 当前lua数组的第i个数据放到栈顶		
		bool ok = encoderRepeatedField(mess, fieldDesc, l, lua_absindex(l, -1));
		lua_pop(l, 1);
		if (!ok)
		{
			return false;
		}
	}
	return true;
}

bool Codec::encoderOptional(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx)
{
	if (lua_isnil(l, idx))
	{
		logDebug("encoderOptional err nil val, field name = %s", fieldDesc->full_name().c_str());
		return false;
	}
	const google::protobuf::Reflection* reftion = mess->GetReflection();
	switch (fieldDesc->cpp_type())
	{
	case google::protobuf::FieldDescriptor::CPPTYPE_DOUBLE:
		reftion->SetDouble(mess, fieldDesc, (double)lua_tonumber(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_FLOAT:
		reftion->SetFloat(mess, fieldDesc, (float)lua_tonumber(l, idx));
		return true;;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT32:
		reftion->SetInt32(mess, fieldDesc, (google::protobuf::int32)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_INT64:
		reftion->SetInt64(mess, fieldDesc, (google::protobuf::int64)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT32:
		reftion->SetUInt32(mess, fieldDesc, (google::protobuf::uint32)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_UINT64:
		reftion->SetUInt64(mess, fieldDesc, (google::protobuf::uint64)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_ENUM:
		reftion->SetEnumValue(mess, fieldDesc, (int)lua_tointeger(l, idx));
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_BOOL:
		reftion->SetBool(mess, fieldDesc, lua_toboolean(l, idx) != 0);
		return true;
	case google::protobuf::FieldDescriptor::CPPTYPE_STRING:
	{
		size_t len = 0;
		const char* data = lua_tolstring(l, idx, &len);
		reftion->SetString(mess, fieldDesc, std::string(data, len));
		return true;
	}
	case google::protobuf::FieldDescriptor::CPPTYPE_MESSAGE:
	{
		google::protobuf::Message* subMess = reftion->MutableMessage(mess, fieldDesc);
		bool ok = encoderMessage(subMess, fieldDesc->message_type(), l, idx);
		return true;
	}
	default:
		logDebug("encoderOptional err name = %s", fieldDesc->full_name().c_str());
		return false;
	}
}

char* Codec::encoder(const char* name, lua_State* l, int idx, size_t& lens)
{
	const google::protobuf::Descriptor* desc = g_importer.pool()->FindMessageTypeByName(name);
	if (!desc)
	{
		logDebug("no find desc");
		return nullptr;
	}
	const google::protobuf::Message* mess = g_factory.GetPrototype(desc);
	if (!mess)
	{
		logDebug("no find mess");
		return nullptr;
	}

	std::unique_ptr<google::protobuf::Message> message(mess->New());
	if (!encoderMessage(message.get(), desc, l, idx))
	{
		logDebug("encoderMessage err");
		return nullptr;
	}

	lens = message->ByteSizeLong();
	char* buf = (char*)malloc(lens);
	message->SerializeToArray(buf, (int)lens);
	return buf;
}
