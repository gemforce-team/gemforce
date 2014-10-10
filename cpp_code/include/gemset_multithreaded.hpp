#ifndef _GEMSET_MULTITHREADED_HPP
#define _GEMSET_MULTITHREADED_HPP

#include <vector>
#include <mutex>
#include <thread>
#include <semaphore.h>

using namespace std;
typedef pair<vector<Gem*>*, vector<Gem*>*> job;

const int BLOCK_SIZE=1000;
const int MIN_THREAD_SIZE=BLOCK_SIZE*BLOCK_SIZE;
const int THREADS_N=10;
sem_t sema;

void worker (
  vector<job>*      sources,
  int               size,
  vector<Gem>*      current_pool,
  mutex             *m,
  void              (*limiter) (vector<Gem>*&)
)
{
  sem_wait(&sema);
  vector<Gem> *tmp = new vector<Gem>;
  tmp->resize(size);
  int k=0;
  for (auto p : *sources)
    for (Gem* g1 : *p.first)
      for (Gem* g2 : *p.second)
        (*tmp)[k++]=combine(g1, g2);
  limiter(tmp);
  sem_post(&sema);
  m->lock();
  cout<<"thread size = "<<size<<"\n";
  for (Gem a : *tmp)
    current_pool->push_back(a);
  if (current_pool->size()>1000*1000)
  {
    cout<<"overflow: "<<current_pool->size()<<" ";
    limiter(current_pool);
    current_pool->reserve(1000*1000);
    cout<<current_pool->size()<<"\n";
  }
  m->unlock();
  delete tmp;
  delete sources;
}

vector<Gem*>** generate_gemset_multithreaded (
  vector<Gem*>*  base_gems,
  int           limit_value,
  void          (*limiter) (vector<Gem>*&)
)
{
  vector<Gem*>** gemset = new vector<Gem*>*[limit_value+1];
  vector<vector<Gem*>*>*pools = new vector<vector<Gem*>*>[limit_value+1];
  
  for (int i=1; i<=limit_value; i++)
    gemset[i]=new vector<Gem*>;
  for (Gem* e:*base_gems)
    gemset[e->value]->push_back(e);
  pools[1].push_back(new vector<Gem*>);
  for (Gem* e:*gemset[1])
    pools[1].back()->push_back(e);
  
  sem_init(&sema, 0, THREADS_N);
  int s=0, sum=0, last_sum=0;
  for (int i=2; i<=limit_value; i++)
  {
    cout<<"Iteration: "<<i<<"\n";
    vector<Gem>* current_pool = new vector<Gem>;
    vector<thread*> threads;
    mutex m;
    vector<job>* bundle = new vector<job>;
    
    for (int first_val=1; 2*first_val<=i; first_val++)
    {
      int second_val=i-first_val;
      for (vector<Gem*>* p1 : pools[first_val])
        for (vector<Gem*>* p2 : pools[second_val])
        {
          bundle->push_back(make_pair(p1, p2));
          s+=p1->size()*p2->size();
          sum+=p1->size()*p2->size();
          if (s>=min(last_sum/THREADS_N, MIN_THREAD_SIZE))
          {
            threads.push_back(new thread(&worker, bundle, s, current_pool, &m, limiter));
            bundle = new vector<job>;
            s=0;
          }
        }
    }
    
    last_sum=sum;
    sum=0;
    
    if(s!=0)
      threads.push_back(new thread(&worker, bundle, s, current_pool, &m, limiter));
    else
      delete bundle;

    for (thread* th : threads)
    {
      th->join();
      delete th;
    }
    
    limiter(current_pool);
    for (Gem e : *current_pool)
      gemset[i]->push_back(new Gem(e));
    delete current_pool;
    pools[i].push_back(new vector<Gem*>);
    for (Gem* e : *gemset[i])
    {
      if (pools[i].back()->size()>=BLOCK_SIZE)
        pools[i].push_back(new vector<Gem*>);
      pools[i].back()->push_back(e);
    }
    cout<<"Pool size after limiting: "<<gemset[i]->size()<<"\n";
    cout<<"End of iteration "<<i<<"\n\n\n";
  }
  for (int i=0; i<=limit_value; i++)
    for (vector<Gem*>* v : pools[i])
      delete v;
  delete [] pools;
  return gemset;
}

#endif // _GEMSET_MULTITHREADED_HPP
