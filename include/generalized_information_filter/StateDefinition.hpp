/*
 * StateDefinition.hpp
 *
 *  Created on: Jul 29, 2016
 *      Author: Bloeschm
 */

#ifndef GIF_STATEDEFINITION_HPP_
#define GIF_STATEDEFINITION_HPP_

#include <unordered_map>

#include "generalized_information_filter/common.hpp"
#include "generalized_information_filter/ElementDefinition.hpp"
#include "generalized_information_filter/State.hpp"

namespace GIF{

class StateWrapper;

class StateDefinition{
 public:
  StateDefinition(){};
  ~StateDefinition(){};
  std::shared_ptr<State> newState() const{
    std::shared_ptr<State> newState(new State());
    for(auto d : elementDefinitions_){
      newState->addElement(d->newElement());
    }
    return newState;
  }
  template<typename T>
  int addElementDefinition(const std::string& name){
    auto query = namesMap_.find(name);
    if (query != namesMap_.end()){
      return query->second;
    } else {
      std::shared_ptr<ElementDefinitionBase> newDefinition(new ElementDefinition<T>());
      elementDefinitions_.push_back(newDefinition);
      namesMap_.insert(std::pair<std::string, int>(name,elementDefinitions_.size()-1));
      return elementDefinitions_.size()-1;
    }
  }

 protected:
  std::vector<std::shared_ptr<ElementDefinitionBase>> elementDefinitions_;
  std::unordered_map<std::string, int> namesMap_;

  friend StateWrapper;
};

class StateWrapper{ // TODO: implement definiton checking (Wrapper-State, Wrapper-Definition)
 public:
  StateWrapper(){};
  ~StateWrapper(){};
  void computeMap(){
    indexMap_.resize(outDefinition_->namesMap_.size());
    for(auto outElement : outDefinition_->namesMap_){
      indexMap_.at(outElement.second) = inDefinition_->namesMap_.at(outElement.first);
    }
  }
  void wrap(const std::shared_ptr<State>& out, const std::shared_ptr<State>& in){
    for(int i=0;i<indexMap_.size();i++){
      out->getElement(i) = in->getElement(indexMap_[i]);
    }
  }

 protected:
  std::shared_ptr<StateDefinition> inDefinition_;
  std::shared_ptr<StateDefinition> outDefinition_;
  std::vector<int> indexMap_;
};

}

#endif /* GIF_STATEDEFINITION_HPP_ */
