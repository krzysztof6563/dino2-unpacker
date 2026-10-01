#include "../unpacker/DC2Animations.h"
#include <fstream>
#include <iterator>
#include <iostream>
int main(int argc,char** argv){if(argc!=2)return 2;int index=0;const size_t expected[]={70,32,27,11,29,23,28,32};for(auto n:{"E00","E10","E20","E30","E31","E32","E40","E60"}){std::ifstream f(std::string(argv[1])+"/"+n+".block",std::ios::binary);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});std::string r;unsigned base=std::string(n)=="E00"?0x633000:std::string(n)=="E40"?0x63f500:std::string(n)=="E60"?0x638000:0x640000;auto c=dc2::decodeAnimations(b,base,b[20]+256*b[21],{},r);if(c.size()!=expected[index++])return 1;std::cout<<n<<" "<<c.size()<<" "<<r<<"\n";}}
