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

typedef  uint32_t Hash;
uint64_t number_bit(sizeof(Hash)*8);
uint score_function_used(1);




uint64_t xs(uint64_t y){
	y^=(y<<13); y^=(y>>17);y=(y^=(y<<15)); return y;
}




void print_bin(Hash hash){
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



uint64_t number_flip(Hash hash){
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



uint64_t number_1(Hash hash){
    uint64_t res(0);
    for(uint64_t i(0);i<number_bit;++i){
        if(hash%2==1){
            res++;
        }
        hash>>=1;
    }
    return res;
}



vector<uint64_t> generate_hashes(uint64_t n){
    vector<uint64_t> res;
    uint64_t hash=xs(rand());
    for (uint64_t i(0);i<n;i++){
        if(i%1000){
            hash=xs(rand());
        }else{
            hash=xs(hash);
        }
        res.push_back(hash);
    }
    return res;
}


string bitwiseRLEint(Hash x){
    string res;
    uint old_bit(x%2);
    unsigned char run_length(0);
    for(uint64_t i(1);i<number_bit;++i){
        x>>=1;
        uint64_t new_bit(x%2);
        if(old_bit!=new_bit){
            res+=run_length;
            run_length=0;
            old_bit=new_bit;
        }else{
            run_length++;
        }
    }
    return res;
}


string bitwiseRLE(const vector<Hash>& V){
    string res;
    uint old_bit(V[0]%2);
    unsigned char run_length(0);
    for(uint32_t e(0);e<V.size();++e){
        Hash x=V[e];
        for(uint64_t i(0);i<number_bit;++i){
            uint64_t new_bit(x%2);
            if(old_bit!=new_bit){
                res+=run_length;
                run_length=0;
                old_bit=new_bit;
            }else{
                run_length++;
            }
            x>>=1;
        }
    }
    return res;
}



vector<Hash> partitionskecth(const vector<uint64_t>& hashes, uint64_t H){
    vector<Hash> sketch(H,-1);
    vector<uint64_t> sketchscore(H,-1);
    uint64_t score;
    for (uint64_t i(0);i<hashes.size();i++){
        Hash hash(hashes[i]/H);
        uint32_t indice(hashes[i]%H);
        if(score_function_used==0){
            score=hash;
        }else if (score_function_used==1) {
            score=number_1(hash);
        }else if (score_function_used==2) {
            score=number_flip(hash);
        }

        if(score<sketchscore[indice] or (score==sketchscore[indice] and hash<sketch[indice])){
            sketchscore[indice]=score;
            sketch[indice]=hash;
        }
    }
    // for(uint i(0);i<H;++i){
    //     print_bin(sketch[i]);cin.get();
    // }
    return sketch;
}


bool comparehash(Hash a, Hash b){
    if (score_function_used==1) {
        if(number_1(a)<number_1(b) or (number_1(a)==number_1(b) and a<b) ){
            return true;
        }
        return false;
    }else if (score_function_used==2) {
        if(number_flip(a)<number_flip(b) or (number_flip(a)==number_flip(b) and a<b) ){
            return true;
        }
        return false;
    }
    return (a < b);
}


vector<Hash> kminsketch(vector<uint64_t>& hashes, uint64_t H){
    vector<Hash> res;
    sort(hashes.begin(),hashes.end(),comparehash);
    hashes.erase( unique( hashes.begin(), hashes.end() ), hashes.end() );
    for(uint i(0);i<H;++i){
        res.push_back(hashes[i]);
    }
    // for(uint i(0);i<H;++i){
    //     print_bin(res[i]);cin.get();
    // }
    return res;
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



void print_vector(zstr::ofstream* outz,const vector<Hash>& sketch){
        outz->write(reinterpret_cast<const char *>(&sketch[0]), sizeof(Hash)*sketch.size());
}


void print_vector_rle(zstr::ofstream* outz,const vector<Hash>& sketch){
    string rle(bitwiseRLE(sketch));
    *outz<<rle;
}



uint64_t filesize(const string& filename){
    ifstream in(filename, ifstream::ate | ifstream::binary);
    return in.tellg();
}


void run_simulation(uint64_t hash_number,uint64_t sketch_size,uint64_t doc_number){
    uint64_t size_vanilla(sketch_size*sizeof(Hash)*doc_number);
    {
        zstr::ofstream outh("hashes.gz");
        zstr::ofstream outz("sketch.gz");
        zstr::ofstream outrlez("sketchrle.gz");


        // #pragma omp parallel for
        for(uint64_t i=(0);i<doc_number;++i){
            auto hashes(generate_hashes(hash_number));
            auto sketch(partitionskecth(hashes,sketch_size));
            // auto sketch(kminsketch(hashes,sketch_size));
            #pragma omp critical
            {
                // print_vector(&outh,hashes);
                print_vector(&outz,sketch);
                print_vector_rle(&outrlez,sketch);
            }
        }
    }
    // uint64_t size_compressedhashes(filesize("hashes.gz"));
    uint64_t size_compressed(filesize("sketch.gz"));
    uint64_t size_compressedrle(filesize("sketchrle.gz"));

    // cout<<"Gz compression hashes:      "<<intToString(size_compressedhashes)<<"    bytes"<<endl;
    cout<<"Gz compression:      "<<intToString(size_compressed)<<"    bytes"<<endl;
    cout<<"Compression ratio:       "<<(double)size_vanilla/(double)size_compressed<<endl;
    cout<<"Gz compression RLE:      "<<intToString(size_compressedrle)<<"    bytes"<<endl;
    cout<<"Compression ratio RLE:       "<<(double)size_vanilla/(double)size_compressedrle<<endl;
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
    uint64_t size_vanilla(sketch_size*sizeof(Hash)*doc_number);
    cout<<sizeof(Hash)*8<<" bits hashes"<<endl;
    cout<<intToString(hash_number)<<" Hashes per document"<<endl;
    cout<<intToString(sketch_size)<<" Minimizers per sketch"<<endl;
    cout<<intToString(doc_number)<<" Documents (input) / Sketches (output)"<<endl;
    cout<<"A minimizer is chosen among "<<intToString(hash_number/sketch_size)<<" hashes (on average)"<<endl;
    cout<<"The uncompressed sketch file is "<<intToString(size_vanilla)<<" bytes"<<endl;


    score_function_used=0;
    cout<<"\nIDENTITY:"<<endl;
    run_simulation(hash_number,sketch_size,doc_number);
    score_function_used=1;
    cout<<"\nNUMBER 1:"<<endl;
    run_simulation(hash_number,sketch_size,doc_number);
    score_function_used=2;
    cout<<"\nNUMBER FLIP:"<<endl;
    run_simulation(hash_number,sketch_size,doc_number);


    auto end = std::chrono::system_clock::now();
    chrono::duration<double> elapsed_seconds = end - start;
    cout<<"Elapsed time: " << elapsed_seconds.count() << "s\n";

}
