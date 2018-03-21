#include <EventLoop/Job.h>
#include <EventLoop/StatusCode.h>
#include <EventLoop/Worker.h>
#include <FakeTauFilterXaod/FilterValidator.h>

#include "xAODRootAccess/Init.h"
#include "xAODRootAccess/TEvent.h"
#include "xAODRootAccess/tools/ReturnCheck.h"
#include "xAODRootAccess/tools/Message.h"

#include "AsgTools/MsgStream.h"
#include "AsgTools/MsgStreamMacros.h"

// EDM
#include "xAODEventInfo/EventInfo.h"
#include "xAODTau/TauJetContainer.h"

// this is needed to distribute the algorithm to the workers
//ClassImp(FilterValidator)

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


FilterValidator :: FilterValidator ()
{
  m_book["nosel"] = new FilterValidation("nosel");
  m_book["presel"] = new FilterValidation("presel");
  m_book["core_tracks"] = new FilterValidation("core_tracks");
  m_book["isol_tracks"] = new FilterValidation("isol_tracks");
  m_book["bdt"] = new FilterValidation("bdt");
  m_book["cr"] = new FilterValidation("cr");

  m_book["nosel_passfilter"] = new FilterValidation("nosel_passfilter");
  m_book["presel_passfilter"] = new FilterValidation("presel_passfilter");
  m_book["core_tracks_passfilter"] = new FilterValidation("core_tracks_passfilter");
  m_book["isol_tracks_passfilter"] = new FilterValidation("isol_tracks_passfilter");
  m_book["bdt_passfilter"] = new FilterValidation("bdt_passfilter");
  m_book["cr_passfilter"] = new FilterValidation("cr_passfilter");

}



EL::StatusCode FilterValidator :: setupJob (EL::Job& job)
{
  job.useXAOD ();
  EL_RETURN_CHECK("setupJob ()", xAOD::Init());

  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: histInitialize ()
{

  he = new TH1F("events", "events", 1, 0.5, 1.5);
  hw = new TH1F("weighted_events", "weighted_event", 1, 0.5, 1.5);
  hnpassfilter = new TH1F("npassfilter", "npassfilter", 8, -0.5, 7.5);
  hnpairpassfilter = new TH1F("npairpassfilter", "npairpassfilter", 8, -0.5, 7.5);
  htaus = new TH1F("matched_taus", "matched_taus", 1, 0.5, 1.5);

  wk()->addOutput(he);
  wk()->addOutput(hw);
  wk()->addOutput(hnpassfilter);
  wk()->addOutput(hnpairpassfilter);
  wk()->addOutput(htaus);

  for (auto item: m_book) {
    item.second->book();
    item.second->record(wk());
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: fileExecute ()
{
  // Here you do everything that needs to be done exactly once for every
  // single file, e.g. collect a list of all lumi-blocks processed
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: changeInput (bool /*firstFile*/)
{
  // Here you do everything you need to do when we change input files,
  // e.g. resetting branch addresses on trees.  If you are using
  // D3PDReader or a similar service this method is not needed.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: initialize ()
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



EL::StatusCode FilterValidator :: execute ()
{

  xAOD::TEvent* event = wk()->xaodEvent();
  ATH_MSG_DEBUG("execute next event");
  if ((wk()->treeEntry() % 200) == 0)
    ATH_MSG_INFO("Read event number "<< wk()->treeEntry() << " / " << event->getEntries());



  // retrieve the EDM objects
  ATH_MSG_DEBUG("retrieve event info");
  const xAOD::EventInfo * ei = 0;
  EL_RETURN_CHECK("execute", event->retrieve(ei, "EventInfo"));

  ATH_MSG_DEBUG("retrieve taus");
  const xAOD::TauJetContainer* taus = 0;
  EL_RETURN_CHECK("execute", event->retrieve(taus, "TauJets"));
  
  ATH_MSG_DEBUG("retrieve truth particles");
  const xAOD::TruthParticleContainer *truthParticles = 0;
  EL_RETURN_CHECK("execute", event->retrieve(truthParticles, "TruthParticles"));

  double weight = 1.0;
  bool isMC = false;
  if (ei->eventType(xAOD::EventInfo::IS_SIMULATION)) 
    isMC = true;
  
  if (isMC) { 
    const std::vector<float> weights = ei->mcEventWeights();
    if (weights.size() > 0) 
      weight = weights[0];
  }

  he->Fill(1.0);
  hw->Fill(1.0, weight);
  
  // Excuting the Filter
  EL_RETURN_CHECK("execute", m_filter->execute(truthParticles));

  auto filtered_part = m_filter->GetTruthFakeTaus();
  hnpassfilter->Fill(filtered_part.size());
  
  auto filtered_pairs = m_filter->GetDiTruthFakeTaus();
  hnpairpassfilter->Fill(filtered_pairs.size());

  // Looping over all offline taus
  for (const auto tau: *taus) {
    ATH_MSG_DEBUG("0: reading tau " << tau->index());
    auto * truthfake = m_filter->matchedFake(tau);
    ATH_MSG_DEBUG("0.1: reading tau " << tau->index());

    htaus->Fill((int)(truthfake != NULL), weight);
    ATH_MSG_DEBUG("1: reading tau " << tau->index());
      
    if (truthfake == NULL)
      continue;

    m_book["nosel"]->fill_histograms(tau, truthfake, weight);
    ATH_MSG_DEBUG("2: reading tau " << tau->index());

    if (truthfake->is_good())
      m_book["nosel_passfilter"]->fill_histograms(tau, truthfake, weight);

    ATH_MSG_DEBUG("3: reading tau " << tau->index());

    if (tau->pt() < 30000.) 
      continue;
      
    if (fabs(tau->eta() > 2.5) )
      continue;

    m_book["presel"]->fill_histograms(tau, truthfake, weight);

    if (truthfake->is_good())
      m_book["presel_passfilter"]->fill_histograms(tau, truthfake, weight);
    
    if (tau->nTracks() != 1 and tau->nTracks() != 3)
      continue;

    m_book["core_tracks"]->fill_histograms(tau, truthfake, weight);

    if (truthfake->is_good())
      m_book["core_tracks_passfilter"]->fill_histograms(tau, truthfake, weight);

    if (tau->nTracks(xAOD::TauJetParameters::TauTrackFlag::classifiedIsolation) != 0)
      continue;
      
    m_book["isol_tracks"]->fill_histograms(tau, truthfake, weight);
    if (truthfake->is_good()) {
      m_book["isol_tracks_passfilter"]->fill_histograms(tau, truthfake, weight);
    }

    bool is_loose_off = tau->isTau(xAOD::TauJetParameters::JetBDTSigLoose);
    bool is_medium_off = tau->isTau(xAOD::TauJetParameters::JetBDTSigMedium);

    if (is_loose_off and not is_medium_off) {
      m_book["cr"]->fill_histograms(tau, truthfake, weight);
      if (truthfake->is_good())
	m_book["cr_passfilter"]->fill_histograms(tau, truthfake, weight);
    }
    
    if (not is_medium_off)
      continue;

    m_book["bdt"]->fill_histograms(tau, truthfake, weight);

    if (truthfake->is_good())
      m_book["bdt_passfilter"]->fill_histograms(tau, truthfake, weight);
  }
  // End of the loop over all offline taus



  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: postExecute ()
{
  // Here you do everything that needs to be done after the main event
  // processing.  This is typically very rare, particularly in user
  // code.  It is mainly used in implementing the NTupleSvc.
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: finalize ()
{
  if (m_filter) {
    m_filter = NULL;
    delete m_filter;
  }
  return EL::StatusCode::SUCCESS;
}



EL::StatusCode FilterValidator :: histFinalize ()
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
