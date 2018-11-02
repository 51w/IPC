#ifndef H264VIDEOSTREAMSOURCE_HH
#define H264VIDEOSTREAMSOURCE_HH

#include <pthread.h>
#include <semaphore.h>
#include "FramedSource.hh"
#include "VideoInput.hh"

class H264VideoStreamSource: public FramedSource {
public:
  static H264VideoStreamSource* createNew(UsageEnvironment& env, VideoInput& input);
  void* PollingThread1();
  H264VideoStreamSource(UsageEnvironment& env, VideoInput& input);
  // called only by createNew()
  virtual ~H264VideoStreamSource();

public:
  EventTriggerId eventTriggerId;


private:
  static void incomingDataHandler(void* clientData);
  void incomingDataHandler1();
  virtual void doGetNextFrame();

private:
  pthread_t polling_tid;
  sem_t sem;
  VideoInput& fInput;

protected:
  Boolean isH264VideoStreamFramer() const { return True; }
  unsigned maxFrameSize()  const { return 150000; }
};

#endif

