#include <iostream>

#include "proto.h"

using namespace std;

/*	Ã¬¶Ü:
	struct A{
		T L V	//optional, so A is optional
	}
	struct A{
		struct C
		T L V	//optional, is A optional?
	}

	cmd{
		A
	}

	array{
		A
	}
*/

CProtocol protocol_0x22;

void InitProtocol()
{
	//user info
	CStruct userInfo;
	userInfo.AddLv("type", U8, U64);
	userInfo.AddTlv("ptl", U8, 222, U16, STR);
	//elem
	CStruct elem;
	elem.AddTlv("elem", U8, 7, U16, STR);
	//elem1
	CStruct elem1;
	elem1.AddV("elem1", userInfo);
	//0x22
	protocol_0x22.AddV("stx", U8);
	protocol_0x22.AddTv("ver1", U8, 1, userInfo);	//branches
	protocol_0x22.AddTv("ver2", U8, 2, elem);
	protocol_0x22.AddTlv("user", U16, 111, U32, userInfo);	//struct user
	protocol_0x22.AddTla("array", U8, 99, U32, elem);
	protocol_0x22.AddTsa("array1", U16, 1234, U32, elem1, 5);
	protocol_0x22.AddTlv("user_bak", U8, 2345, U16, userInfo);
	protocol_0x22.AddArray("array2", U32, 2);

	cout<<"protocol_0x22 = "<<protocol_0x22.ToString();
}

void InitCmd(CCmd & cmd)
{
	cmd.Set("stx", 0x2);
	cmd.Set("user.type", 10);
	cmd.Set("user.ptl", "abc");
	//branch 1
	//cmd.Set("ver1.type", 123);
	//cmd.Set("ver1.ptl", "hahaha");
	//branch 2
	cmd.Set("ver2.elem", "hehehe");
	string s;
	for(int i = 0;i < 2;++i)
		cmd.AddElemCmd("array").Set("elem", s += 'a' + i);
	s.clear();
	for(int i = 0;i < 5;++i){
		CCmd elem = cmd.AddElemCmd("array1");
		elem.Set("elem1.type", 200 + i);
		if(i % 2)
			elem.Set("elem1.ptl", s += '0' + i);
	}
	//cmd.Set("user_back.type", 99);
	//cmd.Set("user_.back.ptl", "ggf");
	cmd.AddElem("array2", 111);
	cmd.AddElem("array2", 222);
	cout<<"cmd = "<<cmd.ToString();
}

void Show(const CCmd & cmd)
{
	cout<<"stx = "<<cmd.GetInt("stx")
		<<"\nuser = {"
		<<cmd.GetTag("user")
		<<", type = "<<cmd.GetInt64("user.type")
		<<", ptl = {"<<cmd.GetTag("user.ptl")<<", "<<cmd.GetStr("user.ptl")<<"}"
		<<"}\n";
	cout<<"array = {"
		<<cmd.GetTag("array")<<",\n";
	for(size_t i = 0;i < cmd.GetArraySize("array");++i)
		cout<<"["<<i<<"] = "<<cmd.GetElemCmd("array", i).GetStr("elem")
			<<endl;
	cout<<"}\n";
	cout<<"array1 = {"
		<<cmd.GetTag("array1")<<",\n";
	for(size_t i = 0;i < cmd.GetArraySize("array1");++i)
		cout<<"["<<i<<"] = {"
			<<"type = "<<cmd.GetElemCmd("array1", i).GetInt("elem1.type")
			<<", ptl = "<<cmd.GetElemCmd("array1", i).GetStr("elem1.ptl")
			<<"}\n";
	cout<<"}";
	cout<<"\nuser_bak = {"<<cmd.GetTag("user_bak")
		<<", "<<cmd.GetInt("user_bak.type")
		<<", ptl = {"<<cmd.GetTag("user_bak.ptl")<<", "<<cmd.GetStr("user_bak.ptl")
		<<"}}";
	cout<<"\nempty="<<cmd.GetInt("empty")<<endl;
}

void Dump(const std::string & buf)
{
	const char * HEX = "0123456789ABCDEF";
	cout<<"("<<buf.size()<<")";
	for(size_t i = 0;i < buf.size();++i)
		cout<<HEX[(buf[i] >> 4) & 0xF]
			<<HEX[buf[i] & 0xF]
			<<" ";
	cout<<endl;
}

bool Encode(const CCmd & cmd, string & buf)
{
	COutStream out;
	if(!cmd.Encode(out))
		return false;
	out.Export(buf);
	return true;
}

bool Decode(const std::string & buf, CCmd & cmd)
{
	CInStream in(buf);
	return cmd.Decode(in);
}

int main()
{
	//set protocol
	InitProtocol();

	//set req
	CCmd reqCmd(protocol_0x22);
	InitCmd(reqCmd);

	//encode
	string buf;
	if(!Encode(reqCmd, buf)){
		cerr<<"encode error\n";
		return 1;
	}
	Dump(buf);

	//decode
	CCmd respCmd(protocol_0x22);
	if(!Decode(buf, respCmd)){
		cerr<<"decode error\n";
		return 1;
	}

	//fetch fields
	//Show(respCmd);
	cout<<respCmd.ToString();

	return 0;
}
//
