#include <vector>
#include <deque>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>

#include "bufpool2.h"
#define BUFF_SIZE  2*1024*1024
#define Max_Node   10

typedef struct
{
  BufData datapool;
  int	    start_pos;
  int     end_pos;
}BufList;

class Bufpool
{
public:
  Bufpool(int size = BUFF_SIZE);
  ~Bufpool();
  int push(BufData &data);
  int pull(BufData *data);
  int insert_buf(int start, int len, BufData &mdat);
  
private:
  int _size;
  int _bpos;
  unsigned char *_bptr;
  std::deque<BufList> buflist;
    
  pthread_mutex_t mutex;
  pthread_cond_t  condn;
};

Bufpool::Bufpool(int size)
    : _size(size), _bpos(0), _bptr(NULL)
{
  printf("==========>> %d\n", _size);
  _bptr = (unsigned char*)malloc(_size);

  pthread_cond_init( &condn, NULL);
  pthread_mutex_init(&mutex, NULL);	
}

Bufpool::~Bufpool()
{
  if(_bptr!=NULL)  free(_bptr);

  pthread_cond_destroy( &condn);
  pthread_mutex_destroy(&mutex);
}

int check_ANDSET(int a, int len, int x, int y)
{
	if(x >= a+len || y <= a)
		return 0;
	else
		return 1;
}

int Bufpool::insert_buf(int start, int len, BufData &mdat)
{
  while(buflist.size())
  {
    if(check_ANDSET(start, len, buflist.front().start_pos, buflist.front().end_pos) )
    { buflist.pop_front(); }
    else  break;
  }

  memcpy(_bptr+start, mdat.bptr, len);
	
  BufList input;
  input.start_pos     = start;
  input.end_pos       = start+len;	
  input.datapool.len  = mdat.len;
  input.datapool.bptr = _bptr+start;    
  buflist.push_back(input);

  if(buflist.size() > Max_Node)
  buflist.pop_front();

  return 1;
}

int Bufpool::push(BufData &data)
{
  int len = data.len;
  if(len>_size || len<=0) return 0;

  pthread_mutex_lock(&mutex);

  int ret = 0;
  if(len+_bpos > _size)
  {
    ret = insert_buf(    0, len, data);
    _bpos = len;
  }
  else if(len+_bpos == _size){
    ret = insert_buf(_bpos, len, data);
    _bpos = 0;
  }
  else{
    ret = insert_buf(_bpos, len, data);
    _bpos = len+_bpos;
  }
	
  if(buflist.size()==1)
    pthread_cond_broadcast(&condn);

  pthread_mutex_unlock(&mutex);
  return ret;
}

int Bufpool::pull(BufData *data)
{
  pthread_mutex_lock(&mutex);

  while(buflist.size() == 0)
  {
    struct timeval now;
    struct timespec outtime;
    gettimeofday(&now, NULL);
    outtime.tv_sec = now.tv_sec + 3;
    outtime.tv_nsec = now.tv_usec * 1000;
		
    int ret = pthread_cond_timedwait(&condn, &mutex, &outtime);
    if(ret != 0) //g++ -lpthread 否则无法正常运行，上面一直循环
    {
      pthread_mutex_unlock(&mutex);
      data->len = 0;
      printf("[Bufpool pull] timeout\n");
      return 0;
    }
  }
	
  data->len = buflist.front().datapool.len;
  memcpy(data->bptr, buflist.front().datapool.bptr, data->len);
  buflist.pop_front();
	
  pthread_mutex_unlock(&mutex);	
  return 1;
}



#ifdef __cplusplus
#if __cplusplus
extern "C" 
{
#endif
#endif

Bufpool _videobuf;

int PushBuf(BufData data)
{
  return _videobuf.push(data);
}

int PullBuf(BufData *data)
{
  return _videobuf.pull(data);
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif