# NewProtocol
This is a very tiny library focused on easy-understanding code of protocol handling.
Say you have a protocol like this:
```
//protocol 0x22 
uint8_t type; 
uint32_t ptl; 
uint16_t version;
```
you can translate it into **CProtocol** object using code like this:
```
#include "proto.h"

CProtocol protocol_0x22;

void Init0x22() {
    protocol_0x22.AddV("type", U8);
    protocol_0x22.AddV("ptl", U32);
    protocol_0x22.AddV("version", U16);
}
```
then you can create a **CCmd** object of protocol 0x22 using *protocol_0x22*, and set the values of fields, like this:
```
CCmd cmd(protocol_0x22);
cmd.Set("type", 0xA);
cmd.Set("ptl", 21331);
cmd.Set("version", 1001);
```
or get the values of fields, like this:
```
cout << cmd.GetInt("ptl");
```
But the most interesting part is, you can now encode the *cmd* object into bytes stream of format protocol 0x22, with one line of code:
```
COutStream out;
cmd.Encode(out);
```
or vice versa:
```
string buf; // bytes stream of format protocol 0x22
cmd.Decode(CInStream(buf));
cout << cmd.ToString(); //show the content of cmd
```
There are more functionality in **NewProtocol**, here is a summary of supported formats:
```
Type Suffix    Description 
--------------------------------
V              simple value, e.g. U32
LV             length and value, e.g. length + bytes
TLV            tag, length and value, e.g. 1 for length + structOne, 2 for length + structTwo
TV             tag and value, with fixed length, e.g. 1 for U32, 2 for U16
A              array of fixed size, e.g. U32[10]
LA             size and array
TLA            tag, size and array, e.g. 1 for U32.md, 2 for U16.md
SA             byte size and array, e.g. 32 bytes of U32.md, which has 8 elements
TSA            tag, byte size and array, e.g. 1 for U32.md, 2 for U16.md
```
Hope you enjoy it!
