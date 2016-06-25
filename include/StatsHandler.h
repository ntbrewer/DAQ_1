// Small class to handle the statistics information from pixies

#ifndef __STATSHANDLER_H_
#define __STATSHANDLER_H_ 1

#include <string>
#include <vector>

#include "pixie16app_defs.h"

template<typename T>
class ChannelArray : public std::vector<T>
{
 public:
 ChannelArray() : std::vector<T>(NUMBER_OF_CHANNELS, T()) {};
  void assign(const T &t) {
    std::vector<T>::assign(NUMBER_OF_CHANNELS, t);
  };
};

class StatsHandler
{
 private:
  typedef ChannelArray<unsigned int> channel_counter_t;
  typedef ChannelArray<double> channel_rates_t;
  std::vector<channel_counter_t> nEventsDelta;
  std::vector<channel_counter_t> nEventsTotal;
  std::vector<size_t> dataDelta;
  std::vector<size_t> dataTotal;
  std::vector<channel_rates_t> calcEventRate;
  std::vector<size_t> calcDataRate;
  double timeElapsed; // in seconds
  double totalTime;   // in seconds
  double dumpTime; // time in seconds before data is dumped
  size_t numCards;

  static const std::string dumpFile;

  void Dump(void);
 public:
  StatsHandler(size_t nCards = 1);
  ~StatsHandler();

  void AddEvent(unsigned int mod, unsigned int ch, size_t size);
  void AddTime(double dtime);

  double GetDataRate(size_t mod);
  double GetEventRate(size_t mod);
};

#endif // __STATSHANDLER_H_ 
