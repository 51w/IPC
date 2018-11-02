#include <vector>
#include <deque>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <string.h>
#include "bufpool.h"

#define MinSize   1000*1024
#define MaxChnn   5
#define SizeVal   2000*1024
#define MaxNode   8

typedef struct
{
	BufData datapool;
	int	    start_pos;
	int     end_pos;
}BufList;

class Bufpool
{
public:
    Bufpool(int size=SizeVal, int chn=2);
    ~Bufpool();

    int push(BufData &data);

    int pop(BufData *data, int chn=0);

    int insert_buf(int start, int len, BufData &mdat);

private:
    int _size;
    int _chnn;
    int _bpos;

    std::vector<std::deque<BufList> > buflist;
    unsigned char *_bptr;
    pthread_mutex_t mutex;
    pthread_cond_t  condn;
};

Bufpool::Bufpool(int size, int chn)
    : _size(size), _chnn(chn), _bpos(0), _bptr(NULL)
{
    _size = (_size>MinSize) ? _size:MinSize;
    _chnn = (_chnn<MaxChnn) ? _chnn:MaxChnn;
    
    buflist.resize(_chnn);
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
    if(start+len > _size) return 0;

	pthread_mutex_lock(&mutex);
    for(int i=0; i<_chnn; i++)
    {
        while(buflist[i].size())
        {
            if(check_ANDSET(start, len, buflist[i].front().start_pos, buflist[i].front().end_pos) )
                buflist[i].pop_front();
            else
                break;
        }  
	}
	pthread_mutex_unlock(&mutex);

    memcpy(_bptr+start, mdat.bptr, len);
	
	// List Insert
	pthread_mutex_lock(&mutex);
    for(int i=0; i<_chnn; i++)
    {
        BufList input;
        input.start_pos         = start;
		input.end_pos           = start+len;
		input.datapool.len 		= mdat.len;
		input.datapool.timestamp= mdat.timestamp;
		input.datapool.datetype = mdat.datetype;
		input.datapool.naltype 	= mdat.naltype;
		input.datapool.bptr 	= _bptr+start;
        
        buflist[i].push_back(input);

        if(buflist[i].size() > MaxNode)
        buflist[i].pop_front();
    }
	pthread_mutex_unlock(&mutex);

    return 1;
}

int Bufpool::push(BufData &data)
{
    int len = data.len;
	if(_chnn<1 || len>_size || len<4) return 0;

    int ret = 0;
	//int broadcast = 0;
	//for(int i=0; i<_chnn; i++)
    //{
	//	broadcast += (buflist[i].size() != 0)?0:1;
	//}

    //Copy to the position//
	if(len+_bpos > _size){
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

    //if(broadcast)
	pthread_cond_broadcast(&condn);

    return ret;
}

int Bufpool::pop(BufData *data, int chn)
{
    if(chn<0 || chn>=_chnn || buflist.size()==0)
		return 0;

    pthread_mutex_lock(&mutex);

	while(buflist[chn].size() == 0)
	{
		struct timeval now;
		struct timespec outtime;
		gettimeofday(&now, NULL);
		outtime.tv_sec = now.tv_sec + 3;
		outtime.tv_nsec = now.tv_usec * 1000;
		
		int ret;
		ret = pthread_cond_timedwait(&condn, &mutex, &outtime);
		if(ret != 0) //g++ -lpthread 否则无法正常运行，上面一直循环
        {
			pthread_mutex_unlock(&mutex);
			printf("[CHN%d] timeout\n", chn);
			return 0;
		}
	}

    *data = buflist[chn].front().datapool;
    buflist[chn].pop_front();
	
	pthread_mutex_unlock(&mutex);	
	return 1;
}



#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

Bufpool *_bufpool;
int bufpool_init(int size, int channel)
{
	_bufpool = new Bufpool(size, channel);
}

void bufpool_exit()
{
	delete _bufpool;
}

int PushBuf(BufData data)
{
	int ret = 0;
	
	if(_bufpool != NULL)
		ret = _bufpool->push(data);
	else
		return 0;
	
	return ret;
}

int PopBuf(BufData *data, int channel)
{
	memset(data, 0, sizeof(BufData));
	int ret = 0;
	
	if(_bufpool != NULL)
		ret = _bufpool->pop(data, channel);
	else
		return 0;
	
	return ret;
}

#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif