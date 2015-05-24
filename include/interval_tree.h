#ifndef _INTERVAL_TREE_H
#define _INTERVAL_TREE_H

/* place is always 0..N-1 */

void tree_add_element(int* tree, int N, int place, int val)
{
	place+=N;
	while (place!=1) {
		if (val > tree[place]) {
			tree[place]=val;
			place >>= 1;		// place/=2 bitwise, it's a bit faster
		}
		else return;			// no need to go on, they'll sure be even bigger
	}
	if (val > tree[1]) tree[1]=val;
}

int tree_check_after(int* tree, int N, int place, int val)
{
	place+=N;
	if (val <= tree[place]) return 0;
	while (place!=1) {
		if (place%2==0 && val <= tree[place+1]) return 0;
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	return 1;					// if we never found bigger return 1
}

/* we also have support for an unapproximated tree of floats */

inline float max (float a, float b)
{
	return a>b ? a : b;
}

void ftree_add_element(float* tree, int N, int place, float val)
{
	place+=N;
	while (place!=1) {
		if (val > tree[place]) {
			tree[place]=val;
			place >>= 1;		// place/=2 bitwise, it's a bit faster
		}
		else return;			// no need to go on, they'll sure be even bigger
	}
	if (val > tree[1]) tree[1]=val;
}

float ftree_read_max(float* tree, int N, int place)
{
	place+=N;
	float result = tree[place];
	while (place!=1) {
		if (place%2==0) result=max(result,tree[place+1]);
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	return result;
}

int ftree_check_after(float* tree, int N, int place, float val)
{
	place+=N;
	if (val <= tree[place]) return 0;
	while (place!=1) {
		if (place%2==0 && val <= tree[place+1]) return 0;
		place >>= 1;			// place/=2 bitwise, it's a bit faster
	}
	return 1;					// if we never found bigger return 1
}
#endif // _INTERVAL_TREE_H
