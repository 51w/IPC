#if __cplusplus
extern "C"{
#endif

int send_rtp(char *data, int size, int nal_type, int nTime);

int init_rtp(char* ip, int port);

int exit_rtp();

#if __cplusplus
}
#endif