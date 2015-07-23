#include "FakeTauFilterXaod/FilterValidation.h"


FilterValidation::FilterValidation(const std::string & name)
{
  m_h1d["reco_pt"] = new TH1F(("reco_pt_" + name).c_str(), "reco pt", 16, 20, 100); 
  m_h1d["truth_pt"] = new TH1F(("truth_pt_" + name).c_str(), "reco pt", 16, 20, 100); 

  m_h2d["ntracks"] = new TH2F(("ntracks_" + name).c_str(), "ntracks", 6, 0, 6, 6, 0, 6);

}



void FilterValidation::fill_histograms(const xAOD::TauJet * tau, const TruthFakeTau * truthFakeTau)
{

  m_h1d["reco_pt"]->Fill(tau->pt() / 1000.);
  m_h1d["truth_pt"]->Fill(truthFakeTau->pt() / 1000.);

  m_h2d["ntracks"]->Fill(truthFakeTau->nTracks(), tau->nTracks());

}
