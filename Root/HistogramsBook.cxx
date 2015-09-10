#include "FakeTauFilterXaod/HistogramsBook.h"

#include "TruthUtils/PIDCodes.h"
#include "TruthUtils/PIDUtils.h"

HistogramsBook::HistogramsBook(const std::string & name): m_name(name)
{}


void HistogramsBook::book()
{
  // tau1 
  m_h1d["tau1_pt"]      = new TH1F((m_name + "/h_tau1_pt").c_str(),  "tau1_pt", 40, 20, 100);
  m_h1d["tau1_eta"]     = new TH1F((m_name + "/h_tau1_eta").c_str(), "tau1_eta", 10, -2.5, 2.5);
  m_h1d["tau1_phi"]     = new TH1F((m_name + "/h_tau1_phi").c_str(), "tau1_phi", 10, -3.15, 3.15);
  m_h1d["tau1_ntracks"] = new TH1F((m_name + "/h_tau1_ntracks").c_str(), "tau1_ntracks", 5, 0, 5);
  m_h1d["tau1_bdt"]     = new TH1F((m_name + "/h_tau1_bdt").c_str(), "tau1_bdt", 10, 0, 1);
  m_h1d["tau1_flavor"]  = flavour_hist(m_name + "/h_tau1_flavor");

  // tau 2
  m_h1d["tau2_pt"]      = new TH1F((m_name + "/h_tau2_pt").c_str(), "tau2_pt", 40, 20, 100);
  m_h1d["tau2_eta"]     = new TH1F((m_name + "/h_tau2_eta").c_str(), "tau2_eta", 10, -2.5, 1.5);
  m_h1d["tau2_phi"]     = new TH1F((m_name + "/h_tau2_phi").c_str(), "tau2_phi", 10, -3.15, 3.15);
  m_h1d["tau2_ntracks"] = new TH1F((m_name + "/h_tau2_ntracks").c_str(), "tau2_ntracks", 5, 0, 5);
  m_h1d["tau2_bdt"]     = new TH1F((m_name + "/h_tau2_bdt").c_str(), "tau2_bdt", 10, 0, 1);
  m_h1d["tau2_flavor"]  = flavour_hist(m_name + "/h_tau2_flavor");

  // tau-tau system
  m_h1d["tautau_dr"]    = new TH1F((m_name + "/h_tautau_dr").c_str(), "dr_tau_tau", 15, 0, 3); 

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

}


void HistogramsBook::record(EL::Worker* wk)
{
  for (auto h: m_h1d) {
    std::cout << h.second->GetName() << std::endl;
    wk->addOutput(h.second);
  }
}
