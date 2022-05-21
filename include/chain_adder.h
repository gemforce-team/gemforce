#ifndef _CHAIN_ADDER_H
#define _CHAIN_ADDER_H

#include "container_utils.h"

template<class gem, class chain_init_class, class cfr_class>
gem* gem_putchain_templ(const pool_t<gem>& pool, size_t pool_length, vector<gem>& chain_gems,
                        chain_init_class chain_init, cfr_class cfr_expr)
{
	double best_pow = 0;
	gem* best_gem = NULL;
	vector<gem>& best_array = chain_gems;
	
	for (size_t i = 0; i < pool_length; ++i) {
		int depth = gem_getdepth(pool + i);
		vector<gem> new_array = vector<gem>(depth);
		chain_init(new_array + 0);
		
		vector<const gem*> stack = vector<const gem*>(depth);
		stack[0] = pool + i;
		int stack_length = 1;
		
		while (1) { // loop over subgems
			
			gem* curr_place = new_array + 0;
			gem* next_place = new_array + 1;
			
			// the stack contains only parents up to root, so that we can recombine them up
			while (stack[stack_length - 1]->father != NULL) {
				stack[stack_length] = stack[stack_length - 1]->father;
				stack_length++;
			}
			
			// the stack has a g1 and all its ancestors
			// combine them up from the bottom and build a candidate
			for (int level = stack_length - 1; level > 0; level--, curr_place++, next_place++) {
				if (stack[level] == stack[level - 1]->father)
					gem_combine(curr_place, stack[level - 1]->mother, next_place);
				else
					gem_combine(stack[level - 1]->father, curr_place, next_place);
			}
			
			double new_pow = cfr_expr(*curr_place);
			if (new_pow > best_pow) {
				best_pow = new_pow;
				best_gem = curr_place;

				std::swap(best_array, new_array);
				new_array.resize(depth);
				chain_init(new_array + 0);
			}
			
			pop:
			if (stack[stack_length - 1] == stack[stack_length - 2]->mother) {
				/* we've done the mother, pop recursively
				 * this check may capture the father at the first round, but
				 * if it does it means the father was equal to the mother,
				 * so we're actually skipping useless work
				 */
				stack_length--;
				if (stack_length == 1)
					break;
				else
					goto pop;
			}
			else {
				// we've done the father, but not the mother, push the mother
				stack[stack_length - 1] = stack[stack_length - 2]->mother;
			}
		}
	}
	return best_gem;
}


#endif // _CHAIN_ADDER_H
