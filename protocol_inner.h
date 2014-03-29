#ifndef DOZERG_PROTOCOL_H_20110619
#define DOZERG_PROTOCOL_H_20110619

#include <vector>
#include <string>
#include <cstring>
#include <cassert>
#include <stdint.h>
#include <sstream>
#include <map>

//debug tools
#if 1
#	define return_false	throw 1
#else
#	define return_false return false
#endif

#define TAB	"  "
#define SEP	" "

enum EDataType{
	NONE,
	U8, U16, U32, U64,
	STR,
	PROT, ARR
};

static inline const char * TypeName(EDataType t)
{
	const char * const NAME[] = {
		"NONE",
		"U8", "U16", "U32", "U64",
		"STR",
		"STRUCT", "ARRAY"
	};
	return NAME[t];
}

enum EItemType{
	V, LV, TLV, TV,
	SA, TSA
};

static inline const char * ItemName(EItemType t, bool required = false)
{
	switch(t){
		case V:
		case SA:break;
		case LV:return " [LV]";
		case TLV:return (required ? " [TLV, required]" : " [TLV]");
		case TV:return (required ? " [TV, required]" : " [TV]");
		case TSA:return (required ? " [TSA, required]" : " [TSA]");
		default:;
	}
	return "";
}

class COutStream
{
	std::vector<char> data_;
	size_t cur_;
public:
	explicit COutStream(size_t sz = 1000)
		: data_(sz)
		, cur_(0)
	{}
    void Clear(){cur_ = 0;}
	void Export(std::string & buf){
		data_.resize(cur_);
		buf.assign(data_.begin(), data_.end());
	}
	COutStream & operator <<(uint8_t v){return writePod(v);}
	COutStream & operator <<(uint16_t v){return writePod(v);}
	COutStream & operator <<(uint32_t v){return writePod(v);}
	COutStream & operator <<(uint64_t v){return writePod(v);}
	COutStream & operator <<(const std::string & v){
		return writeBuf(v.c_str(), v.size());
	}
private:
	void ensure(size_t sz){
		size_t curSz = data_.size();
		if(curSz < sz + cur_)
			data_.resize(curSz + (curSz >> 1) + sz);
	}
	template<typename T>
	COutStream & writePod(T v){
		ensure(sizeof(T));
		memcpy(&data_[cur_], &v, sizeof(T));
		cur_ += sizeof(T);
		return *this;
	}
	COutStream & writeBuf(const char * buf, size_t sz){
		assert(buf);
		if(sz){
			ensure(sz);
			memcpy(&data_[cur_], buf, sz);
			cur_ += sz;
		}
		return *this;
	}
};

class CInStream
{
	typedef CInStream __Myt;
	typedef bool (__Myt::*__SafeBool)(size_t);
	const char *	buf_;
	size_t			cur_, sz_;
	bool			good_;
public:
	CInStream(const char * buf, size_t size)
		: buf_(buf)
		, cur_(0)
		, sz_(size)
		, good_(true)
	{}
	explicit CInStream(const std::string & buf)
		: buf_(buf.c_str())
		, cur_(0)
		, sz_(buf.size())
		, good_(true)
	{}
	bool GetBuf(std::string & value, size_t sz){
		if(!sz)
			return true;
		if(!ensure(sz))
			return_false;
		value.resize(sz);
		memcpy(&value[0], &buf_[cur_], sz);
		cur_ += sz;
		return true;
	}
	bool operator !() const{return !good_;}
	operator __SafeBool() const{return operator !() ? 0 : &__Myt::ensure;}
	CInStream & operator >>(uint8_t & v){return readPod(v);}
	CInStream & operator >>(uint16_t & v){return readPod(v);}
	CInStream & operator >>(uint32_t & v){return readPod(v);}
	CInStream & operator >>(uint64_t & v){return readPod(v);}
	size_t Size() const{return sz_;}
	void Size(size_t sz){sz_ = sz;}
	size_t Left() const{return sz_ - cur_;}
	bool Good() const{return good_;}
	void Good(bool g){good_ = g;}
	size_t Cur() const{return cur_;}
	void SetCur(size_t cur){cur_ = cur;}
	void Reserve(size_t len){
		if(ensure(len))
			sz_ = cur_ + len;
	}
private:
	bool ensure(size_t len){
		return (good_ ? (cur_ + len <= sz_ ? true : (good_ = false)) : false);
	}
	template<typename T>
	CInStream & readPod(T & v){
		if(ensure(sizeof(T))){
			memcpy(&v, &buf_[cur_], sizeof(T));
			cur_ += sizeof(T);
		}
		return *this;
	}
};

class CInStreamStop
{
	CInStream & in_;
	size_t stop_;
public:
	CInStreamStop(CInStream & in, size_t len)
		: in_(in)
		, stop_(in.Size())
	{
		in_.Reserve(len);
	}
	~CInStreamStop(){in_.Size(stop_);}
};

class CInStreamFetch
{
	CInStream & in_;
	size_t cur_;
	bool good_;
public:
	explicit CInStreamFetch(CInStream & in)
		: in_(in)
		, cur_(in.Cur())
		, good_(in.Good())
	{}
	~CInStreamFetch(){
		if(cur_){
			in_.SetCur(cur_);
			in_.Good(good_);
		}
	}
	void Succ(){cur_ = 0;}
};

template<typename T>
static bool encodeInt(COutStream & out, EDataType type, T val)
{
	switch(type){
		case U8:out<<uint8_t(val);break;
		case U16:out<<uint16_t(val);break;
		case U32:out<<uint32_t(val);break;
		case U64:out<<uint64_t(val);break;
		default:return_false;
	}
	return true;
}

#define __DECODE_INT(type, in, val)	{	\
	type t;	\
	if(!(in>>t))	\
		return false;	\
	val = t;	\
}

template<typename T>
static bool decodeInt(CInStream & in, EDataType type,  T & val)
{
	switch(type){
		case U8:__DECODE_INT(uint8_t, in, val);break;
		case U16:__DECODE_INT(uint16_t, in, val);break;
		case U32:__DECODE_INT(uint32_t, in, val);break;
		case U64:__DECODE_INT(uint64_t, in, val);break;
		default:return false;
	}
	return true;
}

#undef __DECODE_INT

//return: 0-error; 1-no tag; 2-has tag
static int decodeTag(CInStream & in, EDataType type, int tag, bool required)
{
	CInStreamFetch f(in);
	int val;
	if(decodeInt(in, type, val))
		if(tag == val){
			f.Succ();
			return 2;
		}
	return required ? 0 : 1;
}

template<class Elem,class IntType>
struct _ref_imp{
    explicit _ref_imp(Elem * pe){
        ptr_ = pe;
        cnt_ = 1;
    }
    void addRef(_ref_imp *& p){
        p = this;
        ++cnt_;
    }
    static void changeRef(_ref_imp *& p,_ref_imp * v){
        if(p != v){
            subRef(p);
            if(v)
                v->addRef(p);
        }
    }
    static void subRef(_ref_imp *& p){
        if(p){
            if(!--(p->cnt_)){
				delete p->ptr_;
				delete p;
            }
            p = 0;
        }
    }
private:
    _ref_imp(const _ref_imp &);
    _ref_imp & operator =(const _ref_imp &);
public:
    Elem *  ptr_;
private:
    IntType cnt_;
};

template<class T>
class CSharedPtr
{   //typedefs
    typedef CSharedPtr<T> __Myt;
	typedef int __Int;
    typedef _ref_imp<T, __Int> __ref_type;
public:
    typedef T   value_type;
    typedef T & reference;
    typedef const T & const_reference;
    typedef T * pointer;
    typedef const T * const_pointer;
private:
    typedef void (__Myt::*safe_bool_type)(pointer);
public:
    //functions
    CSharedPtr(pointer p = 0):ref_(0){init(p);}
    CSharedPtr(const __Myt & a):ref_(0){__ref_type::changeRef(ref_, a.ref_);}
    ~CSharedPtr(){__ref_type::subRef(ref_);}
    __Myt & operator =(const __Myt & a){
        __ref_type::changeRef(ref_, a.ref_);
        return *this;
    }
    __Myt & operator =(pointer a){
        if(operator !=(a)){
            __ref_type::subRef(ref_);
            init(a);
        }
        return *this;
    }
    reference operator *(){return *ref_->ptr_;}
    pointer operator ->(){return ref_->ptr_;}
    const_reference operator *() const{return *ref_->ptr_;}
    const_pointer operator ->() const{return ref_->ptr_;}
    bool operator !() const{return !ref_;}
    operator safe_bool_type() const{return operator !() ? 0 : &__Myt::init;}
    bool operator ==(const __Myt & a) const{return ref_ == a.ref_;}
    bool operator !=(const __Myt & a) const{return !operator ==(a);}
    bool operator ==(pointer a) const{
        return operator !() ? !a : a == operator ->();
    }
    bool operator !=(pointer a) const{return !operator ==(a);}
    void swap(__Myt & a){std::swap(ref_, a.ref_);}
private:
    void init(pointer p){
        if(p)
            ref_ = new __ref_type(p);
    }
    //field
    __ref_type * ref_;
};

struct CProtocolInner;

struct CItemBase
{
	typedef CSharedPtr<CProtocolInner> __Proto;
	EItemType	itemType_;
	EDataType	tagType_;
	EDataType	lenType_;
	EDataType	sizeType_;
	EDataType	valType_;
	int tag_;
	__Proto proto_;		//struct as value
	CSharedPtr<CItemBase> elem_;	//array as value
	size_t size_;	//array size (No. of elems)
	bool required_;	//only for TLV, TV or TSA
	//functions
	CItemBase()
		: itemType_(V)
		, tagType_(NONE)
		, lenType_(NONE)
		, sizeType_(NONE)
		, valType_(NONE)
		, tag_(0)
		, proto_(0)
		, size_(0)
		, required_(false)
	{}
	CItemBase & SetType(EItemType t){itemType_ = t;return *this;}
	CItemBase & SetTag(EDataType t, int v){tagType_ = t;tag_ = v;return *this;}
	CItemBase & SetLen(EDataType t){lenType_ = t;return *this;}
	CItemBase & SetValue(EDataType t){valType_ = t;return *this;}
	CItemBase & SetValue(__Proto p){valType_ = PROT;proto_ = p;return *this;}
	CItemBase & SetSize(size_t size){size_ = size;return *this;}
	CItemBase & SetSize(EDataType t, size_t size){sizeType_ = t;size_ = size;return *this;}
	CItemBase & SetReuired(bool r){required_ = r;return *this;}
	CItemBase & SetArray(){
		valType_ = ARR;
		if(!elem_)
			elem_ = new CItemBase();
		return *elem_;
	}
	bool Encode(COutStream & out) const{
		switch(itemType_){
			case V:return encodeV(out);
			case LV:return encodeLV(out);
			case SA:return encodeSA(out);
			case TLV:
			case TV:
			case TSA:return true;
			default:;
		}
		return_false;
	}
	bool Required() const{return required_ ? true : (TLV != itemType_ && TV != itemType_ && TSA != itemType_);}
	std::string ToString(const std::string & name, const std::string & tab) const;
private:
	bool encodeV(COutStream & out) const{
		switch(valType_){
			case U8:
			case U16:
			case U32:
			case U64:return encodeInt(out, valType_, 0);
			case STR:break;
			case PROT:return encodeStruct(out);
			case ARR:return encodeArray(out);
			default:return_false;
		}
		return true;
	}
	bool encodeLV(COutStream & out) const{
		if(PROT == valType_ ||
				ARR == valType_){
			COutStream tmp;
			if(!encodeV(tmp))
				return_false;
			std::string buf;
			tmp.Export(buf);
			if(!encodeInt(out, lenType_, buf.size()))
				return_false;
			out<<buf;
			return true;
		}
		return encodeInt(out, lenType_, 0);
	}
	bool encodeSA(COutStream & out) const{
		return (encodeInt(out, sizeType_, size_) && encodeArray(out));
	}
	bool encodeStruct(COutStream & out) const;
	bool encodeArray(COutStream & out) const{
		if(!elem_)
			return false;
		for(size_t i = 0;i < size_;++i)
			if(!elem_->Encode(out))
				return false;
		return true;
	}
};

struct CItem : public CItemBase
{
	std::string	name_;
	CItem(){}
	explicit CItem(const std::string & name):name_(name){}
	std::string ToString(const std::string & tab) const{
		return CItemBase::ToString(name_, tab);
	}
};

struct CProtocolInner
{
	typedef std::vector<CItem> __Items;
	const __Items & Items() const{return items_;}
	const CItem * Find(const std::string & name) const{
		for(__Items::const_iterator i = items_.begin();i != items_.end();++i)
			if(name == i->name_)
				return &(*i);
		return 0;
	}
	CItem & Add(const std::string & name){
		items_.push_back(CItem(name));
		return items_.back();
	}
	bool Encode(COutStream & out) const{
		for(__Items::const_iterator i = items_.begin();i != items_.end();++i)
			if(!i->Encode(out))
				return false;
		return true;
	}
	std::string ToString(const std::string & tab) const{
		std::ostringstream oss;
		for(__Items::const_iterator i = items_.begin();i != items_.end();++i)
			oss<<i->ToString(tab);
		return oss.str();
	}
private:
	__Items	items_;
};

class CCmdInner;
class CArray;

class CValue
{
	typedef CSharedPtr<CCmdInner> __CmdInner;
	typedef CSharedPtr<CArray> __Array;
	const CItemBase * item_;
	std::string	val_;
	__CmdInner cmd_;
	__Array array_;
public:
	explicit CValue(const CItemBase * item = 0):item_(item){}
	//set
	template<typename T>
	bool SetInt(const T & val){
		if(!item_)
			return true;
		switch(item_->valType_){
			case U8:setInt(uint8_t(val));break;
			case U16:setInt(uint16_t(val));break;
			case U32:setInt(uint32_t(val));break;
			case U64:setInt(uint64_t(val));break;
			default:return_false;
		}
		return true;
	}
	bool SetStr(const std::string & val){
		if(!item_)
			return_false;
		switch(item_->valType_){
			case STR:val_ = val;break;
			default:return_false;
		}
		return true;
	}
	__CmdInner SetStruct();
	__Array SetArray();
	//get
	int GetTag() const{return item_ ? item_->tag_ : 0;}
	template<class T>
	T GetInt() const{
		if(item_){
			switch(item_->valType_){
				case U8:return getInt<T, uint8_t>();
				case U16:return getInt<T, uint16_t>();
				case U32:return getInt<T, uint32_t>();
				case U64:return getInt<T, uint64_t>();
				default:;
			}
		}
		return T();
	}
	std::string GetStr() const{
		if(item_){
			switch(item_->valType_){
				case STR:return val_;
				default:;
			}
		}
		return "";
	}
	__CmdInner GetStruct() const{return cmd_;}
	const CArray & GetArray() const;
	//const CValue & operator [](const std::string & name) const;
	bool Encode(COutStream & out) const{
		if(!item_)
			return_false;
		switch(item_->itemType_){
			case V:return encodeV(out);
			case LV:return encodeLV(out);
			case TLV:return encodeTLV(out);
			case TV:return encodeTV(out);
			case SA:return encodeSA(out);
			case TSA:return encodeTSA(out);
			default:;
		}
		return_false;
	}
	bool Decode(CInStream & in){
		if(!item_)
			return_false;
		switch(item_->itemType_){
			case V:return decodeV(in);
			case LV:return decodeLV(in);
			case TLV:return decodeTLV(in);
			case TV:return decodeTV(in);
			case SA:return decodeSA(in);
			case TSA:return decodeTSA(in);
			default:;
		}
		return_false;
	}
	std::string ToString(const std::string & name, const std::string & tab) const;
private:
	bool ensure(size_t sz) const{return sz == val_.size();}
	//encode
	bool encodeStruct(COutStream & out) const;
	bool encodeArray(COutStream & out) const;
	bool encodeV(COutStream & out) const{
		assert(item_);
		switch(item_->valType_){
			case U8:assert(ensure(1));break;
			case U16:assert(ensure(2));break;
			case U32:assert(ensure(4));break;
			case U64:assert(ensure(8));break;
			case STR:break;
			case PROT:return encodeStruct(out);
			case ARR:return encodeArray(out);
			default:return_false;
		}
		out<<val_;
		return true;
	}

	bool encodeLV(COutStream & out) const{
		assert(item_);
		if(PROT == item_->valType_ ||
				ARR == item_->valType_){
			COutStream tmp;
			if(!encodeV(tmp))
				return_false;
			std::string buf;
			tmp.Export(buf);
			if(!encodeInt(out, item_->lenType_, buf.size()))
				return_false;
			out<<buf;
			return true;
		}
		return encodeInt(out, item_->lenType_, val_.size()) && encodeV(out);
	}
	bool encodeTLV(COutStream & out) const{
		assert(item_);
		return (encodeInt(out, item_->tagType_, item_->tag_) && encodeLV(out));
	}
	bool encodeTV(COutStream & out) const{
		assert(item_);
		return (encodeInt(out, item_->tagType_, item_->tag_) && encodeV(out));
	}
	bool encodeSA(COutStream & out) const;
	bool encodeTSA(COutStream & out) const{
		assert(item_);
		return (encodeInt(out, item_->tagType_, item_->tag_) && encodeSA(out));
	}
	//decode
	bool decodeStruct(CInStream & in);
	bool decodeArray(CInStream & in, size_t sz = 0);
	bool decodeV(CInStream & in, size_t len = 0){
		assert(item_);
		switch(item_->valType_){
			case U8:in.GetBuf(val_, 1);break;
			case U16:in.GetBuf(val_, 2);break;
			case U32:in.GetBuf(val_, 4);break;
			case U64:in.GetBuf(val_, 8);break;
			case STR:in.GetBuf(val_, len);break;
			case PROT:return decodeStruct(in);
			case ARR:return decodeArray(in);
			default:return_false;
		}
		return true;
	}
	bool decodeLV(CInStream & in){
		assert(item_);
		size_t len;
		if(!decodeInt(in, item_->lenType_, len))
			return_false;
		CInStreamStop s(in, len);
		if(!decodeV(in, len))
			return_false;
		return true;
	}
	bool decodeTLV(CInStream & in){
		assert(item_);
		switch(decodeTag(in, item_->tagType_, item_->tag_, item_->required_)){
			case 1:return true;
			case 2:return decodeLV(in);
			default:;
		}
		return_false;
	}
	bool decodeTV(CInStream & in){
		assert(item_);
		switch(decodeTag(in, item_->tagType_, item_->tag_, item_->required_)){
			case 1:return true;
			case 2:return decodeV(in);
			default:;
		}
		return_false;
	}
	bool decodeSA(CInStream & in){
		assert(item_);
		size_t sz;
		if(!decodeInt(in, item_->sizeType_, sz))
			return_false;
		return decodeArray(in, sz);
	}
	bool decodeTSA(CInStream & in){
		assert(item_);
		switch(decodeTag(in, item_->tagType_, item_->tag_, item_->required_)){
			case 1:return true;
			case 2:return decodeSA(in);
			default:;
		}
		return_false;
	}
	template<class T, typename V>
	T getInt() const{
		if(ensure(sizeof(V)))
			return T(*reinterpret_cast<const V *>(&val_[0]));
		return T();
	}
	template<typename T>
	void setInt(T val){
		val_.resize(sizeof(T));
		memcpy(&val_[0], &val, sizeof(T));
	}
};

class CArray
{
	typedef std::vector<CValue> __Values;
	__Values values_;
	const CItemBase * elem_;	//element type
public:
	CArray(const CItemBase * elemType = 0):elem_(elemType){}
	size_t Size() const{return values_.size();}
	const CValue & GetElem(size_t index) const{return values_.at(index);}
	CValue & GetElem(size_t index){return values_.at(index);}
	void SetType(const CItemBase * elem){elem_ = elem;}
	CValue & AddElem(){
		values_.push_back(CValue(elem_));
		return values_.back();
	}
	bool Encode(COutStream & out, size_t size) const{
		if(size && size != values_.size())
			return_false;	//array size mismatch
		for(__Values::const_iterator i = values_.begin();i != values_.end();++i)
			if(!i->Encode(out))
				return_false;
		return true;
	}
	bool Decode(CInStream & in, size_t size){
		if(size){
			values_.reserve(size);
			for(size_t i = 0;i < size;++i){
				CValue v(elem_);
				size_t cur = in.Cur();
				if(!v.Decode(in))
					return_false;
				if(in.Cur() == cur)
					break;
				values_.push_back(v);
			}
		}else{
			for(;;){
				CValue v(elem_);
				size_t cur = in.Cur();
				if(!v.Decode(in))
					break;
				if(in.Cur() == cur)
					break;
				values_.push_back(v);
			}
			if(in.Left())
				return_false;
		}
		return true;
	}
	std::string ToString(const std::string & tab) const{
		std::ostringstream oss;
		for(size_t i = 0, e = values_.size();i < e;++i){
			std::ostringstream oss_tmp;
			oss_tmp<<"["<<i<<"]";
			oss<<values_[i].ToString(oss_tmp.str(), tab);
		}
		return oss.str();
	}
};

class CCmdInner
{
	typedef CProtocolInner::__Items __Items;
	typedef std::map<std::string, CValue> __Values;
	typedef CSharedPtr<CProtocolInner> __Proto;
	__Values values_;
	__Proto proto_;
public:
	CCmdInner(){}
	explicit CCmdInner(__Proto proto):proto_(proto){}
	void SetProtocol(__Proto proto){proto_= proto;}
	//set
	CValue & SetValue(const std::string & name){
		__Values::iterator wh = values_.find(name);
		if(wh != values_.end())
			return wh->second;
		const CItem * item = (proto_ ? proto_->Find(name) : 0);
		return addValue(name, CValue(item));
	}
	//get
	const CValue & GetValue(const std::string & name) const{
		static const CValue EMPTY_VAL;
		const CValue * value = findValue(name);
		if(!value)
			return EMPTY_VAL;
		return *value;
	}
	bool HasValue(const std::string & name) const{
		return findValue(name) ? true : false;
	}
	bool Encode(COutStream & out) const{
		if(!proto_)
			return_false;
		const __Items & items = proto_->Items();
		for(__Items::const_iterator i = items.begin();i != items.end();++i){
			const CValue * value = findValue(i->name_);
			if(value){
				if(!value->Encode(out))
					return_false;
			}else if(i->Required()){
				if(!i->Encode(out))
					return_false;
			}
		}
		return true;
	}
	bool Decode(CInStream & in){
		if(!proto_)
			return_false;
		const __Items & items = proto_->Items();
		for(__Items::const_iterator i = items.begin();i != items.end();++i){
			CValue value(&*i);
            size_t cur = in.Cur();
			if(!value.Decode(in))
				return_false;
            if(cur != in.Cur())
                addValue(i->name_, value);
		}
		return true;
	}
	std::string ToString(const std::string & tab) const{
		std::ostringstream oss;
		if(proto_){
			const __Items & items = proto_->Items();
			for(__Items::const_iterator i = items.begin();i != items.end();++i){
				const CValue * value = findValue(i->name_);
				std::string str;
				if(value)
					str = value->ToString(i->name_, tab);
				else
					str = i->ToString(tab);
				oss<<str;
			}
		}
		return oss.str();
	}
private:
	CValue & addValue(const std::string & name, const CValue & value){
		return (values_[name] = value);
	}
	const CValue * findValue(const std::string & name) const{
		__Values::const_iterator wh = values_.find(name);
		if(wh == values_.end())
			return 0;
		return &wh->second;
	}
};

#endif
