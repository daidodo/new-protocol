#include "proto.h"
#include "protocol_inner.h"

CProtocol::CProtocol():proto_(new CProtocolInner){}

void CProtocol::AddV(const std::string & name, EDataType valType){
	assert(proto_);
	proto_->Add(name).SetValue(valType);
}

void CProtocol::AddLv(const std::string & name, EDataType lenType, EDataType valType){
	assert(proto_);
	proto_->Add(name).SetType(LV).SetLen(lenType).SetValue(valType);
}

void CProtocol::AddTlv(const std::string & name, EDataType tagType, int tag, EDataType lenType, EDataType valType, bool required){
	assert(proto_);
	proto_->Add(name).SetType(TLV).SetReuired(required).SetTag(tagType, tag).SetLen(lenType).SetValue(valType);
}

void CProtocol::AddTv(const std::string & name, EDataType tagType, int tag, EDataType valType, bool required){
	assert(proto_);
	proto_->Add(name).SetType(TV).SetReuired(required).SetTag(tagType, tag).SetValue(valType);
}

void CProtocol::AddV(const std::string & name, const CProtocol & proto){
	assert(proto_);
	proto_->Add(name).SetValue(proto.proto_);
}

void CProtocol::AddLv(const std::string & name, EDataType lenType, const CProtocol & proto){
	assert(proto_);
	proto_->Add(name).SetType(LV).SetLen(lenType).SetValue(proto.proto_);
}

void CProtocol::AddTlv(const std::string & name, EDataType tagType, int tag, EDataType lenType, const CProtocol & proto, bool required){
	assert(proto_);
	proto_->Add(name).SetType(TLV).SetReuired(required).SetTag(tagType, tag).SetLen(lenType).SetValue(proto.proto_);
}

void CProtocol::AddTv(const std::string & name, EDataType tagType, int tag, const CProtocol & proto, bool required){
	assert(proto_);
	proto_->Add(name).SetType(TV).SetReuired(required).SetTag(tagType, tag).SetValue(proto.proto_);
}

void CProtocol::AddArray(const std::string & name, EDataType valType, size_t sz){
	assert(proto_);
	proto_->Add(name).SetSize(sz).SetArray().SetValue(valType);
}

void CProtocol::AddLa(const std::string & name, EDataType lenType, EDataType valType, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(LV).SetLen(lenType).SetSize(sz).SetArray().SetValue(valType);
}

void CProtocol::AddTla(const std::string & name, EDataType tagType, int tag, EDataType lenType, EDataType valType, bool required, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(TLV).SetReuired(required).SetTag(tagType, tag).SetLen(lenType).SetSize(sz).SetArray().SetValue(valType);
}

void CProtocol::AddSa(const std::string & name, EDataType sizeType, EDataType valType, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(SA).SetSize(sizeType, sz).SetArray().SetValue(valType);
}

void CProtocol::AddTsa(const std::string & name, EDataType tagType, int tag, EDataType sizeType, EDataType valType, bool required, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(TSA).SetReuired(required).SetTag(tagType, tag).SetSize(sizeType, sz).SetArray().SetValue(valType);
}

void CProtocol::AddArray(const std::string & name, const CProtocol & proto, size_t sz){
	assert(proto_);
	proto_->Add(name).SetSize(sz).SetArray().SetValue(proto.proto_);
}

void CProtocol::AddLa(const std::string & name, EDataType lenType, const CProtocol & proto, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(LV).SetLen(lenType).SetSize(sz).SetArray().SetValue(proto.proto_);
}

void CProtocol::AddTla(const std::string & name, EDataType tagType, int tag, EDataType lenType, const CProtocol & proto, bool required, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(TLV).SetReuired(required).SetTag(tagType, tag).SetLen(lenType).SetSize(sz).SetArray().SetValue(proto.proto_);
}

void CProtocol::AddSa(const std::string & name, EDataType sizeType, const CProtocol & proto, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(SA).SetSize(sizeType, sz).SetArray().SetValue(proto.proto_);
}

void CProtocol::AddTsa(const std::string & name, EDataType tagType, int tag, EDataType sizeType, const CProtocol & proto, bool required, size_t sz){
	assert(proto_);
	proto_->Add(name).SetType(TSA).SetReuired(required).SetTag(tagType, tag).SetSize(sizeType, sz).SetArray().SetValue(proto.proto_);
}

std::string CProtocol::ToString() const
{
	assert(proto_);
	std::ostringstream oss;
	oss<<"{\n"
		<<proto_->ToString(TAB)
		<<"}\n";
	return oss.str();
}

static inline CValue & setValue(CCmdInner & cmd, const std::string & name){
	CCmdInner * c = &cmd;
	std::string n(name);
	for(size_t pos;;){
		pos = n.find('.');
		if(std::string::npos == pos)
			break;
		c = &*(c->SetValue(n.substr(0, pos)).SetStruct());
		n = n.substr(pos + 1);
	}
	return c->SetValue(n);
}

static inline const CValue & getValue(CSharedPtr<CCmdInner> cmd, const std::string & name){
	static const CValue EMPTY_VAL;
	assert(cmd);
	std::string n(name);
	for(size_t pos;;){
		pos = n.find('.');
		if(std::string::npos == pos)
			break;
		cmd = cmd->GetValue(n.substr(0, pos)).GetStruct();
		if(!cmd)
			return EMPTY_VAL;
		n = n.substr(pos + 1);
	}
	return cmd->GetValue(n);
}

static inline bool hasValue(CSharedPtr<CCmdInner> cmd, const std::string & name){
	assert(cmd);
	std::string n(name);
	for(size_t pos;;){
		pos = n.find('.');
		if(std::string::npos == pos)
			break;
		cmd = cmd->GetValue(n.substr(0, pos)).GetStruct();
		if(!cmd)
			return false;
		n = n.substr(pos + 1);
	}
	return cmd->HasValue(n);
}

CCmd::CCmd(__CmdInner cmd):cmd_(cmd){}

CCmd::CCmd(const CProtocol & proto):cmd_(new CCmdInner(proto.proto_)){}

#define __CMD_SET(type)	\
	void CCmd::Set(const std::string & name, type val){	\
		assert(cmd_);	\
		setValue(*cmd_, name).SetInt(val);	\
	}
#define __CMD_ADD_ELEM(type)	\
	void CCmd::AddElem(const std::string & name, type val){	\
		assert(cmd_);	\
		setValue(*cmd_, name).SetArray()->AddElem().SetInt(val);	\
	}

__CMD_SET(char)
__CMD_SET(signed char)
__CMD_SET(unsigned char)
__CMD_SET(short)
__CMD_SET(unsigned short)
__CMD_SET(int)
__CMD_SET(unsigned int)
__CMD_SET(long)
__CMD_SET(unsigned long)
__CMD_SET(long long)
__CMD_SET(unsigned long long)

__CMD_ADD_ELEM(char)
__CMD_ADD_ELEM(signed char)
__CMD_ADD_ELEM(unsigned char)
__CMD_ADD_ELEM(short)
__CMD_ADD_ELEM(unsigned short)
__CMD_ADD_ELEM(int)
__CMD_ADD_ELEM(unsigned int)
__CMD_ADD_ELEM(long)
__CMD_ADD_ELEM(unsigned long)
__CMD_ADD_ELEM(long long)
__CMD_ADD_ELEM(unsigned long long)

#undef __CMD_SET
#undef __CMD_ADD_ELEM

void CCmd::Set(const std::string & name, const std::string & val){
	assert(cmd_);
	setValue(*cmd_, name).SetStr(val);
}

void CCmd::AddElem(const std::string & name, const std::string & val){
	assert(cmd_);
	setValue(*cmd_, name).SetArray()->AddElem().SetStr(val);
}

CCmd CCmd::AddElemCmd(const std::string & name){
	assert(cmd_);
	return CCmd(setValue(*cmd_, name).SetArray()->AddElem().SetStruct());
}

bool CCmd::IsSet(const std::string & name) const
{
	assert(cmd_);
	return hasValue(cmd_, name);
}

int CCmd::GetTag(const std::string & name) const{
	assert(cmd_);
	return getValue(cmd_, name).GetTag();
}

int CCmd::GetInt(const std::string & name) const{
	assert(cmd_);
	return getValue(cmd_, name).GetInt<int>();
}

int64_t CCmd::GetInt64(const std::string & name) const{
	assert(cmd_);
	return getValue(cmd_, name).GetInt<int64_t>();
}

std::string CCmd::GetStr(const std::string & name) const{
	assert(cmd_);
	return getValue(cmd_, name).GetStr();
}

size_t CCmd::GetArraySize(const std::string & name) const{
	assert(cmd_);
	return getValue(cmd_, name).GetArray().Size();
}

int CCmd::GetElemInt(const std::string & name, size_t index) const{
	assert(cmd_);
	return getValue(cmd_, name).GetArray().GetElem(index).GetInt<int>();
}

int64_t CCmd::GetElemInt64(const std::string & name, size_t index) const{
	assert(cmd_);
	return getValue(cmd_, name).GetArray().GetElem(index).GetInt<int64_t>();
}

std::string CCmd::GetElemStr(const std::string & name, size_t index) const{
	assert(cmd_);
	return getValue(cmd_, name).GetArray().GetElem(index).GetStr();
}

CCmd CCmd::GetElemCmd(const std::string & name, size_t index) const{
	assert(cmd_);
	return CCmd(getValue(cmd_, name).GetArray().GetElem(index).GetStruct());
}

bool CCmd::Encode(COutStream & out) const{
	assert(cmd_);
	return cmd_->Encode(out);
}

bool CCmd::Decode(CInStream & in){
	assert(cmd_);
	return cmd_->Decode(in);
}

std::string CCmd::ToString() const
{
	assert(cmd_);
	std::ostringstream oss;
	oss<<"{\n"
		<<cmd_->ToString(TAB)
		<<"}\n";
	return oss.str();
}

std::string CValue::ToString(const std::string & name, const std::string & tab) const{
	std::ostringstream oss;
	if(item_){
		oss<<tab<<TypeName(item_->valType_)<<SEP
			<<name<<SEP
			<<"="<<SEP;
		switch(item_->valType_){
			case U8:
			case U16:
			case U32:oss<<GetInt<int>();break;
			case U64:oss<<GetInt<int64_t>();break;
			case STR:oss<<val_;break;
            case PROT:oss<<"{\n"<<(cmd_ ? cmd_->ToString(tab + TAB) : "NULL")<<tab<<"}";break;  //---------
			case ARR:assert(array_);oss<<"{\n"<<array_->ToString(tab + TAB)<<tab<<"}";break;
			default:;
		}
		oss<<ItemName(item_->itemType_, item_->required_);
	}
	oss<<"\n";
	return oss.str();
}

std::string CItemBase::ToString(const std::string & name, const std::string & tab) const{
	std::ostringstream oss;
	oss<<tab<<TypeName(valType_)<<SEP<<name;
	switch(valType_){
		case U8:
		case U16:
		case U32:
		case U64:
		case STR:break;
		case PROT:assert(proto_);oss<<SEP<<"="<<SEP<<"{\n"<<proto_->ToString(tab + TAB)<<tab<<"}";break;
		case ARR:assert(elem_);oss<<SEP<<"="<<SEP<<"{\n"<<elem_->ToString("[0]", tab + TAB)<<tab<<"}";break;
		default:;
	}
	oss<<ItemName(itemType_, required_)
		<<"\n";
	return oss.str();
}

bool CItemBase::encodeStruct(COutStream & out) const{
	return (proto_ ? proto_->Encode(out) : false);
}

CValue::__CmdInner CValue::SetStruct(){
	if(!cmd_){
		cmd_ = new CCmdInner;
		if(item_ && PROT == item_->valType_)
			cmd_->SetProtocol(item_->proto_);
	}
	return cmd_;
}

CValue::__Array CValue::SetArray(){
	if(!array_){
		array_ = new CArray;
		if(item_ && ARR == item_->valType_)
			array_->SetType(&*item_->elem_);
	}
	return array_;
}

const CArray & CValue::GetArray() const{
	static const CArray EMPTY_ARR;
	if(!array_)
		return EMPTY_ARR;
	return *array_;
}

bool CValue::encodeStruct(COutStream & out) const{
	assert(item_);
	if(PROT != item_->valType_ || !cmd_)
		return_false;	//empty struct
	return cmd_->Encode(out);
}

bool CValue::encodeArray(COutStream & out) const{
	assert(item_);
	if(ARR != item_->valType_ || !array_)
		return_false;	//empty array
	if(V == item_->itemType_ && !item_->size_)
		return_false;	//array size unknown
	return array_->Encode(out, item_->size_);
}

bool CValue::encodeSA(COutStream & out) const{
	assert(item_);
	return (encodeInt(out, item_->sizeType_, array_->Size()) && encodeArray(out));
}

bool CValue::decodeStruct(CInStream & in){
	assert(item_);
	assert(PROT == item_->valType_);
	if(!cmd_){
		if(!item_->proto_)
			return_false;
		cmd_ = new CCmdInner(item_->proto_);
	}
	return cmd_->Decode(in);
}

bool CValue::decodeArray(CInStream & in, size_t sz){
	assert(item_);
	assert(ARR == item_->valType_);
	if(!sz)
		sz = item_->size_;
	if(item_->size_ && sz != item_->size_)
		return_false;	//array size mismatch
	if(!array_){
		if(!item_->elem_)
			return_false;	//array elem type unknown
		if(V == item_->itemType_ && !sz)
			return_false;	//array size unknown
		array_ = new CArray(&(*item_->elem_));
	}
	return array_->Decode(in, sz);
}
