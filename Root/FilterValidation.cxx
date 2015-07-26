#include "FakeTauFilterXaod/FilterValidation.h"


FilterValidation::FilterValidation(const std::string & name)
{
  m_h1d["reco_pt"] = new TH1F(("reco_pt_" + name).c_str(), "reco pt", 16, 20, 100); 
  m_h1d["reco_pt_wide"] = new TH1F(("reco_pt_wide_" + name).c_str(), "reco pt", 16, 20, 2000); 
  m_h1d["reco_track_pt"] = new TH1F(("reco_track_pt_" + name).c_str(), "reco track pt", 20, 0, 10); 
  m_h1d["truth_pt"] = new TH1F(("truth_pt_" + name).c_str(), "truth pt", 16, 20, 100); 
  m_h1d["reco_ntracks"] = new TH1F(("reco_ntracks_" + name).c_str(), "reco ntracks", 10, 0, 10);
  m_h1d["truth_ntracks"] = new TH1F(("truth_ntracks_" + name).c_str(), "truth ntracks", 10, 0, 10);
  m_h1d["reco_nwidetracks"] = new TH1F(("reco_nwidetracks_" + name).c_str(), "reco nwidetracks", 10, 0, 10);
  m_h1d["truth_nwidetracks"] = new TH1F(("truth_nwidetracks_" + name).c_str(), "truth nwidetracks", 10, 0, 10);

  m_h1d["reco_bdt_score"] = new TH1F(("reco_bdt_score_" + name).c_str(), "BDT score", 11, -0.1, 1.1);

  m_h2d["ntracks"] = new TH2F(("ntracks_" + name).c_str(), "ntracks", 10, 0, 10, 10, 0, 10);
  m_h2d["nwidetracks"] = new TH2F(("nwidetracks_" + name).c_str(), "nwidetracks", 10, 0, 10, 10, 0, 10);
  m_h2d["pt"] = new TH2F(("pt_" + name).c_str(), "pt", 16, 20, 100, 16, 20, 100);

}



void FilterValidation::fill_histograms(const xAOD::TauJet * tau, const TruthFakeTau * truthFakeTau, const double & weight)
{

  m_h1d["reco_pt"]->Fill(tau->pt() / 1000., weight);
  m_h1d["reco_pt_wide"]->Fill(tau->pt() / 1000., weight);
  m_h1d["truth_pt"]->Fill(truthFakeTau->pt() / 1000., weight);
  m_h1d["reco_ntracks"]->Fill(tau->nTracks(), weight);
  for (unsigned int itr = 0; itr < tau->nTracks(); itr++) {
    m_h1d["reco_track_pt"]->Fill(tau->track(itr)->pt() / 1000., weight);
  }
  m_h1d["truth_ntracks"]->Fill(truthFakeTau->nTracks(), weight);
  m_h1d["reco_nwidetracks"]->Fill(tau->nWideTracks(), weight);
  m_h1d["truth_nwidetracks"]->Fill(truthFakeTau->nWideTracks(), weight);
  m_h1d["reco_bdt_score"]->Fill(tau->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);

  m_h2d["ntracks"]->Fill(truthFakeTau->nTracks(), tau->nTracks(), weight);
  m_h2d["nwidetracks"]->Fill(truthFakeTau->nWideTracks(), tau->nWideTracks(), weight);
  m_h2d["pt"]->Fill(truthFakeTau->pt() / 1000., tau->pt() / 1000., weight);

}
