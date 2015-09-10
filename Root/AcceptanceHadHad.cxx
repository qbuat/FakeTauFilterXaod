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



AcceptanceHadHad :: AcceptanceHadHad () : 
m_book("default"),
  m_book_os("os"),
  m_book_nos("nos"),
  m_book_loose("loose")

{
  // Here you put any code for the base initialization of variables,
  // e.g. initialize all pointers to 0.  Note that you should only put
  // the most basic initialization here, since this method will be
  // called on both the submission and the worker node.  Most of your
  // initialization code will go into histInitialize() and
  // initialize().
}



EL::StatusCode AcceptanceHadHad :: setupJob (EL::Job& job)
{
  // Here you put code that sets up the job on the submission object
  // so that it is ready to work with your algorithm, e.g. you can
  // request the D3PDReader service or add output files.  Any code you
  // put here could instead also go into the submission script.  The
  // sole advantage of putting it here is that it gets automatically
  // activated/deactivated when you add/remove the algorithm from your
  // job, which may or may not be of value to you.
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
  m_cutflow->GetXaxis()->SetBinLabel(2, "taus");
  m_cutflow->GetXaxis()->SetBinLabel(3, "taus_pt");
  m_cutflow->GetXaxis()->SetBinLabel(4, "dr_tau_tau");
  m_cutflow->GetXaxis()->SetBinLabel(5, "jets");
  m_cutflow->GetXaxis()->SetBinLabel(6, "jets_pt");
  m_cutflow->GetXaxis()->SetBinLabel(7, "deta_jets");
  m_cutflow->GetXaxis()->SetBinLabel(8, "l1taus");

  m_book.book();
  m_book.record(wk());

  m_book_os.book();
  m_book_os.record(wk());

  m_book_nos.book();
  m_book_nos.record(wk());

  m_book_loose.book();
  m_book_loose.record(wk());
    
  wk()->addOutput(m_cutflow);

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
  // Here you do everything that you need to do after the first input
  // file has been connected and before the first event is processed,
  // e.g. create additional histograms based on which variables are
  // available in the input files.  You can also create all of your
  // histograms and trees in here, but be aware that this method
  // doesn't get called if no events are processed.  So any objects
  // you create here won't be available in the output if you have no
  // input events.
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

  xAOD::TauJetContainer* selected_taus = new xAOD::TauJetContainer();
  xAOD::AuxContainerBase* selected_taus_aux = new xAOD::AuxContainerBase();
  selected_taus->setStore(selected_taus_aux);

  select_taus(selected_taus, taus);

  selected_taus->sort(Utils::compareBDT);

  if (selected_taus->size() < 2)
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("taus", 1);

  xAOD::TauJet* tau1 = selected_taus->at(0);
  xAOD::TauJet* tau2 = selected_taus->at(1);

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

  // ATH_MSG_INFO("DR(tau1, tau2) = " << tau1->p4().DeltaR(tau2->p4()));
  
  xAOD::JetContainer* selected_jets = new xAOD::JetContainer();
  xAOD::AuxContainerBase* selected_jets_aux = new xAOD::AuxContainerBase();
  selected_jets->setStore(selected_jets_aux);
  select_jets(selected_jets, jets, tau1, tau2);

  // ATH_MSG_INFO("Number of jets = " << selected_jets->size());
  if ((int)selected_jets->size() < n_jets)
    return EL::StatusCode::SUCCESS;

  m_cutflow->Fill("jets", 1);


  xAOD::Jet * jet1 = nullptr;
  xAOD::Jet * jet2 = nullptr;

  if (n_jets > 0) {
    jet1  = selected_jets->at(0);

    if (jet1->pt() < jet1_pt)
      return EL::StatusCode::SUCCESS;

    if (n_jets > 1) { 
      jet2  = selected_jets->at(1);

      if (jet2->pt() < jet2_pt)
	return EL::StatusCode::SUCCESS;
      m_cutflow->Fill("jets_pt", 1);

      double delta_eta = fabs(jet1->eta() - jet2->eta());
      if (delta_eta < delta_eta_jj)
	return EL::StatusCode::SUCCESS;
      m_cutflow->Fill("deta_jets", 1);

    } else {
      m_cutflow->Fill("jets_pt", 1);
    }      
  }    

  ATH_MSG_DEBUG("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());
  ATH_MSG_DEBUG("Fill kinematics histograms:");

  m_book.fill(tau1, tau2, 1.);

  if (tau1->charge() * tau1->charge() < 0)
    m_book_os.fill(tau1, tau2, 1.);
  else
    m_book_nos.fill(tau1, tau2, 1.);
      
  if (tau1->isTau(xAOD::TauJetParameters::JetBDTSigLoose) and tau2->isTau(xAOD::TauJetParameters::JetBDTSigLoose))
    m_book_loose.fill(tau1, tau2, 1.);




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
  // This method is the mirror image of initialize(), meaning it gets
  // called after the last event has been processed on the worker node
  // and allows you to finish up any objects you created in
  // initialize() before they are written to disk.  This is actually
  // fairly rare, since this happens separately for each worker node.
  // Most of the time you want to do your post-processing on the
  // submission node after all your histogram outputs have been
  // merged.  This is different from histFinalize() in that it only
  // gets called on worker nodes that processed input events.
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

