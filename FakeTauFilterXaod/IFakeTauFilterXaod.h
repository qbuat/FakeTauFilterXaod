#ifndef IFAKETAUFILTERXAOD_FAKETAUFILTERXAOD_H
#define IFAKETAUFILTERXAOD_FAKETAUFILTERXAOD_H

#include "PATInterfaces/CorrectionCode.h"
#include "AsgTools/IAsgTool.h"
#include "xAODTruth/TruthParticleContainer.h"

class IFakeTauFilterXaod : virtual public asg::IAsgTool
{

  ASG_TOOL_INTERFACE(IFakeTauFilterXaod)

  public : 

  virtual ~IFakeTauFilterXaod() {};
  virtual StatusCode execute(const xAOD::TruthParticleContainer * TruthParticles) = 0;


};
#endif
