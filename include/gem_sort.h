#ifndef _GEM_SORT_H
#define _GEM_SORT_H

// black magic to have overloaded function pointers correctly resolved as template params in the sorting functions
// from https://stackoverflow.com/a/36794145
#define AS_LAMBDA(func) [&](auto&&... args) -> decltype(func(std::forward<decltype(args)>(args)...)) { return func(std::forward<decltype(args)>(args)...); }

constexpr int switch_size = 20;

template<class gem, class comparer>
inline void ins_sort(gem* gems, int len, comparer gem_less_equal)
{
	int i,j;
	gem element;
	for (i=1; i<len; i++) {
		element=gems[i];
		for (j=i; j>0 && gem_less_equal(element, gems[j-1]); j--) {
			gems[j]=gems[j-1];
		}
		gems[j]=element;
	}
}

template<class gem, class comparer>
void quick_sort(gem* gems, int len, comparer gem_less_equal)
{
	if (len > switch_size)  {
		gem pivot = gems[len/2];
		gem* beg = gems;
		gem* end = gems+len-1;
		while (beg <= end) {
			while (gem_less_equal(*beg, pivot)) {
				beg++;
			}
			while (gem_less_equal(pivot,*end)) {
				end--;
			}
			if (beg <= end) {
				gem temp = *beg;
				*beg = *end;
				*end = temp;
				beg++;
				end--;
			}
		}
		if (end - gems + 1 < gems - beg + len) {		// sort smaller first
			quick_sort(gems, end - gems + 1, gem_less_equal);
			quick_sort(beg, gems - beg + len, gem_less_equal);
		}
		else {
			quick_sort(beg, gems - beg + len, gem_less_equal);
			quick_sort(gems, end - gems + 1, gem_less_equal);
		}
	}
}

template<class gem, class comparer>
void gem_sort(gem* gems, int len, comparer gem_less_equal)
{
	quick_sort(gems, len, gem_less_equal);		// partially sort
	ins_sort(gems, len, gem_less_equal);		// finish the nearly sorted array
}

#endif // _GEM_SORT_H
