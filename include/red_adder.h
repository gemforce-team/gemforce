#ifndef _RED_ADDER_H
#define _RED_ADDER_H

gem* gem_explore(gem* gemf, int* isRed, gem* pred, int* gems_left, gem** new_slot)
{
	if (gemf->father==NULL || *isRed) return gemf;
	if (gemf->father->father==NULL) {    // father is g1
		if (*gems_left > 0) (*gems_left)--;
		else {
			gem* gemt=*new_slot;
			(*new_slot)++;
			gem_combine(pred, gemf->mother, gemt);
			*isRed=1;
			return gemt;
		}
	}
	if (gemf->mother->father==NULL) {    // mother is g1
		if (*gems_left > 0) (*gems_left)--;
		else {
			gem* gemt=*new_slot;
			(*new_slot)++;
			gem_combine(gemf->father, pred, gemt);
			*isRed=1;
			return gemt;
		}
	}
	gem* g2= gem_explore(gemf->mother, isRed, pred, gems_left, new_slot);
	gem* g1= gem_explore(gemf->father, isRed, pred, gems_left, new_slot);
	if (g1==gemf->father && g2==gemf->mother) return gemf;
	
	gem* gemt=*new_slot;
	(*new_slot)++;
	gem_combine(g1, g2, gemt);
	return gemt;
}


#endif // _RED_ADDER_H
