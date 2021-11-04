#pragma once

#include <string>

#include "google/protobuf/dynamic_message.h"
#include "google/protobuf/compiler/importer.h"

struct lua_State;
class Descriptor;
class Message;
class FieldDescriptor;
struct LuaTable;



class Codec
{
public:
	inline void ps(lua_State* L);
	bool decodeMessage(const google::protobuf::Message* mess, const google::protobuf::Descriptor* desc, lua_State* l);
	void decode(LuaTable* luatable, lua_State* l);
	bool decodeField(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l);
	bool decodeMap(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l);
	bool decodeRepeated(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l);
	bool decodeOptional(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l);
	bool decoderRepeatedField(const google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int index);

	bool encoderMessage(google::protobuf::Message* mess, const google::protobuf::Descriptor* desc, lua_State* l, int idx);
	bool encoderField(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx);
	bool encoderRepeatedField(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx);
	bool encoderMap(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx);
	bool encoderRepeated(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx);
	bool encoderOptional(google::protobuf::Message* mess, const google::protobuf::FieldDescriptor* fieldDesc, lua_State* l, int idx);
	void encoder(const char* name, lua_State* l, int idx, LuaTable* luatable);
};
