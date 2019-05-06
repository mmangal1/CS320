#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <math.h>
#include <list>
 
  
using namespace std;
 
struct instr {
        unsigned long long addr;
        string op;
};
 
vector<int> directMapCache(vector<instr>, int bitLen);
vector<int> setAssocCache(vector<instr> instr_tb, int bitLen, int way);
vector<int> fullyAssocCacheLRU(vector<instr> instr_tb, int bitLen, int way);
vector<int> fullyAssocCacheHC(vector<instr> instr_tb, int bitLen, int way);
vector<int> SA_noAlloc(vector<instr> instr_tb, int bitlen, int way);
vector<int> SA_prefetch(vector<instr> instr_tb, int bitlen, int way);
vector<int> SA_prefetchMiss(vector<instr> instr_tb, int bitLen, int way);

int main(int argc, char *argv[]){
    	if(argc != 3){
            cout << "Usage: ./prj2 <input file> <output file>" << endl;
            exit(1);
	}

        vector<struct instr> input;
        unsigned long long address;
        string instr_type;
        ifstream input_fp(argv[1]);
        while(input_fp >> instr_type >> hex >> address){
            struct instr ins;
            ins.op = instr_type;
            ins.addr = address;
            input.push_back(ins);
        }
 
        int directMapCacheArr[4] = {10, 12, 14, 15};
        vector<vector<int>> dm_fin;
        for(int x = 0; x < 4; x++){
            vector<int> dm_res1 = directMapCache(input, directMapCacheArr[x]);
            dm_fin.push_back(dm_res1);
        }

        int setAssocCacheArr[4] = {2, 4, 8, 16};
        vector<vector<int>> sa_fin;
        for(int x = 0; x < 4; x++){
            vector<int> sa_res1 = setAssocCache(input, 14, setAssocCacheArr[x]);
            sa_fin.push_back(sa_res1);
        }

        vector<int> fa_res1 = fullyAssocCacheLRU(input, 14, 512);
        vector<vector<int>> fa_fin;
        fa_fin.push_back(fa_res1);

        vector<int> faHC_res1 = fullyAssocCacheHC(input, 14, 512);
        vector<vector<int>> faHC_fin;
        faHC_fin.push_back(faHC_res1);

        int arr[4] = {2, 4, 8, 16};
        vector<vector<int>> saNoAlloc_fin;
        for(int x = 0; x < 4; x++){
            vector<int> saNoAlloc_res1 = SA_noAlloc(input, 14, arr[x]);
            saNoAlloc_fin.push_back(saNoAlloc_res1);
        }
        
        vector<vector<int>> saPrefetch_fin;
        for(int x = 0; x < 4; x++){
            vector<int> saPrefetch_res1 = SA_prefetch(input, 14, arr[x]);
            saPrefetch_fin.push_back(saPrefetch_res1);
        }

        vector<vector<int>> saPrefetchMiss_fin;
        for(int x = 0; x < 4; x++){
            vector<int> saPrefetchMiss_res1 = SA_prefetchMiss(input, 14, arr[x]);
            saPrefetchMiss_fin.push_back(saPrefetchMiss_res1);
        }

        vector<vector<vector<int>>> res_vec;
        res_vec.push_back(dm_fin);
        res_vec.push_back(sa_fin);
        res_vec.push_back(fa_fin);
        res_vec.push_back(faHC_fin);
        res_vec.push_back(saNoAlloc_fin);
        res_vec.push_back(saPrefetch_fin);
        res_vec.push_back(saPrefetchMiss_fin);

        string outputFile = argv[2];
		ofstream fp(outputFile);
		for(int x = 0; x < res_vec.size(); x++){
			for(int y = 0; y < res_vec.at(x).size(); y++){
				for(int z = 0; z < res_vec.at(x).at(y).size(); z++){
					if(z != res_vec.at(x).at(y).size() - 1){
						fp << res_vec.at(x).at(y).at(z) << ",";
					}else{
						fp << res_vec.at(x).at(y).at(z) << ";";
					}
				}
				if(y != res_vec.at(x).size()-1){
					fp << " ";
				}
			}
			fp << endl;
		}
		fp.close();
}

vector<int> directMapCache(vector<instr> instr_tb, int bitLen){
	    int actual_bit_len = bitLen - 5;
        int cache_size = pow(2, actual_bit_len);
        vector<unsigned long long> tag_tb(cache_size, 0);
        
        int num_hit = 0;
        int count = 0;
        while(count < instr_tb.size()){
            	unsigned long long block_idx = ((instr_tb[count].addr >> 5) % (int)pow(2, actual_bit_len));
        	unsigned long long tagVal = (instr_tb[count].addr / (int)pow(2, actual_bit_len));

        	if(tag_tb[block_idx] != tagVal){
                tag_tb[block_idx] = tagVal;
        	}else{
                num_hit++;
        	}
        	count++;
        }
        vector<int> res;
        res.push_back(num_hit);
        res.push_back(instr_tb.size());
        return res;
}

vector<int> setAssocCache(vector<instr> instr_tb, int bitLen, int way){
    int actual_bit_len = bitLen - 5;
    int cache_size = (int)(pow(2, actual_bit_len - log2(way)));
    
    vector<unsigned long long> tag_tb(cache_size, 0);
    vector<vector<unsigned long long>> associative_cache_tb(way, tag_tb);
    
    vector<int> LRU_tb(cache_size, -1);
    vector<vector<int>> LRU(way, LRU_tb);

    int num_hit = 0;
    int count = 0;
    while(count < instr_tb.size()){
        unsigned long long block_idx = ((instr_tb[count].addr >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long tagVal = (instr_tb[count].addr / (int)pow(2, actual_bit_len));

        bool flag1 = false;
        bool flag2 = false;

        for(int x = 0; x < way; x++){
            if(associative_cache_tb[x][block_idx] == tagVal){
                LRU[x][block_idx] = count;
                flag2 = true;
                flag1 = true;
                num_hit++;
                break;
            }
        }

        if(!flag1){
           for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    LRU[x][block_idx] = count;
                    associative_cache_tb[x][block_idx] = tagVal;
                    flag2 = true;
                    break;
                }
            } 
        }

        if(!flag2){
            int vic_idx = 0;
            long long vic_val = instr_tb.size() + 1;
            for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    vic_idx = x;
                    break;
                }
                if(LRU[x][block_idx] < vic_val){
                    vic_val = LRU[x][block_idx];
                    vic_idx = x;
                }
            }
            associative_cache_tb[vic_idx][block_idx] = tagVal;
            LRU[vic_idx][block_idx] = count;
        }

        count++;
    } 
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;
}


vector<int> fullyAssocCacheLRU(vector<instr> instr_tb, int bitLen, int way){
    int actual_bit_len = bitLen-5;
    int cache_size = way;
    
    vector<unsigned long long> tag_tb(cache_size, 0);
    vector<int> LRU(cache_size, -1);

    int num_hit = 0;
    int tb_idx = 0;
    int count = 0;
    while(count < instr_tb.size()){
	    unsigned long long tag_val = instr_tb[count].addr / pow(2, actual_bit_len - 4);
        bool hit = false;

        for(int x = 0; x < cache_size; x++){
            if(tag_tb[x] == tag_val){
                num_hit++;
                LRU[x] = count;
                hit = true;
                break;
            }
        }

        if(!hit){
            if(tb_idx >= cache_size){
                int vic_idx = 0;
                long long vic_val = instr_tb.size() + 1;
                for(int x = 0; x < way; x++){
                    if(LRU[x] == -1){
                        vic_idx = x;
                        break;
                    }
                    if(LRU[x] < vic_val){
                        vic_val = LRU[x];
                        vic_idx = x;
                    }
                }
                tag_tb[vic_idx] = tag_val;
                LRU[vic_idx] = count;
            }
            else{
                tag_tb[tb_idx] = tag_val;
                LRU[tb_idx] = count;
                tb_idx++;
            }
        }
        count++;
    }
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;
}

vector<int> fullyAssocCacheHC(vector<instr> instr_tb, int bitLen, int way){
    int cache_size = way;
    int actual_bit_len = bitLen-5;
    vector<unsigned long long> FA_tb(cache_size, 0);
    vector<int> HC(cache_size-1, 0);

    int num_hit = 0;
    int count = 0;
    while(count < instr_tb.size()){
        unsigned long long tag_val = instr_tb[count].addr / pow(2, actual_bit_len - 4);
        int idx;
        bool hit = false;

        for(int x = 0; x < cache_size; x++){
            if(FA_tb[x] == tag_val){
                num_hit++;
                idx = x;
                hit = true;
                break;
            }
        }

        int low = 0;
        int up = cache_size -1;
        int pos = (low+up)/2;

        if(!hit){
            while(pos % 2 == 1){
                if(!HC[pos]){
                    low = pos;
                    HC[pos] = 1;
                }else{
                    up = pos;
                    HC[pos] = 0;
                }
                pos = (low+up)/2;
            }
            if(!HC[pos]){
                HC[pos] = 1;
                FA_tb[pos+1] = tag_val;
            }else{
                HC[pos] = 0;
                FA_tb[pos] = tag_val;
            }    
        }else{       
            while((idx/2)*2 != pos){
                if((idx/2)*2 < pos){
                    HC[pos] = 0;
                    up = pos;
                }else{
                    HC[pos] = 1;
                    low = pos;
                }
                pos = (low+up)/2;
            }
            HC[pos] = idx - pos;
            FA_tb[idx] = tag_val;      
        }
        count++;
    }
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;
}

vector<int> SA_noAlloc(vector<instr> instr_tb, int bitLen, int way){
    int actual_bit_len = bitLen - 5;
    int cache_size = (int)(pow(2, actual_bit_len - log2(way)));
    
    vector<unsigned long long> tag_tb(cache_size, 0);
    vector<vector<unsigned long long>> associative_cache_tb(way, tag_tb);
    
    vector<int> LRU_tb(cache_size, -1);
    vector<vector<int>> LRU(way, LRU_tb);

    int num_hit = 0;
    int count = 0;
    while(count < instr_tb.size()){

        bool flag1 = false;
        bool flag2 = false;
        unsigned long long block_idx = ((instr_tb[count].addr >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long tagVal = (instr_tb[count].addr / (int)pow(2, actual_bit_len));

        for(int x = 0; x < way; x++){
            if(associative_cache_tb[x][block_idx] == tagVal){
                LRU[x][block_idx] = count;
                flag2 = true;
                flag1 = true;
                num_hit++;
                break;
            }
        }

        if(instr_tb[count].op == "L" && !flag1){
           for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    flag2 = true;
                    associative_cache_tb[x][block_idx] = tagVal;
                    LRU[x][block_idx] = count;
                    break;
                }
            } 
        }

        if(instr_tb[count].op == "L" && !flag2){
            int vic_idx = 0;
            long long vic_val = instr_tb.size() + 1;
            for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    vic_idx = x;
                    break;
                }
                if(LRU[x][block_idx] < vic_val){
                    vic_val = LRU[x][block_idx];
                    vic_idx = x;
                }
            }
            LRU[vic_idx][block_idx] = count;
            associative_cache_tb[vic_idx][block_idx] = tagVal;
        }

        count++;
    } 
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;

}

vector<int> SA_prefetch(vector<instr> instr_tb, int bitLen, int way){
    int actual_bit_len = bitLen - 5;
    int cache_size = (int)(pow(2, actual_bit_len - log2(way)));

    vector<unsigned long long> tag_tb(cache_size, 0);
    vector<vector<unsigned long long>> associative_cache_tb(way, tag_tb);

    vector<int> LRU_tb(cache_size, -1);
    vector<vector<int>> LRU(way, LRU_tb);

    int num_hit = 0;
    int num_fetch = 0;
    int count = 0;
    while(count < instr_tb.size()){
        bool hit = false;
        bool hit_prev = false;
        unsigned long long block_idx = ((instr_tb[count].addr >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long tagVal = (instr_tb[count].addr / (int)pow(2, actual_bit_len));
        unsigned long long next_idx =  (((instr_tb[count].addr+32) >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long next_tag = ((instr_tb[count].addr+32) / (int)pow(2, actual_bit_len));

        for(int x = 0; x < way; x++){
            if(associative_cache_tb[x][block_idx] == tagVal){
                LRU[x][block_idx] = num_fetch;
                hit = true;
                num_hit++;
                break;
            }
        }

        if(!hit){
            int vic_idx = 0;
            long long vic_val = instr_tb.size() + 1;
            for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    vic_idx = x;
                    break;
                }
                if(LRU[x][block_idx] < vic_val){
                    vic_val = LRU[x][block_idx];
                    vic_idx = x;
                }
            }
            associative_cache_tb[vic_idx][block_idx] = tagVal;
            LRU[vic_idx][block_idx] = num_fetch;
        }

        if(block_idx != (cache_size - 1)){
            for(int x = 0; x < way; x++){
                if(associative_cache_tb[x][next_idx] == next_tag){
                    LRU[x][next_idx] = num_fetch;
                    hit_prev = true;
                    break;
                }
            }
            if(!hit_prev){
                int vic_idx = 0;
                long long vic_val = instr_tb.size() + 1;
                for(int x = 0; x < way; x++){
                    if(LRU[x][next_idx] == -1){
                        vic_idx = x;
                        break;
                    }
                    if(LRU[x][next_idx] < vic_val){
                        vic_val = LRU[x][next_idx];
                        vic_idx = x;
                    }
                }
                associative_cache_tb[vic_idx][next_idx] = next_tag;
                LRU[vic_idx][next_idx] = num_fetch;
            }
        }else{
            for(int x = 0; x < way; x++){
                if(associative_cache_tb[x][0] == next_tag){
                    LRU[x][0] = num_fetch;
                    hit_prev = true;
                    break;
                }
            }
            if(!hit_prev){
                int vic_idx = 0;
                long long vic_val = instr_tb.size() + 1;
                for(int x = 0; x < way; x++){
                    if(LRU[x][next_idx] == -1){
                        vic_idx = x;
                        break;
                    }
                    if(LRU[x][next_idx] < vic_val){
                        vic_val = LRU[x][next_idx];
                        vic_idx = x;
                    }
                }
                associative_cache_tb[vic_idx][0] = next_tag;
                LRU[vic_idx][0] = num_fetch;
            }
        }
	    num_fetch++;
        count++;
    } 
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;

}

vector<int> SA_prefetchMiss(vector<instr> instr_tb, int bitLen, int way){
    int actual_bit_len = bitLen - 5;
    int cache_size = (int)(pow(2, actual_bit_len - log2(way)));
    
    vector<unsigned long long> tag_tb(cache_size, 0);
    vector<vector<unsigned long long>> associative_cache_tb(way, tag_tb);
    
    vector<int> LRU_tb(cache_size, -1);
    vector<vector<int>> LRU(way, LRU_tb);

    int num_hit = 0;
    int num_fetch = 0;
    int count = 0;
    while(count < instr_tb.size()){
        bool hit = false;
        bool fetch = false;
        unsigned long long block_idx = ((instr_tb[count].addr >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long tagVal = (instr_tb[count].addr / (int)pow(2, actual_bit_len));
        unsigned long long next_idx =  (((instr_tb[count].addr+32) >> 5) % (int)pow(2, actual_bit_len-log2(way)));
        unsigned long long next_tag = ((instr_tb[count].addr+32) / (int)pow(2, actual_bit_len));

        for(int x = 0; x < way; x++){
            if(associative_cache_tb[x][block_idx] == tagVal){
                LRU[x][block_idx] = num_fetch;
                hit = true;
                num_hit++;
                break;
            }
        }

        if(!hit){
            int vic_idx = 0;
            long long vic_val = instr_tb.size() + 1;
            for(int x = 0; x < way; x++){
                if(LRU[x][block_idx] == -1){
                    vic_idx = x;
                    break;
                }
                if(LRU[x][block_idx] < vic_val){
                    vic_val = LRU[x][block_idx];
                    vic_idx = x;
                }
            }
            associative_cache_tb[vic_idx][block_idx] = tagVal;
            LRU[vic_idx][block_idx] = num_fetch;
            fetch = true;
        }

        if(fetch){
            bool hit_prev = false;
            if(block_idx != (cache_size - 1)){
                for(int x = 0; x < way; x++){
                    if(associative_cache_tb[x][next_idx] == next_tag){
                        LRU[x][next_idx] = num_fetch;
                        hit_prev = true;
                        break;
                    }
                }
                if(!hit_prev){
                    int vic_idx = 0;
                    long long vic_val = instr_tb.size() + 1;
                    for(int x = 0; x < way; x++){
                        if(LRU[x][next_idx] == -1){
                            vic_idx = x;
                            break;
                        }
                        if(LRU[x][next_idx] < vic_val){
                            vic_val = LRU[x][next_idx];
                            vic_idx = x;
                        }
                    }
                    associative_cache_tb[vic_idx][next_idx] = next_tag;
                    LRU[vic_idx][next_idx] = num_fetch;
                }
            }else{
                for(int x = 0; x < way; x++){
                    if(associative_cache_tb[x][0] == next_tag){
                        LRU[x][0] = num_fetch;
                        hit_prev = true;
                        break;
                    }
                }
                if(!hit_prev){
                    int vic_idx = 0;
                    long long vic_val = instr_tb.size() + 1;
                    for(int x = 0; x < way; x++){
                        if(LRU[x][next_idx] == -1){
                            vic_idx = x;
                            break;
                        }
                        if(LRU[x][next_idx] < vic_val){
                            vic_val = LRU[x][next_idx];
                            vic_idx = x;
                        }
                    }
                    associative_cache_tb[vic_idx][0] = next_tag;
                    LRU[vic_idx][0] = num_fetch;
                }
            }
        }
        num_fetch++;
        count++;
    } 
    vector<int> res;
    res.push_back(num_hit);
    res.push_back(instr_tb.size());
    return res;

}




