#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include "TurboRLE/trle.h"
#include "zstr.hpp"


using namespace std;


uint64_t number_bit(32);


uint64_t xs(uint64_t y){
	y^=(y<<13); y^=(y>>17);y=(y^=(y<<15)); return y;
}




void print_bin(uint64_t hash){
    string binrep;
    cout<<"print_bin    "<<hash<<"  ";
    for(uint64_t i(0);i<number_bit;++i){
        if(hash%2==0){
            binrep+="0";
        }else{
            binrep+="1";
        }
        hash>>=1;
    }
    reverse(binrep.begin(), binrep.end());
    cout<<binrep<<endl;
}



uint64_t number_flip(uint32_t hash){
    // cout<<"numberfliphash:  "<<hash<<endl;
    uint64_t res(0);
    uint64_t old_bit(hash%2);
    for(uint64_t i(1);i<number_bit;++i){
        hash>>=1;
        uint64_t new_bit(hash%2);
        if(old_bit!=new_bit){
            ++res;
            old_bit=new_bit;
        }
    }
    // cout<<res<<endl;
    return res;
}



uint32_t keep_best_hash(uint32_t seed, uint64_t steps){
    // cout<<"keepbesthash:    "<<seed<<" "<<steps<<endl;
    uint64_t best_score(number_flip(seed));
    uint32_t best_hash(seed);
    // cout<<"best score:  "<<best_score<<endl;
    // print_bin(best_hash);
    uint64_t hash(seed);
    for(uint64_t i(0);i<steps;++i){
        hash=(xs(hash));
        uint64_t score(number_flip(hash));
        if(score<best_score){
            best_score=score;
            best_hash=hash;
            // cout<<"best score:  "<<best_score<<" step:  "<<i<<endl;
            // print_bin(best_hash);
        }
    }
    // cout<<"best score:  "<<best_score<<endl;
    // print_bin(best_hash);
    return best_hash;
}


int main(int argc, char ** argv){
    srand (time(NULL));

	if(argc<2){
		cout<<"[steps] "<<endl;
		exit(0);
	}
	uint64_t steps(stoi(argv[1]));
    uint64_t iteration(100000);
    vector<uint32_t> sketch;
    #pragma omp parallel for
    for(uint64_t i=(0);i<iteration;++i){
        uint32_t tp(keep_best_hash(xs(rand()),steps));
        #pragma omp critical(push)
        sketch.push_back(tp);
   }
   zstr::ofstream out("sketch.gz",ofstream::binary);
   for(auto const& hash: sketch) {
       out.write(reinterpret_cast<const char *>(&hash), 4);
   }

   ofstream out2("sketch.txt",ofstream::binary);
   for(auto const& hash: sketch) {
       out2.write(reinterpret_cast<const char *>(&hash), 4);
   }

    // uint64_t size(4*iteration);
    // uint64_t compressed_size;
    // unsigned char compressed[4*iteration+1024];
    // compressed_size=trlec(  (unsigned char*)(&sketch[0]) ,size, compressed);
    // cout<<compressed_size<<endl;

}
