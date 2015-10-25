#ifndef FAKETAUFILTERXAOD_FILTERVALIDATION_H
#define FAKETAUFILTERXAOD_FILTERVALIDATION_H

#include <map>

#include "TH1F.h"
#include "TH2F.h"

#include "xAODTau/TauJet.h"
#include "FakeTauFilterXaod/FakeTauFilterXaod.h"
#include "EventLoop/Worker.h"

class FilterValidation

{
 public: 
  FilterValidation (const std::string & name);
  virtual ~ FilterValidation() {};

  void book();
  void fill_histograms(const xAOD::TauJet * tau, const TruthFakeTau * truthFakeTau, const double & weight=1.0);
  void record(EL::Worker * wk);

  std::map<std::string, TH1F*> Histograms() {return m_h1d;}
  std::map<std::string, TH2F*> Maps() {return m_h2d;}

 private:

  std::string m_name;
  std::map<std::string, TH1F*> m_h1d;
  std::map<std::string, TH2F*> m_h2d;


};

#endif
