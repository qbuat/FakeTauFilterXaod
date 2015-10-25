#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <FakeTauFilterXaod/AcceptanceHadHad.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"
#include "TruthUtils/PIDUtils.h"

// EDM includes
#include "xAODEventInfo/EventInfo.h"
#include "xAODCore/AuxContainerBase.h"

#include "FakeTauFilterXaod/Utils.h"

/// Helper macro for checking xAOD::TReturnCode return values
#define EL_RETURN_CHECK( CONTEXT, EXP )                     \
  do {                                                     \
  if( ! EXP.isSuccess() ) {                             \
  Error( CONTEXT,                                    \
    XAOD_MESSAGE( "Failed to execute: %s" ),    \
	 #EXP );                                     \
  return EL::StatusCode::FAILURE;                    \
  }                                                     \
  } while( false )

// this is needed to distribute the algorithm to the workers
ClassImp(AcceptanceHadHad)



AcceptanceHadHad :: AcceptanceHadHad () : m_book("default"),
  m_book_os("os"),
  m_book_loose("loose")

{

}



EL::StatusCode AcceptanceHadHad :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: histInitialize ()
{
  // Here you do everything that needs to be done at the very
  // beginning on each worker node, e.g. create histograms and output
  // trees.  This method gets called before any input files are
  // connected.
  m_cutflow = new TH1F("cutflow", "cutflow", 10, 0, 10);
  m_cutflow->GetXaxis()->SetBinLabel(1, "init");
  m_cutflow->GetXaxis()->SetBinLabel(2, "truth_matching");
  m_cutflow->GetXaxis()->SetBinLabel(3, "taus");
  m_cutflow->GetXaxis()->SetBinLabel(4, "taus_pt");
  m_cutflow->GetXaxis()->SetBinLabel(5, "dr_tau_tau");
  m_cutflow->GetXaxis()->SetBinLabel(6, "taus_tracks");
  m_cutflow->GetXaxis()->SetBinLabel(7, "os");
  m_cutflow->GetXaxis()->SetBinLabel(8, "loose");
  m_cutflow->GetXaxis()->SetBinLabel(9, "medium");

  m_truthpairs = new TH1F("ntruthpairs", "ntruthpairs", 10, 0, 10);

  m_book.book();
  m_book.record(wk());

  m_book_os.book();
  m_book_os.record(wk());

  m_book_loose.book();
  m_book_loose.record(wk());
    
  wk()->addOutput(m_cutflow);
  wk()->addOutput(m_truthpairs);

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: initialize ()
{

  if (asg::ToolStore::contains<FakeTauFilterXaod>("FakeTauFilter"))
    m_filter = asg::ToolStore::get<FakeTauFilterXaod>("FakeTauFilter");
  else {
    m_filter = new FakeTauFilterXaod("FakeTauFilter");
    EL_RETURN_CHECK("initialize", m_filter->initialize());
  }
    

  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_INFO("Number of events = " << event->getEntries());
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: execute ()
{
  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_DEBUG("execute next event");
  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());


  m_cutflow->Fill("init", 1);

  // retrieve the EDM objects
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(taus, "TauJets"));
  
  const xAOD::JetContainer* jets = 0;
  EL_RETURN_CHECK("execute", event->retrieve(jets, "AntiKt4LCTopoJets"));

  const xAOD::TruthParticleContainer *truthParticles = 0;
  EL_RETURN_CHECK("execute", event->retrieve(truthParticles, "TruthParticles"));

  EL_RETURN_CHECK("execute", m_filter->execute(truthParticles));
  auto filtered_pairs = m_filter->GetDiTruthFakeTaus();


  if (filtered_pairs.size() == 0)
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("truth_matching", 1);
  m_truthpairs->Fill(filtered_pairs.size());

  xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
  xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
  selected_taus->setStore(selected_taus_aux);

  select_taus_fromtruth(selected_taus, taus, filtered_pairs);
  ATH_MSG_DEBUG("number of selected taus = "<< selected_taus->size());

  if (selected_taus->size() < 2) {
    ATH_MSG_INFO("SCAN THE TRUE PAIRS");
    for (auto truth_pair: filtered_pairs) {
      ATH_MSG_INFO("first: pt = "<< (truth_pair.first)->pt()
		   << ", eta = " << (truth_pair.first)->eta()
		   << ", phi = " << (truth_pair.first)->phi());
      ATH_MSG_INFO("second: pt = "<< (truth_pair.second)->pt()
		   << ", eta = "  << (truth_pair.second)->eta()
		   << ", phi = "  << (truth_pair.second)->phi());
    }
    ATH_MSG_DEBUG("number of truth-level pairs = " << filtered_pairs.size());
    return EL::StatusCode::SUCCESS;
  }

  m_cutflow->Fill("taus", 1);

  xAOD::TauJet* tau1 = selected_taus->at(0);
  xAOD::TauJet* tau2 = selected_taus->at(1);

  m_book.fill(tau1, tau2, 1.);
  for (const auto tau: *selected_taus) {
    auto * truthfake = m_filter->matchedFake(tau);
    m_book.fill_resol(tau, truthfake);
  }

  // Leading tau pt cut
  if (tau1->pt() < tau1_pt)
    return EL::StatusCode::SUCCESS;
  
  // Subleading tau pt cut
  if (tau2->pt() < tau2_pt)
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("taus_pt", 1);

  // DR(TAU, TAU) cut
  if (tau1->p4().DeltaR(tau2->p4()) < min_dr_tautau)
    return EL::StatusCode::SUCCESS;

  if (tau1->p4().DeltaR(tau2->p4()) > max_dr_tautau)
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("dr_tau_tau", 1);
  
  // ntracks cut
  if (tau1->nTracks() != 1 and tau1->nTracks() != 3)
    return EL::StatusCode::SUCCESS;                                                                                                   

  if (tau2->nTracks() != 1 and tau2->nTracks() != 3)
    return EL::StatusCode::SUCCESS;                                                                                                   
  
  m_cutflow->Fill("taus_tracks", 1);

  // os cut
  if (tau1->charge() * tau2->charge() < 0)
    return EL::StatusCode::SUCCESS;
  
  m_cutflow->Fill("os", 1);
  m_book_os.fill(tau1, tau2, 1.);
  

  // loose-loose
  if (not tau1->isTau(xAOD::TauJetParameters::JetBDTSigLoose))
    return EL::StatusCode::SUCCESS;

  if (not tau2->isTau(xAOD::TauJetParameters::JetBDTSigLoose))
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("loose", 1);
  m_book_loose.fill(tau1, tau2, 1.);

  // medium-medium
  if (not tau1->isTau(xAOD::TauJetParameters::JetBDTSigMedium))
    return EL::StatusCode::SUCCESS;

  if (not tau2->isTau(xAOD::TauJetParameters::JetBDTSigMedium))
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("medium", 1);



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: finalize ()
{
  if (m_filter) {
    m_filter = NULL;
    delete m_filter;
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode AcceptanceHadHad :: histFinalize ()
{
  // This method is the mirror image of histInitialize(), meaning it
  // gets called after the last event has been processed on the worker
  // node and allows you to finish up any objects you created in
  // histInitialize() before they are written to disk.  This is
  // actually fairly rare, since this happens separately for each
  // worker node.  Most of the time you want to do your
  // post-processing on the submission node after all your histogram
  // outputs have been merged.  This is different from finalize() in
  // that it gets called on all worker nodes regardless of whether
  // they processed input events.
  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AcceptanceHadHad :: select_taus(xAOD::TauJetContainer *selected_taus, const xAOD::TauJetContainer * taus)


{

  for (const auto tau: *taus) {

    // pt cut
    if (tau->pt() < 20000.) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with pt = " << tau->pt());
      continue;
    }

    // eta cut
    if (fabs(tau->eta()) > 2.5) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with eta = " << tau->eta());
      continue;
    }

    // if (fabs(tau->eta()) > 1.37 and fabs(tau->eta()) < 1.52) {
    //   ATH_MSG_DEBUG("Reject tau " << tau->index() << " with eta = " << tau->eta());
    //   continue;
    // }
      
    // 1 or 3 tracks
    if (tau->nTracks() != 1 and tau->nTracks() != 3) {
      ATH_MSG_DEBUG("Reject tau " << tau->index() << " with nTracks = " << tau->nTracks());
      continue;
    }
    // // ID cut
    // if (not tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose)) {
    //   ATH_MSG_DEBUG("Reject tau " << tau->index() << " with medium ID = " << tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose));
    //   continue;
    // }      

    // int label = tau->jet()->getAttribute<int>("PartonTruthLabelID");
    // ATH_MSG_INFO(Form("tau %d: parton = %d, isParton = %d, isQuark = %d", (int)tau->index(), label, (int)MC::PID::isParton(label), (int)MC::PID::isQuark(label)));
      // selectDec(*tau) = true;
    xAOD::TauJet * new_tau = new xAOD::TauJet();
    new_tau->makePrivateStore(*tau);
    selected_taus->push_back(new_tau);
  }

  // sort by pt
  selected_taus->sort(Utils::comparePt);

  return EL::StatusCode::SUCCESS;

}

EL::StatusCode AcceptanceHadHad :: select_jets(xAOD::JetContainer *selected_jets, 
					       const xAOD::JetContainer *jets, 
					       const xAOD::TauJet *tau1, 
					       const xAOD::TauJet *tau2)
{
  for (const auto jet: *jets) {
      
    // pt cut
    if (jet->pt() < 30000.) 
      continue;
    
    // eta cut
    if (fabs(jet->eta()) > jet_eta)
      continue;

    // ORL with first tau
    if (jet->p4().DeltaR(tau1->p4()) < 0.4)
      continue;

    // ORL with second tau
    if (jet->p4().DeltaR(tau2->p4()) < 0.4)
      continue;

    xAOD::Jet* new_jet = new xAOD::Jet();
    new_jet->makePrivateStore(*jet);
    selected_jets->push_back(new_jet);
  }
  // sort them by pT
  selected_jets->sort(Utils::comparePt);

  return EL::StatusCode::SUCCESS;
}

EL::StatusCode AcceptanceHadHad :: select_taus_fromtruth(xAOD::TauJetContainer *selected_taus, 
							 const xAOD::TauJetContainer *taus,
							 const DiTruthFakeTaus & truth_pairs)
{


  if (truth_pairs.size() < 1) 
    return EL::StatusCode::SUCCESS;

  // auto truth_pair = truth_pairs[0];
  for (auto truth_pair: truth_pairs) {
    // selected pair
    selected_taus->clear();
    for (const auto tau: *taus) {
      if (m_filter->DeltaR(tau, *(truth_pair.first)) < 0.5 or 
	  m_filter->DeltaR(tau, *(truth_pair.second)) < 0.5) {
	xAOD::TauJet * new_tau = new xAOD::TauJet();
	new_tau->makePrivateStore(*tau);
	selected_taus->push_back(new_tau);
      }
    }
    if (selected_taus->size() > 1)
      break;
  }
  // sort by pt
  selected_taus->sort(Utils::comparePt);

  return EL::StatusCode::SUCCESS;
}
