#ifndef _INTERVAL_TREE_HPP
#define _INTERVAL_TREE_HPP

#include <cmath>
#include <algorithm>
using namespace std;

class IntervalTree
{
  int* tree;
  int N;
public:
  IntervalTree(int size)
  {
    N=int(pow(2, size));
    tree = new int[2*N];
    clear();
  }
  ~IntervalTree()
  {
    delete [] tree;
  }
  void clear ()
  {
    for (int i=0; i<2*N; i++)
      tree[i]=-1;
  }
  void add_element (int point, int val)
  {
    point+=N;
    while (point!=0)
    {
      tree[point]=max(val,tree[point]);
      point/=2;
    }
  }
  int read_max (int point)
  {
    point+=N;
    int result = tree[point];
    while (point!=1)
    {
      if (point%2==0)
        result=max(result,tree[point+1]);
      point/=2;
    }
    return result;
  }
};

#endif // _INTERVAL_TREE_HPP