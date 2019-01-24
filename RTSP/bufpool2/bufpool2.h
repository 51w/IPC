#ifdef __cplusplus
#if __cplusplus
extern "C" 
{
#endif
#endif

typedef struct
{
  unsigned char *bptr;
  int  len;
}BufData;

int PushBuf(BufData  data);

int PullBuf(BufData *data);


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif