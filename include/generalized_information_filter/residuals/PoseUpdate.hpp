/*
 * PoseUpdate.hpp
 *
 *  Created on: Aug 22, 2016
 *      Author: Bloeschm
 */

#ifndef GIF_POSEUPDATE_HPP_
#define GIF_POSEUPDATE_HPP_

#include "generalized_information_filter/common.hpp"
#include "generalized_information_filter/UnaryUpdate.hpp"

namespace GIF{

class PoseMeas: public State{
 public:
  PoseMeas(const V3D& pos = V3D(0,0,0), const QPD& att = QPD()):
      State(ElementPack<V3D,QPD>::makeStateDefinition({"pos","att"})),
      pos_(State::getValue<V3D>("pos")),
      att_(State::getValue<QPD>("att")){
    pos_ = pos;
    att_ = att;
  };
  V3D& pos_;
  QPD& att_;
};

class PoseUpdate: public UnaryUpdate<ElementPack<V3D,QPD>,ElementPack<V3D,QPD>,ElementPack<V3D,V3D>,PoseMeas>{
 public:
  PoseUpdate(): mtUnaryUpdate({"pos","att"},{"pos","att"},{"pos","att"}){
    dt_ = 0.1;
  };
  virtual ~PoseUpdate(){};
  void evalUnaryUpdateImpl(       V3D& posInn,  QPD& attInn,
                            const V3D& posSta ,const QPD& attSta,
                            const V3D& posNoi ,const V3D& attNoi) const{
    posInn = posSta - meas_->pos_ + posNoi;
    QPD dQ = dQ.exponentialMap(attNoi);
    attInn = dQ*attSta*meas_->att_.inverted();
  }
  void jacStaUnaryUpdateImpl(  MXD& J,
                               const V3D& posSta ,const QPD& attSta,
                               const V3D& posNoi ,const V3D& attNoi) const{
    J.setZero();
    setJacBlockPos<POS,POS>(J,M3D::Identity());
    setJacBlockPos<ATT,ATT>(J,M3D::Identity());
  }
  void jacNoiUnaryUpdateImpl(  MXD& J,
                               const V3D& posSta ,const QPD& attSta,
                               const V3D& posNoi ,const V3D& attNoi) const{
    J.setZero();
    setJacBlockNoi<POS,POS>(J,M3D::Identity());
    setJacBlockNoi<ATT,ATT>(J,M3D::Identity());
  }

 protected:
  double dt_;
  enum Elements{POS, ATT};
};

}

#endif /* GIF_POSEUPDATE_HPP_ */
