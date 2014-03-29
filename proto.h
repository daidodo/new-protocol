#ifndef DOZERG_PROTO_H_20110623
#define DOZERG_PROTO_H_20110623

#include "protocol_inner.h"

/* TODO:
	(DONE)tlv
	(DONE)struct as value
	(DONE)array as value
	(DONE)TSA: tag size array
	(DONE)optional item
	byte order

	APIs:
	V		name vt/st				cmd.Set(name.name, val)
	LV		name lt vt/st
	TLV		name tt tag lt vt/st
	TV		name tt tag vt/st
	A		name vt/st sz			cmd.AddElem(name, val), cmd.AddElem(name, cmd)
	LA		name lt vt/st [sz]
	TLA		name lt vt/st [sz]
	SA		name vt/st [sz]
	TSA		name vt/st [sz]
*/

#define PROTO_VER	1

class CProtocolInner;
class CCmdInner;
class CCmd;

class CProtocol
{
	friend class CCmd;
	CSharedPtr<CProtocolInner> proto_;
public:
	CProtocol();
	//value
	void AddV(const std::string & name, EDataType valType);
	void AddLv(const std::string & name, EDataType lenType, EDataType valType);
	void AddTlv(const std::string & name, EDataType tagType, int tag, EDataType lenType, EDataType valType, bool required = false);
	void AddTv(const std::string & name, EDataType tagType, int tag, EDataType valType, bool required = false);
	//struct
	void AddV(const std::string & name, const CProtocol & proto);
	void AddLv(const std::string & name, EDataType lenType, const CProtocol & proto);
	void AddTlv(const std::string & name, EDataType tagType, int tag, EDataType lenType, const CProtocol & proto, bool required = false);
	void AddTv(const std::string & name, EDataType tagType, int tag, const CProtocol & proto, bool required = false);
	//array
	void AddArray(const std::string & name, EDataType valType, size_t sz);
	void AddLa(const std::string & name, EDataType lenType, EDataType valType, size_t sz = 0);
	void AddTla(const std::string & name, EDataType tagType, int tag, EDataType lenType, EDataType valType, bool required = false, size_t sz = 0);
	void AddSa(const std::string & name, EDataType sizeType, EDataType valType, size_t sz = 0);
	void AddTsa(const std::string & name, EDataType tagType, int tag, EDataType sizeType, EDataType valType, bool required = false, size_t sz = 0);
	//struct array
	void AddArray(const std::string & name, const CProtocol & proto, size_t sz);
	void AddLa(const std::string & name, EDataType lenType, const CProtocol & proto, size_t sz = 0);
	void AddTla(const std::string & name, EDataType tagType, int tag, EDataType lenType, const CProtocol & proto, bool required = false, size_t sz = 0);
	void AddSa(const std::string & name, EDataType sizeType, const CProtocol & proto, size_t sz = 0);
	void AddTsa(const std::string & name, EDataType tagType, int tag, EDataType sizeType, const CProtocol & proto, bool required = false, size_t sz = 0);
	//debug
	std::string ToString() const;
};

typedef CProtocol CStruct;

class CCmd
{
	typedef CSharedPtr<CCmdInner> __CmdInner;
	explicit CCmd(__CmdInner cmd);
public:
	explicit CCmd(const CProtocol & proto);
	//set
	//value
	void Set(const std::string & name, char val);
	void Set(const std::string & name, signed char val);
	void Set(const std::string & name, unsigned char val);
	void Set(const std::string & name, short val);
	void Set(const std::string & name, unsigned short val);
	void Set(const std::string & name, int val);
	void Set(const std::string & name, unsigned int val);
	void Set(const std::string & name, long val);
	void Set(const std::string & name, unsigned long val);
	void Set(const std::string & name, long long val);
	void Set(const std::string & name, unsigned long long val);
	void Set(const std::string & name, const std::string & val);
	//array
	void AddElem(const std::string & name, char val);
	void AddElem(const std::string & name, signed char val);
	void AddElem(const std::string & name, unsigned char val);
	void AddElem(const std::string & name, short val);
	void AddElem(const std::string & name, unsigned short val);
	void AddElem(const std::string & name, int val);
	void AddElem(const std::string & name, unsigned int val);
	void AddElem(const std::string & name, long val);
	void AddElem(const std::string & name, unsigned long val);
	void AddElem(const std::string & name, long long val);
	void AddElem(const std::string & name, unsigned long long val);
	void AddElem(const std::string & name, const std::string & val);
	CCmd AddElemCmd(const std::string & name);
	//get
	bool IsSet(const std::string & name) const;	//is field set
	int GetTag(const std::string & name) const;
	//value
	int GetInt(const std::string & name) const;
	int64_t GetInt64(const std::string & name) const;
	std::string GetStr(const std::string & name) const;
	//array
	size_t GetArraySize(const std::string & name) const;
	int GetElemInt(const std::string & name, size_t index) const;
	int64_t GetElemInt64(const std::string & name, size_t index) const;
	std::string GetElemStr(const std::string & name, size_t index) const;
	CCmd GetElemCmd(const std::string & name, size_t index) const;		//fraction
	//en/decode
	bool Encode(COutStream & out) const;
	bool Decode(CInStream & in);
	//debug
	std::string ToString() const;
private:
	__CmdInner cmd_;
};

#endif

