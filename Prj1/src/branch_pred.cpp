#include <iostream>
#include <string>
#include <fstream>
#include <vector>

#include "branch_pred.h"

using namespace std;


int main(int argc, char *argv[]){

	if(argc != 3){
		cout << "Usage: ./branch_pred [input] [output]\n";
		exit(1);
	}

	string filename = argv[1];
	ifstream fp1(filename);
	ifstream fp2(filename);
	
	vector<table> tableTraceOneBit;
	vector<table> tableTraceTwoBit;
	
	unsigned long long addr;
	string behavior;

	//table for one bit
	while(fp1 >> std::hex >> addr >> behavior){
		struct table entry;
		entry.addr = addr;
		if(behavior == "T"){
			entry.taken = 1;
		}else{
			entry.taken = 0;
		}
		tableTraceOneBit.push_back(entry);
	}
	
	//table for two bit
	while(fp2 >> std::hex >> addr >> behavior){
		struct table entry;
		entry.addr = addr;
		if(behavior == "T"){
			entry.taken = 1;
		}else{
			entry.taken = -1;
		}
		tableTraceTwoBit.push_back(entry);
	}

	cout << tableTraceOneBit.size() << endl;	
	vector<vector<int>> resAlwaysTaken;
	resAlwaysTaken.push_back(alwaysTaken(tableTraceOneBit));
	
	vector<vector<int>> resAlwaysNotTaken;
	resAlwaysNotTaken.push_back(alwaysNotTaken(tableTraceOneBit));
	
	vector<int> size_table = {16, 32, 128, 256, 512, 1024, 2048};
	vector<vector<int>> resBimodalSingle;
	vector<vector<int>> resBimodalDouble;

	for(int x = 0; x < size_table.size(); x++){
		resBimodalSingle.push_back(bimodalSingle(tableTraceOneBit, size_table[x]));
		resBimodalDouble.push_back(bimodalDouble(tableTraceTwoBit, size_table[x]));
	}

	vector<vector<int>> resGshare;
	for(int x = 3; x < 12; x++){
		resGshare.push_back(gshare(tableTraceTwoBit, x));
	}

	vector<vector<int>> resTournament;

	resTournament.push_back(tournament(tableTraceTwoBit));

	vector<vector<vector<int>>> res_vec;
	res_vec.push_back(resAlwaysTaken);
	res_vec.push_back(resAlwaysNotTaken);
	res_vec.push_back(resBimodalSingle);
	res_vec.push_back(resBimodalDouble);
	res_vec.push_back(resGshare);
	res_vec.push_back(resTournament);

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

	return 0;
}

vector<int> alwaysTaken(vector<table> tb){
	int accuracy_num = 0;
	for(auto iter = tb.begin(); iter < tb.end(); iter += 1){
		if(iter->taken){
			accuracy_num++;
		}
	}
	
	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
}

vector<int> alwaysNotTaken(vector<table> tb){
	int accuracy_num = 0;
	for(auto iter = tb.begin(); iter < tb.end(); iter += 1){
		if(iter->taken == false){
			accuracy_num++;
		}
	}

	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
}

vector<int> bimodalSingle(vector<table> tb, int size){
	int accuracy_num = 0;
	vector<int> track_table(size, -1);
	
	for(auto iter = tb.begin(); iter < tb.end(); iter += 1){
		int table_index = iter->addr % size;
		bool prediction;
		bool actual;
		if(iter->taken == 1){
			actual = true;
		}else{
			actual = false;
		}
		if(track_table[table_index] > 0){
			if(actual)
				prediction = true;
			else
				prediction = false;
			
		}else{
			if(!actual)
				prediction = true;
			else
				prediction = false;
		}
				
		if(prediction){
			accuracy_num++;
		}else{
			if(track_table[table_index] == 1){
				track_table[table_index] = -1;
			}else{
				track_table[table_index] = 1;
			}
		}
	}
	
	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
	
}

vector<int> bimodalDouble(vector<table> tb, int size){
	int accuracy_num = 0;
	vector<int> track_table(size, -1);
	
	for(auto iter = tb.begin(); iter < tb.end(); iter += 1){
		int table_index = iter->addr % size;
		bool prediction;
		bool actual;
		if(iter->taken == 1){
			actual = true;
		}else{
			actual = false;
		}
		if(track_table[table_index] > 0){
			if(actual)
				prediction = true;
			else
				prediction = false;
			
		}else{
			if(!actual)
				prediction = true;
			else
				prediction = false;
		}
				
		if(prediction){
			accuracy_num++;
			if(track_table[table_index] == 1){
				track_table[table_index] = 2;
			}else if(track_table[table_index] == -1){
				track_table[table_index] = -2;
			}
		}else{
			if(track_table[table_index] == 1 || track_table[table_index] == -2){
				track_table[table_index] = -1;
			}else{
				track_table[table_index] = 1;
			}
		}
	}
	
	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
	
}

vector<int> gshare(vector<table> tb, int hist_size){
	string hist;
	for(int x = 0; x < hist_size; x++){
		hist.append("0");
	}

	int accuracy_num = 0;
	vector<int> track_table(2048, -1);
	
	for(auto iter = tb.begin(); iter < tb.end(); iter += 1){
		int table_index = (iter->addr ^ stoi(hist, nullptr, 2)) % 2048;
		bool prediction;
		bool actual;
		if(iter->taken == 1){
			actual = true;
		}else{
			actual = false;
		}
		if(track_table[table_index] > 0){
			if(actual)
				prediction = true;
			else
				prediction = false;
			
		}else{
			if(!actual)
				prediction = true;
			else
				prediction = false;
		}
				
		if(prediction){
			accuracy_num++;
			if(track_table[table_index] == 1){
				track_table[table_index] = 2;
			}else if(track_table[table_index] == -1){
				track_table[table_index] = -2;
			}
		}else{
			if(track_table[table_index] == 1 || track_table[table_index] == -2){
				track_table[table_index] = -1;
			}else{
				track_table[table_index] = 1;
			}
		}

		if(actual){
			hist = hist.substr(1, hist.size()).append("1");
		}else{
			hist = hist.substr(1, hist.size()).append("0");
		}
	}
	
	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
	
}

vector<int> tournament(vector<table> tb){
	string hist = "00000000000";

	int accuracy_num = 0;
	vector<int> gshare_table(2048, -1);
	vector<int> bimodal_table(2048, -1);
	vector<int> track_table(2048, 2);
	
	for(int x = 0; x < tb.size(); x++){
		int gshare_index = (tb.at(x).addr ^ stoi(hist, nullptr, 2)) % 2048;
		int index = tb.at(x).addr % 2048;
		bool g_prediction;
		bool b_prediction;
		bool actual;
		if(tb.at(x).taken == 1){
			actual = true;
		}else{
			actual = false;
		}
		if(gshare_table[gshare_index] > 0){
			if(actual)
				g_prediction = true;
			else
				g_prediction = false;
			
		}else{
			if(!actual)
				g_prediction = true;
			else
				g_prediction = false;
		}
		
		if(bimodal_table[index] > 0){
			if(actual)
				b_prediction = true;
			else
				b_prediction = false;
			
		}else{
			if(!actual)
				b_prediction = true;
			else
				b_prediction = false;
		}
				
		if(track_table[index] < 2){
			if(g_prediction){
				accuracy_num++;
				if(!b_prediction){
					if(track_table[index] > 0){
						track_table[index]--;
					}
				}
			}else{
				if(b_prediction){
					if(track_table[index] < 3){
						track_table[index]++;
					}
				}
			}
		}else{
			if(b_prediction){
				accuracy_num++;
				if(!g_prediction){
					if(track_table[index] < 3){
						track_table[index]++;
					}
				}
			}else{
				if(g_prediction){
					if(track_table[index] > 0){
						track_table[index]--;
					}
				}
			}
			
		}

		if(g_prediction){
			if(gshare_table[gshare_index] == 1){
				gshare_table[gshare_index] = 2;
			}else if(gshare_table[gshare_index] == -1){
				gshare_table[gshare_index] = -2;
			}
		}else{
			if(gshare_table[gshare_index] == 1 || gshare_table[gshare_index] == -2){
				gshare_table[gshare_index] = -1;
			}else{
				gshare_table[gshare_index] = 1;
			}
		}
		if(b_prediction){
			if(bimodal_table[index] == 1){
				bimodal_table[index] = 2;
			}else if(bimodal_table[index] == -1){
				bimodal_table[index] = -2;
			}
		}else{
			if(bimodal_table[index] == 1 || bimodal_table[index] == -2){
				bimodal_table[index] = -1;
			}else{
				bimodal_table[index] = 1;
			}
		}

		if(actual){
			hist = hist.substr(1, hist.size()).append("1");
		}else{
			hist = hist.substr(1, hist.size()).append("0");
		}
	}
	
	vector<int> res;
	res.push_back(accuracy_num);
	res.push_back(tb.size());
	return res;
	
}



