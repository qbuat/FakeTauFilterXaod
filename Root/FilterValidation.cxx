#include "FakeTauFilterXaod/FilterValidation.h"


FilterValidation::FilterValidation(const std::string & name) : m_name(name)
{}

void FilterValidation::book()
{

  m_h1d["counts"] = new TH1F((m_name + "/counts").c_str(), "counts", 1, 0.5, 1.5);
  m_h1d["weighted_counts"] = new TH1F((m_name + "/weighted_counts").c_str(), "weighted_counts", 1, 0.5, 1.5);
  m_h1d["reco_pt"] = new TH1F((m_name + "/reco_pt").c_str(), "reco pt", 16, 20, 100); 
  m_h1d["reco_pt_wide"] = new TH1F((m_name + "/reco_pt_wide").c_str(), "reco pt", 16, 20, 2000); 
  m_h1d["reco_track_pt"] = new TH1F((m_name + "/reco_track_pt").c_str(), "reco track pt", 20, 0, 10); 
  m_h1d["truth_pt"] = new TH1F((m_name + "/truth_pt").c_str(), "truth pt", 16, 20, 100); 
  m_h1d["reco_ntracks"] = new TH1F((m_name + "/reco_ntracks").c_str(), "reco ntracks", 10, 0, 10);
  m_h1d["truth_ntracks"] = new TH1F((m_name + "/truth_ntracks").c_str(), "truth ntracks", 10, 0, 10);
  m_h1d["reco_nwidetracks"] = new TH1F((m_name + "/reco_nwidetracks").c_str(), "reco nwidetracks", 10, 0, 10);
  m_h1d["truth_nwidetracks"] = new TH1F((m_name + "/truth_nwidetracks").c_str(), "truth nwidetracks", 10, 0, 10);

  m_h1d["reco_bdt_score"] = new TH1F((m_name + "/reco_bdt_score").c_str(), "BDT score", 11, -0.1, 1.1);

  m_h1d["reco_truth_pt"]  = new TH1F((m_name + "/reco_truth_pt").c_str(), "", 20, -20, 20);
  m_h2d["ntracks"] = new TH2F((m_name + "/ntracks").c_str(), "ntracks", 10, 0, 10, 10, 0, 10);
  m_h2d["nwidetracks"] = new TH2F((m_name + "/nwidetracks").c_str(), "nwidetracks", 10, 0, 10, 10, 0, 10);
  m_h2d["pt"] = new TH2F((m_name + "/pt").c_str(), "pt", 16, 20, 100, 16, 20, 100);
  m_h2d["reso_truth_pt"] = new TH2F((m_name + "/reso_truth_pt").c_str(), "reso vs truth pt", 16, 20, 100, 40, -20, 20);
  m_h2d["reso_reco_pt"] = new TH2F((m_name + "/reso_reco_pt").c_str(), "reso vs reco pt", 16, 20, 100, 40, -20, 20);

}

void FilterValidation::record(EL::Worker* wk)
{
  for (auto h: m_h1d) {
    wk->addOutput(h.second);
  }

  for (auto h: m_h2d){
    wk->addOutput(h.second);
  }

}


void FilterValidation::fill_histograms(const xAOD::TauJet * tau, const TruthFakeTau * truthFakeTau, const double & weight)
{

  m_h1d["counts"]->Fill(1.);
  m_h1d["weighted_counts"]->Fill(1., weight);
  m_h1d["reco_pt"]->Fill(tau->pt() / 1000., weight);
  m_h1d["reco_pt_wide"]->Fill(tau->pt() / 1000., weight);
  m_h1d["reco_nwidetracks"]->Fill(tau->nWideTracks(), weight);
  m_h1d["reco_bdt_score"]->Fill(tau->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  m_h1d["reco_ntracks"]->Fill(tau->nTracks(), weight);

  for (unsigned int itr = 0; itr < tau->nTracks(); itr++) {
    m_h1d["reco_track_pt"]->Fill(tau->track(itr)->pt() / 1000., weight);
  }


  if (truthFakeTau != NULL) {
    m_h1d["truth_pt"]->Fill(truthFakeTau->pt() / 1000., weight);
    m_h1d["truth_ntracks"]->Fill(truthFakeTau->nTracks(), weight);
    m_h1d["truth_nwidetracks"]->Fill(truthFakeTau->nWideTracks(), weight);
    m_h1d["reco_truth_pt"]->Fill((tau->pt() - truthFakeTau->pt()) / 1000.);
    m_h2d["ntracks"]->Fill(truthFakeTau->nTracks(), tau->nTracks(), weight);
    m_h2d["nwidetracks"]->Fill(truthFakeTau->nWideTracks(), tau->nWideTracks(), weight);
    m_h2d["pt"]->Fill(truthFakeTau->pt() / 1000., tau->pt() / 1000., weight);
    m_h2d["reso_truth_pt"]->Fill(truthFakeTau->pt() / 1000., (tau->pt() - truthFakeTau->pt()) / 1000.);
    m_h2d["reso_reco_pt"]->Fill(tau->pt() / 1000., (tau->pt() - truthFakeTau->pt()) / 1000.);
  }
}
