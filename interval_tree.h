#ifndef _INTERVAL_TREE_H
#define _INTERVAL_TREE_H

float max (float a, float b)
{
  return a>b ? a : b;
}
// place is always 0..N-1
void tree_add_element(float* tree, int N, int place, float val)
{
	place+=N;
	while (place!=1) {
		tree[place]=max(val,tree[place]);
		place/=2;
	}
	tree[1]=max(val,tree[1]);
}

float tree_read_max(float* tree, int N, int place)
{
	place+=N;
	float result = tree[place];
	while (place!=1) {
		if (place%2==0) result=max(result,tree[place+1]);
		place/=2;
	}
	return result;
}

#endif // _INTERVAL_TREE_H
