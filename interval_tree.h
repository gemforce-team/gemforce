#ifndef _INTERVAL_TREE_H
#define _INTERVAL_TREE_H

const int N=1024*1024;

float tree_tab[2*N];

float max (float a, float b)
{
  return a>b ? a : b;
}

void add_maximum_at_interval (int beg, int end, float val)
{
  beg+=N;
  end+=N;
  tree_tab[beg]=max(tree_tab[beg], val);
  tree_tab[end]=max(tree_tab[end], val);
  while (beg/2!=end/2)
  {
    if (!(beg&1))
      tree_tab[beg+1]=max(tree_tab[beg+1], val);
    if (end&1)
      tree_tab[end-1]=max(tree_tab[end-1], val);
    beg/=2;
    end/=2;
  }
}

float read_maximum_at_point (int point)
{
  point+=N;
  float result=0;
  while (point!=1)
  {
    result=max(result, tree_tab[point]);
    point/=2;
  }
  return result;
}

#endif // _INTERVAL_TREE_H
