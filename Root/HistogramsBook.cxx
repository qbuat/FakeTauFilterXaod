#include "FakeTauFilterXaod/HistogramsBook.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

#include "TruthUtils/PIDHelpers.h"

HistogramsBook::HistogramsBook(const std::string & name): m_name(name)
{}


void HistogramsBook::book()
{


  // tau1 
  m_h1d["tau1_pt"]      = new TH1F((m_name + "/h_tau1_pt").c_str(),  "tau1_pt", 20, 0, 100);
  m_h1d["tau1_eta"]     = new TH1F((m_name + "/h_tau1_eta").c_str(), "tau1_eta", 10, -2.5, 2.5);
  m_h1d["tau1_phi"]     = new TH1F((m_name + "/h_tau1_phi").c_str(), "tau1_phi", 10, -3.15, 3.15);
  m_h1d["tau1_ntracks"] = new TH1F((m_name + "/h_tau1_ntracks").c_str(), "tau1_ntracks", 5, 0, 5);
  m_h1d["tau1_bdt"]     = new TH1F((m_name + "/h_tau1_bdt").c_str(), "tau1_bdt", 10, 0, 1);
  m_h1d["tau1_flavor"]  = flavour_hist(m_name + "/h_tau1_flavor");

  m_h1d["tau1_pt_g"]      = new TH1F((m_name + "/h_tau1_pt_g").c_str(),  "tau1_pt", 20, 0, 100);
  m_h1d["tau1_bdt_g"]     = new TH1F((m_name + "/h_tau1_bdt_g").c_str(), "tau1_bdt", 10, 0, 1);
  m_h1d["tau1_pt_q"]      = new TH1F((m_name + "/h_tau1_pt_q").c_str(),  "tau1_pt", 20, 0, 100);
  m_h1d["tau1_bdt_q"]     = new TH1F((m_name + "/h_tau1_bdt_q").c_str(), "tau1_bdt", 10, 0, 1);


  // tau 2
  m_h1d["tau2_pt"]      = new TH1F((m_name + "/h_tau2_pt").c_str(), "tau2_pt", 20, 0, 100);
  m_h1d["tau2_eta"]     = new TH1F((m_name + "/h_tau2_eta").c_str(), "tau2_eta", 10, -2.5, 2.5);
  m_h1d["tau2_phi"]     = new TH1F((m_name + "/h_tau2_phi").c_str(), "tau2_phi", 10, -3.15, 3.15);
  m_h1d["tau2_ntracks"] = new TH1F((m_name + "/h_tau2_ntracks").c_str(), "tau2_ntracks", 5, 0, 5);
  m_h1d["tau2_bdt"]     = new TH1F((m_name + "/h_tau2_bdt").c_str(), "tau2_bdt", 10, 0, 1);
  m_h1d["tau2_flavor"]  = flavour_hist(m_name + "/h_tau2_flavor");

  m_h1d["tau2_pt_g"]      = new TH1F((m_name + "/h_tau2_pt_g").c_str(),  "tau2_pt", 20, 0, 100);
  m_h1d["tau2_bdt_g"]     = new TH1F((m_name + "/h_tau2_bdt_g").c_str(), "tau2_bdt", 10, 0, 1);
  m_h1d["tau2_pt_q"]      = new TH1F((m_name + "/h_tau2_pt_q").c_str(),  "tau2_pt", 20, 0, 100);
  m_h1d["tau2_bdt_q"]     = new TH1F((m_name + "/h_tau2_bdt_q").c_str(), "tau2_bdt", 10, 0, 1);

  // tau-tau system
  m_h1d["tautau_dr"]    = new TH1F((m_name + "/h_tautau_dr").c_str(), "dr_tau_tau", 16, 0, 3.2); 

  // maps
  m_h2d["bdt_score"] = new TH2F((m_name + "/map_bdt_score").c_str(), "bdt_score", 10, 0, 1, 10, 0, 1);


  // resolution histograms
  m_h2d["resol_pt"] = new TH2F((m_name + "/resol_pt").c_str(), "pt", 20, 0, 100, 20, 0, 100);
  m_h2d["resol_eta"] = new TH2F((m_name + "/resol_eta").c_str(), "eta", 15, -3, 3, 15, -3, 3);
  m_h2d["resol_ntracks"] = new TH2F((m_name + "/resol_ntracks").c_str(), "ntracks", 10, 0, 10, 10, 0, 10);
  m_h2d["resol_nwidetracks"] = new TH2F((m_name + "/resol_nwidetracks").c_str(), "nwidetracks", 10, 0, 10, 10, 0, 10);


}

TH1F* HistogramsBook::flavour_hist(const std::string & name)
{
  TH1F* tmp = new TH1F(name.c_str(), name.c_str(), 4, 0, 4);
  tmp->GetXaxis()->SetBinLabel(1, "gluon");
  tmp->GetXaxis()->SetBinLabel(2, "quark");
  tmp->GetXaxis()->SetBinLabel(3, "light");
  tmp->GetXaxis()->SetBinLabel(4, "heavy");
  return tmp;
}

void HistogramsBook::fill_flavour_hist(TH1F* hist, const xAOD::TauJet * tau, const double & weight)
{
  int pid = tau->jet()->getAttribute<int>("PartonTruthLabelID");

  if (pid == MC::PID::GLUON)
    hist->Fill("gluon", weight);
  else if (MC::PID::isQuark(pid)) {

    hist->Fill("quark", weight);

    if (MC::PID::isLightParton(pid))
      hist->Fill("light", weight);

    if (MC::PID::isHeavyParton(pid))
      hist->Fill("heavy", weight);
  }
}


void HistogramsBook::fill(const xAOD::TauJet * tau1, const xAOD::TauJet * tau2, const double & weight)
{

  m_h1d["tau1_pt"]->Fill(tau1->pt() / 1000., weight);
  m_h1d["tau1_eta"]->Fill(tau1->eta(), weight);
  m_h1d["tau1_phi"]->Fill(tau1->phi(), weight);
  m_h1d["tau1_ntracks"]->Fill(tau1->nTracks(), weight);
  m_h1d["tau1_bdt"]->Fill(tau1->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  fill_flavour_hist(m_h1d["tau1_flavor"], tau1, weight);

  m_h1d["tau2_pt"]->Fill(tau2->pt() / 1000., weight);
  m_h1d["tau2_eta"]->Fill(tau2->eta(), weight);
  m_h1d["tau2_phi"]->Fill(tau2->phi(), weight);
  m_h1d["tau2_ntracks"]->Fill(tau2->nTracks(), weight);
  m_h1d["tau2_bdt"]->Fill(tau2->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  fill_flavour_hist(m_h1d["tau2_flavor"], tau2, weight);

  m_h1d["tautau_dr"]->Fill(tau1->p4().DeltaR(tau2->p4()), weight);

  if (tau1->jet()->getAttribute<int>("PartonTruthLabelID") == MC::PID::GLUON) { 
    m_h1d["tau1_pt_g"]->Fill(tau1->pt() / 1000., weight);
    m_h1d["tau1_bdt_g"]->Fill(tau1->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  } else if (MC::PID::isQuark(tau1->jet()->getAttribute<int>("PartonTruthLabelID"))) {
    m_h1d["tau1_pt_q"]->Fill(tau1->pt() / 1000., weight);
    m_h1d["tau1_bdt_q"]->Fill(tau1->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  }

  if (tau2->jet()->getAttribute<int>("PartonTruthLabelID") == MC::PID::GLUON) { 
    m_h1d["tau2_pt_g"]->Fill(tau2->pt() / 1000., weight);
    m_h1d["tau2_bdt_g"]->Fill(tau2->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  } else if (MC::PID::isQuark(tau2->jet()->getAttribute<int>("PartonTruthLabelID"))) {
    m_h1d["tau2_pt_q"]->Fill(tau2->pt() / 1000., weight);
    m_h1d["tau2_bdt_q"]->Fill(tau2->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore), weight);
  }


  m_h2d["bdt_score"]->Fill(tau1->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore),
			   tau2->discriminant(xAOD::TauJetParameters::TauID::BDTJetScore));
}

void HistogramsBook::fill_resol(const xAOD::TauJet * tau, const TruthFakeTau * truthFakeTau, const double & weight)
{
  if (tau != NULL and truthFakeTau != NULL) {
    m_h2d["resol_pt"]->Fill(truthFakeTau->pt() / 1000., tau->pt() / 1000., weight);
    m_h2d["resol_eta"]->Fill(truthFakeTau->eta(), tau->eta(), weight);
    m_h2d["resol_ntracks"]->Fill(truthFakeTau->nTracks(), tau->nTracks(), weight);
    m_h2d["resol_nwidetracks"]->Fill(truthFakeTau->nWideTracks(), tau->nTracks(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation), weight);
  }

}


void HistogramsBook::record(EL::Worker* wk)
{
  for (auto h: m_h1d) {
    wk->addOutput(h.second);
  }

  for (auto h: m_h2d){
    wk->addOutput(h.second);
  }

}
