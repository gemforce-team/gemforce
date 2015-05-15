#ifndef _RED_ADDER_H
#define _RED_ADDER_H

gem* gem_explore(gem* gemf, int* isRed, gem* pred, int last, int* curr, gem* new_array, int* new_index)
{
	if (gemf->father==NULL || *isRed) return gemf;
	if (gemf->father->father==NULL) {    // father is g1
		if (*curr < last) (*curr)++;
		else {
			(*new_index)++;
			gem* gemt=new_array+(*new_index);
			gem_combine(pred, gemf->mother, gemt);
			*isRed=1;
			return gemt;
		}
	}
	if (gemf->mother->father==NULL) {    // mother is g1
		if (*curr < last) (*curr)++;
		else {
			(*new_index)++;
			gem* gemt=new_array+(*new_index);
			gem_combine(gemf->father, pred, gemt);
			*isRed=1;
			return gemt;
		}
	}
	gem* g2= gem_explore(gemf->mother, isRed, pred, last, curr, new_array, new_index);
	gem* g1= gem_explore(gemf->father, isRed, pred, last, curr, new_array, new_index);
	if (g1==gemf->father && g2==gemf->mother) return gemf;
	
	(*new_index)++;
	gem* gemt=new_array+(*new_index);
	gem_combine(g1, g2, gemt);
	return gemt;
}


#endif // _RED_ADDER_H
