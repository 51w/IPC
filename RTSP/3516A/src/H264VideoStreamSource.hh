#ifndef H264VIDEOSTREAMSOURCE_HH
#define H264VIDEOSTREAMSOURCE_HH

#include "FramedSource.hh"

class H264VideoStreamSource: public FramedSource {
public:
  static H264VideoStreamSource* createNew(UsageEnvironment& env);

  H264VideoStreamSource(UsageEnvironment& env);

  // called only by createNew()
  virtual ~H264VideoStreamSource();

private:
  virtual void doGetNextFrame();

protected:
  Boolean isH264VideoStreamFramer() const { return True; }
  unsigned maxFrameSize()  const { return 150000; }
};

#endif

