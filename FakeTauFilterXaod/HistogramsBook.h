#ifndef FAKETAUFILTERXAOD_HISTOGRAMSBOOK_H
#define FAKETAUFILTERXAOD_HISTOGRAMSBOOK_H

#include <map>

#include "TH1F.h"

#include "xAODTau/TauJet.h"
#include "FakeTauFilterXaod/FakeTauFilterXaod.h"
#include "EventLoop/Worker.h"

class HistogramsBook

{
 public: 

  HistogramsBook (const std::string & name);
  virtual ~ HistogramsBook() {};

  void book();
  void fill(const xAOD::TauJet * tau1, const xAOD::TauJet * tau2, const double & weight=1.0);
  void record(EL::Worker* wk);

 private:

  TH1F* flavour_hist(const std::string & name);
  void fill_flavour_hist(TH1F* hist, const xAOD::TauJet* tau, const double & weight);
  std::string m_name;
  std::map<std::string, TH1F*> m_h1d;


};

#endif
