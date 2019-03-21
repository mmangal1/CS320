#ifndef BRANCH_PRED_H
#define BRANCH_PRED_H
#include <vector>
#include <string>

struct table{
	unsigned long long addr;
	int taken;
};

std::vector<int> alwaysTaken(std::vector<table> tb);
std::vector<int> alwaysNotTaken(std::vector<table> tb);
std::vector<int> bimodalSingle(std::vector<table> tb, int size);
std::vector<int> bimodalDouble(std::vector<table> tb, int size);
void outputResult(std::vector<std::vector<std::vector<int>>> res_vec, std::string outputfile);

#endif
