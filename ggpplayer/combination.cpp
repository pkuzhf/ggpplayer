#include "combination.h"

int compareCombination(vector<int> &comb_a, vector<int> &comb_b, vector<int> &keys) {
	for (int i = 0; i < keys.size(); ++i) {
		if (comb_a[keys[i]] < comb_b[keys[i]]) {
			return -1;
		} else if (comb_a[keys[i]] > comb_b[keys[i]]) {
			return 1;
		}
	}
	return 0;
}

void quickSortCombinations(vector<vector<int> > &combinations, vector<int> &keys, vector<int> &idx, int left, int right) {
	if (left >= right) return;
	//int pos = rand() % (right - left + 1) + left;
	int pos = (left + right) / 2;
	int x = idx[pos];
	idx[pos] = idx[left];
	int i = left;
	int j = right;
	while (i < j) {
		while (i < j && compareCombination(combinations[x], combinations[idx[j]], keys) <= 0) {
			--j;
		}
		if (i < j) {
			idx[i] = idx[j];
			++i;
		}
		while (i < j && compareCombination(combinations[idx[i]], combinations[x], keys) == -1) {
			++i;
		}
		if (i < j) {
			idx[j] = idx[i];
			--j;
		}
	}
	idx[i] = x;
	quickSortCombinations(combinations, keys, idx, left, i - 1);
	quickSortCombinations(combinations, keys, idx, i + 1, right);
}

vector<int> sortCombinations(vector<vector<int> > &combinations, vector<int> &keys) {
	vector<int> idx;
	idx.reserve(combinations.size());
	for (int i = 0; i < combinations.size(); ++i) {
		idx.push_back(i);
	}
	if (keys.size() > 0) {
		quickSortCombinations(combinations, keys, idx, 0, idx.size() - 1);
	}
	return idx;
}

vector<vector<int> > delNotCombinations(vector<vector<int> > &c, vector<vector<int> > &not_c, vector<int> &idx_c, vector<int> &idx_not_c, vector<int> &keys) {
	vector<vector<int> > ret;
	ret.reserve(c.size());
	int i = 0;
	int j = 0;
	
	while (i < idx_c.size() && j < idx_not_c.size()) {
		int compare = compareCombination(c[idx_c[i]], not_c[idx_not_c[j]], keys);
		if (compare == -1) {
			ret.push_back(c[idx_c[i]]);
			++i;
		} else if (compare == 1) {
			++j;
		} else {
			++i;
		}
	}
	while (i < idx_c.size()) {
		ret.push_back(c[idx_c[i]]);
		++i;
	}
	return ret;
}

vector<vector<int> > mergeTwoCombinations(vector<vector<int> > &a, vector<vector<int> > &b, vector<int> &idx_a, vector<int> &idx_b, vector<int> &keys, int size) {
	vector<vector<int> > ret;
	ret.reserve(size);
	int a_begin = 0;
	int b_begin = 0;
	while (a_begin < idx_a.size() && b_begin < idx_b.size()) {
		int compare = compareCombination(a[idx_a[a_begin]], b[idx_b[b_begin]], keys);
		if (compare == -1) {
			++a_begin;
		} else if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (a_end < idx_a.size() && compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (b_end < idx_b.size() && compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
				++b_end;
			}
			for (int i = a_begin; i < a_end; ++i) {
				for (int j = b_begin; j < b_end; ++j) {
					vector<int> &c1 = a[idx_a[i]];
					vector<int> &c2 = b[idx_b[j]];
					int size = c1.size();
					vector<int> c3(size, -1);
					for (int k = 0; k < size; ++k) {
						if (c1[k] != -1) c3[k] = c1[k];
						if (c2[k] != -1) c3[k] = c2[k];
					}
					ret.push_back(c3);
				}
			}
			a_begin = a_end;
			b_begin = b_end;
		}
	}
	return ret;
}

long long calcCombineCost(
	vector<vector<int> > &a, 
	vector<vector<int> > &b, 
	vector<int> &keys,
	vector<int> &idx_a,
	vector<int> &idx_b) {

	long long ret = 0;
	int a_begin = 0;
	int b_begin = 0;
	while (a_begin < idx_a.size() && b_begin < idx_b.size()) {
		int compare = compareCombination(a[idx_a[a_begin]], b[idx_b[b_begin]], keys);
		if (compare == -1) {
			++a_begin;
		} else if (compare == 1) {
			++b_begin;
		} else {
			int a_end = a_begin;
			int b_end = b_begin;
			while (a_end < idx_a.size() && compareCombination(a[idx_a[a_begin]], a[idx_a[a_end]], keys) == 0) {
				++a_end;
			}
			while (b_end < idx_b.size() && compareCombination(b[idx_b[b_begin]], b[idx_b[b_end]], keys) == 0) {
				++b_end;
			}
			ret += (a_end - a_begin) * (b_end - b_begin);			
			a_begin = a_end;
			b_begin = b_end;
		}
	}
	return ret;
}

vector<int> getCombination(vector<int> all_variables, vector<int> &variables, vector<int> values) {
	vector<int> ret(all_variables.size(), -1);
	for (int i = 0; i < all_variables.size(); ++i) {
		for (int j = 0; j < variables.size(); ++j) {
			if (all_variables[i] == variables[j]) {
				ret[i] = values[j];
				break;
			}
		}
	}
	return ret;
}

vector<int> getKeys(vector<int> &combination) {
	vector<int> ret;
	for (int i = 0; i < combination.size(); ++i) {
		if (combination[i] != -1) {
			ret.push_back(i);
		}
	}
	return ret;
}

vector<int> getCommonKeys(vector<int> &a, vector<int> &b, int &idxa, int &idxb) {
	vector<int> ret;
	idxa = 0;
	idxb = 0;
	for (int i = 0; i < a.size(); ++i) {
		if (a[i] != -1) {
			idxa *= 2;
		}
		if (b[i] != -1) {
			idxb *= 2;
		}
		if (a[i] != -1 && b[i] != -1) {
			ret.push_back(i);
			++idxa;
			++idxb;
		}		
	}
	return ret;
}

void uniqCombinations(vector<vector<int> > combinations) {
	vector<vector<int> > new_c;
	vector<int> idx;
	vector<int> keys;
	if (combinations.size() > 0) {
		keys = getKeys(combinations[0]);
	}
	idx = sortCombinations(combinations, keys);
	bool has_duplicates = false;
	for (int i = 1; i < idx.size(); ++i) {
		if (compareCombination(combinations[idx[i]], combinations[idx[i - 1]], keys) == 0) {
			has_duplicates = true;
			break;
		}
	}
	if (has_duplicates) {		
		for (int i = 0; i < idx.size(); ++i) {
			if (i == 0 || compareCombination(combinations[idx[i]], combinations[idx[i - 1]], keys) != 0) {
				new_c.push_back(combinations[idx[i]]);
			}
		}
		combinations = new_c;
	}
}

vector<vector<int> > & mergeMultipleCombinations(
	vector<vector<vector<int> > > &multiple_combinations, 
	vector<vector<vector<int> > > &multiple_not_combinations,
	vector<vector<vector<int> > > &multiple_combinations_indexes,
	vector<vector<int> > &multiple_not_combinations_indexes,
	vector<pair<int, int> > &variable_distincts,
	vector<pair<int, int> > &constant_distincts) {	
		
	int size = multiple_combinations.size();
	if (size == 0) {
		multiple_combinations.push_back(vector<vector<int> >());
		return multiple_combinations[multiple_combinations.size() - 1];
	}
	for (int i = 0; i < size; ++i) {
		if (multiple_combinations[i].size() == 0) {
			return vector<vector<int> >();
		}
	}
	vector<vector<long long> > combine_cost(size, vector<long long>(size, -1));
	vector<vector<vector<int> > > keys(size, vector<vector<int> >(size, vector<int>()));
	vector<vector<pair<vector<int>, vector<int> > > > idxes(size, vector<pair<vector<int>, vector<int> > >(size, pair<vector<int>, vector<int> >()));	
	for (int i = 0; i < size; ++i) {
		for (int j = i + 1; j < size; ++j) {
			vector<vector<int> > &a = multiple_combinations[i];
			vector<vector<int> > &b = multiple_combinations[j];
			int idxa;
			int idxb;
			if (a.size() > 0 && b.size() > 0) {
				keys[i][j] = getCommonKeys(a[0], b[0], idxa, idxb);
			}
			if (i < multiple_combinations_indexes.size()) {
				idxes[i][j].first = multiple_combinations_indexes[i][idxa];
			} else {
				idxes[i][j].first = sortCombinations(a, keys[i][j]);
			}
			if (j < multiple_combinations_indexes.size()) {
				idxes[i][j].second = multiple_combinations_indexes[j][idxb];
			} else {
				idxes[i][j].second = sortCombinations(b, keys[i][j]);
			}
			combine_cost[i][j] = calcCombineCost(a, b, keys[i][j], idxes[i][j].first, idxes[i][j].second);
			if (combine_cost[i][j] == 0) {
				multiple_combinations.push_back(vector<vector<int> >());
				return multiple_combinations[multiple_combinations.size() - 1];;
			}
		}
	}

	vector<bool> deleted(size, false);	
	vector<vector<int> > not_keys(multiple_not_combinations.size(), vector<int>());
	vector<vector<int> > not_idxes(multiple_not_combinations.size(), vector<int>());
	vector<bool> not_used(multiple_not_combinations.size(), false);
	for (int i = 0; i < multiple_not_combinations.size(); ++i) {
		if (multiple_not_combinations[i].size() == 0) {
			not_used[i] = true;
		} else {
			for (int j = 0; j < multiple_not_combinations[i][0].size(); ++j) {
				if (multiple_not_combinations[i][0][j] != -1) {
					not_keys[i].push_back(j);
				}
			}
			if (i < multiple_not_combinations_indexes.size()) {
				not_idxes[i] = multiple_not_combinations_indexes[i];
			} else {
				not_idxes[i] = sortCombinations(multiple_not_combinations[i], not_keys[i]);
			}
		}		
	}
	while(true) {
		long long min_cost = -1;
		int c1 = -1;
		int c2 = -1;
		for (int j = 0; j < size; ++j) {
			if (deleted[j]) continue;
			for (int k = j + 1; k < size; ++k) {
				if (deleted[k]) continue;
				if (combine_cost[j][k] < min_cost || min_cost == -1) {
					min_cost = combine_cost[j][k];
					c1 = j;
					c2 = k;
				}
			}
		}
		if (min_cost == -1) {
			break;
		}
		multiple_combinations.push_back(mergeTwoCombinations(multiple_combinations[c1], multiple_combinations[c2], idxes[c1][c2].first, idxes[c1][c2].second, keys[c1][c2], min_cost));
		keys.push_back(vector<vector<int> >(size + 1, vector<int>()));
		idxes.push_back(vector<pair<vector<int>, vector<int> > > (size + 1, pair<vector<int>, vector<int> >()));
		combine_cost.push_back(vector<long long>(size + 1, 0));
		deleted.push_back(false);

		deleted[c1] = true;
		deleted[c2] = true;		
		for (int j = 0; j < size; ++j) {
			if (deleted[j]) continue;
			int x = j;
			int y = multiple_combinations.size() - 1;
			vector<vector<int> > &a = multiple_combinations[x];
			vector<vector<int> > &b = multiple_combinations[y];
			keys[x].push_back(vector<int>());	
			int idxa;
			int idxb;
			if (a.size() > 0 && b.size() > 0) {
				keys[x][y] = getCommonKeys(a[0], b[0], idxa, idxb);
			}
			idxes[x].push_back(pair<vector<int>, vector<int> >());
			if (x < multiple_combinations_indexes.size()) {
				idxes[x][y].first = multiple_combinations_indexes[x][idxa];
			} else {
				idxes[x][y].first = sortCombinations(a, keys[x][y]);
			}
			idxes[x][y].second = sortCombinations(b, keys[x][y]);
			combine_cost[x].push_back(0);
			combine_cost[x][y] = calcCombineCost(a, b, keys[x][y], idxes[x][y].first, idxes[x][y].second);
			if (combine_cost[x][y] == 0) {
				multiple_combinations.push_back(vector<vector<int> >());
				return multiple_combinations[multiple_combinations.size() - 1];
			}
		}
		size = multiple_combinations.size();
	}
	
	vector<vector<int> > &ret = multiple_combinations[size - 1];
	for (int j = 0; j < multiple_not_combinations.size(); ++j) {
		if (not_used[j] || ret.size() == 0) continue;
		vector<vector<int> > &not_c = multiple_not_combinations[j];
		bool matched = true;				
		for (int k = 0; k < not_keys[j].size(); ++k) {
			if (not_c[0][not_keys[j][k]] != -1 && ret[0][not_keys[j][k]] == -1) {
				matched = false;
				break;						
			}
		}				
		if (matched) {
			vector<int> idx = sortCombinations(ret, not_keys[j]);
			ret = delNotCombinations(ret, not_c, idx, not_idxes[j], not_keys[j]);
			not_used[j] = true;
		}
	}
	for (vector<vector<int> >::iterator j = ret.begin(); j != ret.end();) {
		bool deleted = false;				
		for (int k = 0; k < variable_distincts.size() && !deleted; ++k) {
			pair<int, int> d = variable_distincts[k];
			if ((*j)[d.first] != -1 && (*j)[d.second] != -1 && (*j)[d.first] == (*j)[d.second]) {
				deleted = true;
			}
		}
		if (deleted) {
			j = ret.erase(j);
		} else {
			++j;
		}
	}
	for (int i = 0; i < size; ++i) {
		vector<vector<int> > &c = multiple_combinations[i];		
		for (vector<vector<int> >::iterator j = c.begin(); j != c.end(); ) {
			bool deleted = false;
			for (int k = 0; k < constant_distincts.size() && !deleted; ++k) {
				pair<int, int> d = constant_distincts[k];
				if ((*j)[d.first] != -1 && (*j)[d.first] == d.second) {
					deleted = true;					
				}
			}
			for (int k = 0; k < variable_distincts.size() && !deleted; ++k) {
				pair<int, int> d = variable_distincts[k];
				if ((*j)[d.first] != -1 && (*j)[d.second] != -1 && (*j)[d.first] == (*j)[d.second]) {
					deleted = true;
				}
			}
			if (deleted) {
				j = c.erase(j);
			} else {
				++j;
			}
		}
	}
	return ret;
}
