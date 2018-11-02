#ifdef __cplusplus
#if __cplusplus
extern "C" {
#endif
#endif

typedef struct
{
	unsigned char *bptr;
	int  len;
	int  timestamp;
	int	 naltype;
	int  datetype;	
}BufData;

int PushBuf(BufData data);

int PopBuf(BufData *data, int channel);

int bufpool_init(int size, int channel);

void bufpool_exit();


#ifdef __cplusplus
#if __cplusplus
}
#endif
#endif