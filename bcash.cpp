#include <fstream>
#include <cstring>
#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
#include <chrono>
#include <unordered_set>
#include "zstr.hpp"


using namespace std;


uint64_t number_bit(32);


uint64_t xs(uint64_t y){
	y^=(y<<13); y^=(y>>17);y=(y^=(y<<15)); return y;
}




void print_bin(uint64_t hash){
    string binrep;
    // cout<<"print_bin    "<<hash<<"  ";
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
    return res;
}



uint32_t number_1(uint32_t hash){
    uint32_t res(0);
    for(uint64_t i(0);i<number_bit;++i){
        hash>>=1;
        if(hash%2==1){
            res++;
        }
    }
    return res;
}



uint32_t keep_best_hash(uint32_t seed, uint64_t steps){
    // cout<<"keepbesthash:    "<<seed<<" "<<steps<<endl;
    uint64_t best_score(number_flip(seed));
    uint32_t best_hash(seed);
    // cout<<"init score:  "<<best_score<<endl;
    // print_bin(best_hash);
    uint64_t hash(seed);
    for(uint64_t i(0);i<steps;++i){
        hash=(xs(hash));
        uint64_t score(number_1(hash));
        if(score<best_score){
            best_score=score;
            best_hash=hash;
            // cout<<"best score:  "<<best_score<<" step:  "<<i<<endl;
            // print_bin(best_hash);
        }
    }
    return best_hash;
}


vector<uint64_t> generate_hashes(uint64_t n){
    vector<uint64_t> res;
    uint64_t hash=xs(rand());
    for (uint64_t i(0);i<n;i++){
        if(i%100){
            hash=xs(rand());
        }else{
            hash=xs(hash);
        }
        res.push_back(hash);
    }
    return res;
}


string bitwiseRLEint(uint32_t x){
    string res;
    // print_bin(x);
    uint old_bit(x%2);
    unsigned char run_length(0);
    for(uint64_t i(1);i<number_bit;++i){
        x>>=1;
        uint64_t new_bit(x%2);
        if(old_bit!=new_bit){
            res+=run_length;
            // cout<<(int)run_length<<endl;
            run_length=0;
            old_bit=new_bit;
        }else{
            run_length++;
        }
    }

    // cout<<res<<endl;
    // cin.get();
    return res;
}


string bitwiseRLE(const vector<uint32_t>& V){
    string res;
    uint old_bit(V[0]%2);
    unsigned char run_length(0);
    for(uint32_t e(0);e<V.size();++e){
        uint32_t x=V[e];
        // print_bin(x);
        for(uint64_t i(0);i<number_bit;++i){
            uint64_t new_bit(x%2);
            // cout<<new_bit<<endl;
            if(old_bit!=new_bit){
                res+=run_length;
                // cout<<"rl"<<(int)run_length<<endl;
                run_length=0;
                old_bit=new_bit;
            }else{
                run_length++;
            }
            x>>=1;
        }
        // cin.get();
    }
    // cout<<res.size()<<" "<<V.size()*4<<endl;
    return res;
}



vector<uint32_t> partitionskecth(const vector<uint64_t>& hashes, uint64_t H){
    vector<uint32_t> sketch(H,0);
    vector<uint32_t> sketchscore(H,4000000000);
    for (uint64_t i(0);i<hashes.size();i++){
        uint32_t hash(hashes[i]/H);
        uint32_t indice(hashes[i]%H);
        // uint32_t score(number_flip(hash));
        // uint32_t score((hash));
        uint32_t score(number_1(hash));
        if(score<sketchscore[indice]){
            sketchscore[indice]=score;
            sketch[indice]=hash;
        }
    }
    return sketch;
}


string intToString(uint64_t n){
	if(n<1000){
		return to_string(n);
	}
	string end(to_string(n%1000));
	if(end.size()==3){
		return intToString(n/1000)+","+end;
	}
	if(end.size()==2){
		return intToString(n/1000)+",0"+end;
	}
	return intToString(n/1000)+",00"+end;
}



void print_vector(zstr::ofstream* outz,const vector<uint32_t>& sketch){
    // for (uint64_t i(0);i<sketch.size();i++){
        outz->write(reinterpret_cast<const char *>(&sketch[0]), 4*sketch.size());
    // }
}

void print_vector_rle(zstr::ofstream* outz,const vector<uint32_t>& sketch){
    string rle(bitwiseRLE(sketch));
    *outz<<rle;
}



uint64_t filesize(const string& filename)
{
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}



int main(int argc, char ** argv){
    srand (time(NULL));
    auto start = std::chrono::system_clock::now();

	if(argc<4){
		cout<<"[Hash number per document] [Sketch size] [Document number] "<<endl;
		exit(0);
	}
    uint64_t hash_number(stoi(argv[1]));
    uint64_t sketch_size(stoi(argv[2]));
    uint64_t doc_number(stoi(argv[3]));
    uint64_t size_vanilla(sketch_size*4*doc_number);
    {
        zstr::ofstream outz("sketch.gz");
        cout<<intToString(hash_number)<<" Hashes per document"<<endl;
        cout<<intToString(sketch_size)<<" Minimizers per sketch"<<endl;
        cout<<intToString(doc_number)<<" Documents (input) / Sketches (output)"<<endl;
        cout<<"A minimizer is chosen among "<<intToString(hash_number/sketch_size)<<" hashes (on average)"<<endl;
        cout<<"The uncompressed sketch file is "<<intToString(size_vanilla)<<" bytes"<<endl;

        #pragma omp parallel for
        for(uint64_t i=(0);i<doc_number;++i){

            auto hashes(generate_hashes(hash_number));
            auto sketch(partitionskecth(hashes,sketch_size));
            #pragma omp critical
            {
                print_vector(&outz,sketch);
            }
        }
        cout<<endl;
    }
    uint64_t size_compressed(filesize("sketch.gz"));


    cout<<"Ended, let's take a look at sketch.gz"<<endl;
    cout<<"No compression:      "<<intToString(size_vanilla)<<"   bytes"<<endl;
    cout<<"Gz compression:      "<<intToString(size_compressed)<<"    bytes"<<endl;
    cout<<"Compression ratio:       "<<(double)size_vanilla/(double)size_compressed<<endl;
    auto end = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout<<"Elapsed time: " << elapsed_seconds.count() << "s\n";

}
