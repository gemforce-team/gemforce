#ifndef _GEMSET_MULTITHREADED_HPP
#define _GEMSET_MULTITHREADED_HPP

#include <vector>
#include <mutex>
#include <thread>
#include <semaphore.h>

using namespace std;

const int BLOCK_SIZE=10000;
const int THREADS_N=8;
sem_t sema;

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
  for (int i=2; i<=limit_value; i++)
  {
    cout<<"Iteration: "<<i<<"\n";
    vector<Gem>* current_pool = new vector<Gem>;
    vector<thread*> threads;
    mutex m;
    
    for (int first_val=1; 2*first_val<=i; first_val++)
    {
      int second_val=i-first_val;
      for (vector<Gem*>* p1 : pools[first_val])
        for (vector<Gem*>* p2 : pools[second_val])
        {
          sem_wait(&sema);
          threads.push_back(new thread(
            [&m, &current_pool, p1, p2, limiter] ()
            {
              vector<Gem> *tmp = new vector<Gem>;
              tmp->resize(p1->size()*p2->size());
              int k=0;
              for (Gem* g1 : *p1)
                for (Gem* g2 : *p2)
                  (*tmp)[k++]=combine(g1, g2);
              limiter(tmp);
              sem_post(&sema);
              m.lock();
              for (Gem a : *tmp)
                current_pool->push_back(a);
              if (current_pool->size()>1000*1000)
              {
                cout<<"overflow: "<<current_pool->size()<<" ";
                limiter(current_pool);
                current_pool->reserve(1000*1000);
                cout<<current_pool->size()<<"\n";
              }
              m.unlock();
              delete tmp;
            }
          ));
        }
    }
    
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
      if (pools[i].back()->size()>=1000)
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
