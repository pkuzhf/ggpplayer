#ifndef COMBINATION_H
#define COMBINATION_H

#include <vector>

using namespace std;

vector<int> getCombination(vector<int> all_variables, vector<int> &variables, vector<int> values);
vector<int> getKeys(vector<int> &combination);
void uniqCombinations(vector<vector<int> > combinations);
int compareCombination(vector<int> &comb_a, vector<int> &comb_b, vector<int> &keys);
void quickSortCombinations(vector<vector<int> > &combinations, vector<int> &keys, vector<int> &idx, int left, int right);
vector<int> sortCombinations(vector<vector<int> > &combinations, vector<int> &keys);
vector<vector<int> > mergeTwoCombinations(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &idx_a, vector<int> &idx_b, vector<int> &keys, int size);
vector<vector<int> > delNotCombinations(vector<vector<int> > &c, vector<vector<int> > &not_c, vector<int> &idx_c, vector<int> &idx_not_c, vector<int> &keys);
long long calcCombineCost(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &keys, vector<int> &idx_a, vector<int> &idx_b);
vector<int> getCommonKeys(vector<int> &a, vector<int> &b, int &idxa, int &idxb);
vector<vector<int> > mergeMultipleCombinations(vector<vector<vector<int> > > multiple_combinations, vector<vector<vector<int> > > multiple_not_combinations, 
	vector<vector<vector<int> > > multiple_combinations_indexes, vector<vector<int> > multiple_not_combinations_indexes, 
	vector<pair<int, int> > variable_distincts, vector<pair<int, int> > constant_distincts);
vector<vector<int> > mergeMultipleCombinations(vector<vector<vector<int> > > multiple_combinations, 
	vector<pair<int, int> > variable_distincts, vector<pair<int, int> > constant_distincts);



#endif