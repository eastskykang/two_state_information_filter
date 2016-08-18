#include "generalized_information_filter/Measurement.hpp"
#include "generalized_information_filter/BinaryResidual.hpp"

namespace GIF{

MeasurementTimeline::MeasurementTimeline(const Duration& maxWaitTime, const Duration& minWaitTime){
  maxWaitTime_ = maxWaitTime;
  minWaitTime_ = minWaitTime;
  lastProcessedTime_ = TimePoint::min();
  hasProcessedTime_ = false;
};

MeasurementTimeline::~MeasurementTimeline(){};

void MeasurementTimeline::addMeas(const std::shared_ptr<const MeasurementBase>& meas, const TimePoint& t){
  if(hasProcessedTime_ && t<=lastProcessedTime_){
    std::cout << "Error: adding measurements before last processed time (will be discarded)" << std::endl;
  } else {
    std::pair<std::map<TimePoint,std::shared_ptr<const MeasurementBase>>::iterator,bool> ret;
    ret = measMap_.insert(std::pair<TimePoint,std::shared_ptr<const MeasurementBase>>(t,meas));
    if(!ret.second){
      std::cout << "Error: measurement already exists!" << std::endl;
    }
  }
}

void MeasurementTimeline::removeProcessedFirst(){
  assert(measMap_.size() > 0);
  lastProcessedTime_ = measMap_.begin()->first;
  measMap_.erase(measMap_.begin());
  hasProcessedTime_ = true;
}

void MeasurementTimeline::removeProcessedMeas(const TimePoint& t){
  assert(measMap_.count(t) > 0);
  measMap_.erase(t);
  lastProcessedTime_ = t;
  hasProcessedTime_ = true;
}

void MeasurementTimeline::clear(){
  measMap_.clear();
  hasProcessedTime_ = false;
}

bool MeasurementTimeline::getLastTime(TimePoint& lastTime) const{
  if(!measMap_.empty()){
    lastTime = measMap_.rbegin()->first;
    return true;
  } else if(hasProcessedTime_){
    lastTime = lastProcessedTime_;
    return true;
  } else {
    return false;
  }
}

TimePoint MeasurementTimeline::getMaximalUpdateTime(const TimePoint& currentTime) const{
  TimePoint maximalUpdateTime = currentTime-maxWaitTime_;
  if(!measMap_.empty()){
    maximalUpdateTime = std::max(maximalUpdateTime,measMap_.rbegin()->first+minWaitTime_);
  } else if(hasProcessedTime_){
    maximalUpdateTime = std::max(maximalUpdateTime,lastProcessedTime_+minWaitTime_);
  }
  return maximalUpdateTime;
}

void MeasurementTimeline::addAllInRange(std::set<TimePoint>& times, const TimePoint& start, const TimePoint& end) const{
  auto it = measMap_.upper_bound(start);
  while (it != measMap_.end() && it->first<= end){
    times.insert(it->first);
    ++it;
  }
}

void MeasurementTimeline::addLastInRange(std::set<TimePoint>& times, const TimePoint& start, const TimePoint& end) const{
  auto it = measMap_.upper_bound(end);
  if(it!=measMap_.begin()){
    --it;
    if(it->first > start){
      times.insert(it->first);
    }
  }
}

void MeasurementTimeline::split(const TimePoint& t0, const TimePoint& t1, const TimePoint& t2, const std::shared_ptr<const BinaryResidualBase>& res){
  addMeas(std::shared_ptr<const MeasurementBase>(),t1);
  res->splitMeasurements(measMap_.at(t2),t0,t1,t2,measMap_.at(t1),measMap_.at(t2));
}

void MeasurementTimeline::split(const std::set<TimePoint>& times, const std::shared_ptr<const BinaryResidualBase>& res){
  for(auto t : times){
    auto it = measMap_.lower_bound(t);
    if(it == measMap_.end() || (it == measMap_.begin() && (!hasProcessedTime_ || lastProcessedTime_ >= t))){
      std::cout << "Error: range error while splitting!" << std::endl;
      continue;
    }
    if(it->first == t){
      // Measurement already available
      continue;
    }
    TimePoint previous = (it == measMap_.begin()) ? lastProcessedTime_ : std::prev(it)->first;
    split(previous,t,it->first,res);
  }
}

void MeasurementTimeline::merge(const TimePoint& t0, const TimePoint& t1, const TimePoint& t2, const std::shared_ptr<const BinaryResidualBase>& res){
  res->mergeMeasurements(measMap_.at(t1),measMap_.at(t2),t0,t1,t2,measMap_.at(t2));
  measMap_.erase(t1); // does not count as processed
}

void MeasurementTimeline::mergeUndesired(const std::set<TimePoint>& times, const std::shared_ptr<const BinaryResidualBase>& res){
  // Merge measurements such that only timepoints remain which are in times or past its end
  for(auto it = measMap_.begin(); it != measMap_.end();){
    if(it->first > *times.rbegin()){
      break;
    }
    if(times.count(it->first) > 0){
      continue;
    }
    if((it == measMap_.begin() && (!hasProcessedTime_ || lastProcessedTime_ > it->first)) || next(it) == measMap_.end()){
      std::cout << "Error: range error while merging!" << std::endl;
      break;
    }
    TimePoint previous = (it == measMap_.begin()) ? lastProcessedTime_ : std::prev(it)->first;
    ++it; // Needs to be increment before erase
    merge(previous,std::prev(it)->first,it->first,res);
  }
}

void MeasurementTimeline::print(const TimePoint& start) const{
  for(auto it = measMap_.begin(); it != measMap_.end();++it){
    std::cout << toSec(it->first-start) << "\t";
  }
  std::cout << std::endl;
}

}
