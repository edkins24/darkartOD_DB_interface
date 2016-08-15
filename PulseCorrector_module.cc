////////////////////////////////////////////////////////////////////////
// Class:       PulseCorrector
// Module Type: producer
// File:        PulseCorrector_module.cc
//
// Generated at Tue Feb  3 05:45:02 2015 by Shawn Westerdale using artmod
// from cetpkgsupport v1_07_00.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>
#include <iostream>

#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

#include "darkart/ODProducts/RunInfo.hh"
#include "darkart/ODReco/Tools/Utilities.hh"

//Submodules
#include "darkart/ODReco/Submodules/BaselineCorrector.hh"
#include "darkart/ODReco/Submodules/SaturationCorrector.hh"
#include "darkart/ODReco/Submodules/SPECorrector.hh"
#include "darkart/ODReco/Submodules/CoerceCorrector.hh"

namespace darkart {
  namespace od {
    class PulseCorrector;
  }
}

class darkart::od::PulseCorrector : public art::EDProducer {
public:
  // Default constructors
  explicit PulseCorrector(fhicl::ParameterSet const & p);
  PulseCorrector(PulseCorrector const &) = delete;
  PulseCorrector(PulseCorrector &&) = delete;
  PulseCorrector & operator = (PulseCorrector const &) = delete;
  PulseCorrector & operator = (PulseCorrector &&) = delete;

  // Required functions.
  void produce(art::Event & e) override;

  // Selected optional functions.
  void beginRun(art::Run & r) override;

  // Mode functions
  void modeDefault(darkart::od::ChannelData::Pulse&, darkart::od::ChannelWFs::PulseWF&, int);
  void modeBaselineCorrector(darkart::od::ChannelData::Pulse&, darkart::od::ChannelWFs::PulseWF&, int);
  void modeSaturationCorrector(darkart::od::ChannelData::Pulse&, darkart::od::ChannelWFs::PulseWF&, int);
  void modeSPECorrector(darkart::od::ChannelData::Pulse&, darkart::od::ChannelWFs::PulseWF&, int);
  void modeCoerceCorrector(darkart::od::ChannelData::Pulse&, darkart::od::ChannelWFs::PulseWF&, int);

private:
  std::string _mode;
  bool _mode_default;
  std::string _input_pulses;
  int _verbosity;
  std::vector<darkart::od::RunInfo::ChConfData> _ch_conf_data_vec;

  // Initialize submodules that may be used
  SaturationCorrector  *_saturation_corrector;
  BaselineCorrector    *_baseline_corrector;
  SPECorrector         *_spe_corrector;
  CoerceCorrector      *_coerce_corrector;

  // Other useful things
  art::ServiceHandle<darkart::od::Utilities> _utils;

};


darkart::od::PulseCorrector::PulseCorrector(fhicl::ParameterSet const & p)
  : _mode(p.get<std::string>("mode","default"))
  , _mode_default(false)
  , _input_pulses(p.get<std::string>("input_pulses","ODReader"))
  , _verbosity(p.get<int>("verbosity",0))
  , _saturation_corrector(0)
  , _baseline_corrector(0)
  , _spe_corrector(0)
  , _coerce_corrector(0)
{
  if(_mode == "default"){
    _mode_default = true;    

    _baseline_corrector = new BaselineCorrector(p.get<fhicl::ParameterSet>("baselineparams"));
    _baseline_corrector->setVerbosity(_verbosity);
    _baseline_corrector->setUtilities(_utils->getPointer());

    _coerce_corrector = new CoerceCorrector(p.get<fhicl::ParameterSet>("coerceparams"), _verbosity);
    _coerce_corrector->setUtilities(_utils->getPointer());

    _saturation_corrector = new SaturationCorrector(p.get<fhicl::ParameterSet>("saturationparams"));
    _saturation_corrector->setUtilities(_utils->getPointer());

    _spe_corrector = new SPECorrector(p.get<fhicl::ParameterSet>("speparams"), _verbosity);
  }
  else if(_mode == "baselinecorrector"){
    _baseline_corrector = new BaselineCorrector(p.get<fhicl::ParameterSet>("baselineparams"));
    _baseline_corrector->setVerbosity(_verbosity);
    _baseline_corrector->setUtilities(_utils->getPointer());
  }
  else if(_mode == "saturationcorrector"){
    _saturation_corrector = new SaturationCorrector(p.get<fhicl::ParameterSet>("saturationparams"));
    _saturation_corrector->setUtilities(_utils->getPointer());
  }
  else if(_mode == "specorrector"){
    _spe_corrector = new SPECorrector(p.get<fhicl::ParameterSet>("speparams"), _verbosity);
  }
  else if(_mode == "coercecorrector"){
    _coerce_corrector = new CoerceCorrector(p.get<fhicl::ParameterSet>("coerceparams"), _verbosity);
    _coerce_corrector->setUtilities(_utils->getPointer());
  }
  else throw cet::exception("PulseCorrector") << "Undefined mode: " << _mode << ")\n";

  produces<darkart::od::ChannelDataVec>();
  produces<darkart::od::ChannelWFsVec>();
}

void darkart::od::PulseCorrector::produce(art::Event & e)
{

  // These are the old channel vectors that we'll be extracting things from
  auto const & raw_channel_data_vec = e.getValidHandle<darkart::od::ChannelDataVec>(_input_pulses);
  auto const & raw_channel_wfs_vec = e.getValidHandle<darkart::od::ChannelWFsVec>(_input_pulses);

  // These are the new channel vectors that we'll be populating with corrected pulses
  // We start by copying the old objects into the new ones

  std::unique_ptr<darkart::od::ChannelDataVec> corrected_channel_data_vec(new darkart::od::ChannelDataVec(*raw_channel_data_vec));
  std::unique_ptr<darkart::od::ChannelWFsVec> corrected_channel_wfs_vec(new darkart::od::ChannelWFsVec(*raw_channel_wfs_vec));

  // Loop over channels
  const size_t chan_vec_size = raw_channel_data_vec->size();
  for(size_t ch_idx = 0; ch_idx < chan_vec_size; ch_idx++){
    darkart::od::ChannelData & corrected_channel_data = corrected_channel_data_vec->at(ch_idx);
    darkart::od::ChannelWFs & corrected_channel_wfs = corrected_channel_wfs_vec->at(ch_idx);

    // Check that channel IDs agree
    if(corrected_channel_wfs.channel_id != corrected_channel_data.channel_id)
      throw cet::exception("PulseCorrector") << "Channel ID mismatch in produce function: (" << corrected_channel_wfs.channel_id << ", " << corrected_channel_data.channel_id << ")\n";

    // Loop over pulses
    for(int p_idx = 0; p_idx < corrected_channel_data.npulses; p_idx++){
      darkart::od::ChannelData::Pulse & pulse = corrected_channel_data.pulses.at(p_idx);
      darkart::od::ChannelWFs::PulseWF & wf = corrected_channel_wfs.pulse_wfs.at(p_idx);

      // Check that pulse IDs agree
      if(pulse.pulse_id != wf.pulse_id)
	throw cet::exception("PulseCorrector") << "Pulse ID mismatch in produce function: (" << pulse.pulse_id << ", " << wf.pulse_id << ")\n";


      // Apply corrections that are being used
      if( _mode_default)
	modeDefault(pulse,wf,int(corrected_channel_data.channel_id));
      else {
        if(_saturation_corrector)
	  modeSaturationCorrector(pulse,wf,int(corrected_channel_data.channel_id));
        if(_baseline_corrector)
	  modeBaselineCorrector(pulse,wf,int(corrected_channel_data.channel_id));
        if(_coerce_corrector)
	  modeCoerceCorrector(pulse,wf,int(corrected_channel_data.channel_id));
        if(_spe_corrector)
	  modeSPECorrector(pulse,wf,int(corrected_channel_data.channel_id));
	}
      } // End p_idx loop
  } // End ch_idx loop

  // Insert the products into the ART event
  e.put(std::move(corrected_channel_data_vec));
  e.put(std::move(corrected_channel_wfs_vec)); 
}

void darkart::od::PulseCorrector::beginRun(art::Run & r)
{

  // Get the sample rate (in samples/second)
  art::Handle<darkart::od::RunInfo> riHandle;
  r.getByLabel("ODReader",riHandle);
  _ch_conf_data_vec = riHandle->chHCD;
  
  // Execute submodule functions that need to happen at beginRun
  if(_spe_corrector){
    // Load SPE file in _spe_corrector
    //ToDo: transfer the database interface from the TPC to here, to pick up the SPE calibrations from DB
    //    _spe_corrector->loadSPEFile(int(r.run()));
    _spe_corrector->loadSPEVals();
  }
}

// Mode definitions
void darkart::od::PulseCorrector::modeBaselineCorrector(darkart::od::ChannelData::Pulse &pulse, darkart::od::ChannelWFs::PulseWF &wf, int ch_id){
  
  // Handle baseline corrector functions
  _baseline_corrector->findBaseline(wf);
  _baseline_corrector->subtractBaseline(wf);
  _baseline_corrector->integrate(wf,pulse, ch_id);
  _baseline_corrector->calculateAmplitude(pulse);

  pulse.pedestal_mean = _baseline_corrector->getPedestalMean();
  pulse.pedestal_rms = _baseline_corrector->getPedestalRMS();
  pulse.integral = _baseline_corrector->getIntegral();
  pulse.peak_amplitude = _baseline_corrector->getAmplitude();
}
void darkart::od::PulseCorrector::modeSPECorrector(darkart::od::ChannelData::Pulse &pulse, darkart::od::ChannelWFs::PulseWF &wf, int ch_id){
  
  // Handle baseline corrector functions
  _spe_corrector->normalizeWaveform(wf, ch_id);
  _spe_corrector->normalizePulseMomenta(pulse, ch_id);
  
  pulse.integral = _spe_corrector->getIntegral();
  pulse.peak_amplitude = _spe_corrector->getPeakAmplitude();
  pulse.pedestal_rms = _spe_corrector->getPedestalRMS();
  //pedestal_mean is not corrected for gain
}
void darkart::od::PulseCorrector::modeSaturationCorrector(darkart::od::ChannelData::Pulse &pulse, darkart::od::ChannelWFs::PulseWF &wf, int ch_id){
  const double vertical_range  = _ch_conf_data_vec.at(ch_id).vertical_range;
  const double vertical_offset = _ch_conf_data_vec.at(ch_id).vertical_offset;
  
  // Handle baseline corrector functions
  _saturation_corrector->isSaturated(wf,pulse,vertical_range,vertical_offset, ch_id);
}

void darkart::od::PulseCorrector::modeCoerceCorrector(darkart::od::ChannelData::Pulse &pulse, darkart::od::ChannelWFs::PulseWF &wf, int ch_id){
  
  // Handle baseline corrector functions
  _coerce_corrector->coerce(wf, ch_id);
  if (_coerce_corrector->getCoerced()){
    pulse.integral       = _coerce_corrector->getIntegral();
    pulse.peak_amplitude = _coerce_corrector->getAmplitude();
    pulse.peak_sample    = _coerce_corrector->getPeakSample();
  }
}

void darkart::od::PulseCorrector::modeDefault(darkart::od::ChannelData::Pulse &pulse, darkart::od::ChannelWFs::PulseWF &wf, int ch_id){
  const double vertical_range  = _ch_conf_data_vec.at(ch_id).vertical_range;
  const double vertical_offset = _ch_conf_data_vec.at(ch_id).vertical_offset;

  _saturation_corrector->isSaturated(wf, pulse, vertical_range, vertical_offset, ch_id);
  
  // Handle baseline corrector functions
  _baseline_corrector->findBaseline(wf);
  _baseline_corrector->subtractBaseline(wf);
  _baseline_corrector->integrate(wf, pulse, ch_id);
  _baseline_corrector->calculateAmplitude(pulse);

  pulse.pedestal_mean  = _baseline_corrector->getPedestalMean();
  pulse.pedestal_rms   = _baseline_corrector->getPedestalRMS();
  pulse.integral       = _baseline_corrector->getIntegral();
  pulse.peak_amplitude = _baseline_corrector->getAmplitude();

  // Handle coerce corrector functions
  _coerce_corrector->coerce(wf, ch_id);
  if (_coerce_corrector->getCoerced()){
    pulse.integral       = _coerce_corrector->getIntegral();
    pulse.peak_amplitude = _coerce_corrector->getAmplitude();
    pulse.peak_sample    = _coerce_corrector->getPeakSample();
    if ((_saturation_corrector->getIsSaturated()) && (_saturation_corrector->getReplaceWFWithSatTrig()==false)){
      // if saturation correction does not replace the waveform, the saturation correction needs to be applied again
      pulse.integral -= pulse.saturation_correction;
    }
  }
  
  // Handle SPE corrector functions
  _spe_corrector->normalizeWaveform(wf, ch_id);
  _spe_corrector->normalizePulseMomenta(pulse, ch_id);
  
  pulse.integral       = _spe_corrector->getIntegral();
  pulse.peak_amplitude = _spe_corrector->getPeakAmplitude();
  //pedestal_mean and rms are not corrected for gain
}

DEFINE_ART_MODULE(darkart::od::PulseCorrector)
