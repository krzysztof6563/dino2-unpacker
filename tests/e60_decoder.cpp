#include "../unpacker/DC2Animations.h"
#include <fstream>
#include <iterator>
#include <iostream>
int main(int argc, char** argv){if(argc!=2)return 2;std::ifstream f(argv[1],std::ios::binary);std::vector<unsigned char>b((std::istreambuf_iterator<char>(f)),{});std::string r; auto c=dc2::decodeAnimations(b,0x638000,20,{},r);std::cout<<c.size()<<" "<<r<<"\n";size_t n=0;for(auto&x:c)n+=x.times.size();std::cout<<n<<" keys\n";return c.size()!=32;}
