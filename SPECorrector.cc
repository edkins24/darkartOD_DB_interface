////////////////////////////////////////////////////////////////////////
// Class:       SPECorrector
// Module Type: Submodule of PulseCorrector
// File:        SPECorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale, modified by Bernd Reinhold
////////////////////////////////////////////////////////////////////////

/*
This is a submodule of PulseCorrector
Parameters are set by parameter list: baselineparams
Required Parameters:
  - num_baseline_samples : [int]
Optional Parameters:
  - (none)
*/

#include <string>
#include <sstream>
#include <fstream>
#include <stdlib.h> //getenv
#include "darkart/ODReco/Submodules/SPECorrector.hh"

bool file_exists(std::string filename)
{
  std::ifstream infile(filename.c_str());
  return infile.good();
}


// Constructors
SPECorrector::SPECorrector(fhicl::ParameterSet const &p, const int verbosity):
  _integral(-1),
  _peak_amplitude(-1),
  _pedestal_rms(-1),
  _verbosity(verbosity),
  _spe_filename(p.get<std::string>("spe_filename",""))
{

  LOG_INFO("SPECorrector") << "verbosity:  " << _verbosity << ", spe_filename: " << _spe_filename;

;}


//channel or channle_ID?
float SPECorrector::_getSPE(const int channel_ID) const {
  const float spe_mean=_spe_mean_v[channel_ID];

  return spe_mean;
}

void SPECorrector::normalizeWaveform(darkart::od::ChannelWFs::PulseWF & wf, const int ch_id){
  const float spe_mean = _getSPE(ch_id);
  const size_t n_samp = wf.data.size();

  for(size_t samp = 0; samp < n_samp; ++samp){
    if ( spe_mean==0 ){
      wf.data[samp] = 0;
    } 
    else { 
      wf.data[samp] /= spe_mean;
    }
  }
}

//RMS, amplitude and integral are scaled by the spe_mean, but not the pedestal as it can have an arbitrary offset
void SPECorrector::normalizePulseMomenta(darkart::od::ChannelData::Pulse & pulse, const int ch_id){
  const float spe_mean = _getSPE(ch_id);
  if ( spe_mean==0 ){
    _pedestal_rms = 0.;
    _integral = 0.;
    _peak_amplitude = 0.;
  } 
  else {
    //_pedestal_rms = pulse.pedestal_rms/spe_mean;
    _pedestal_rms = pulse.pedestal_rms;
    _integral = pulse.integral/spe_mean;
    _peak_amplitude = pulse.peak_amplitude/spe_mean;
  }

  if ( _verbosity>2 )
    LOG_INFO("SPECorrector") << "channel_id:  " << ch_id << ", spe_mean: " << spe_mean << ", pedestal_rms: " << _pedestal_rms << ", integral: " << _integral << ", peak_amplitude: " << _peak_amplitude;

}


// load the SPE from a text file (rather than DB)
void SPECorrector::loadSPEFile(){

  // Read SPEs in from file
  //if ( _verbosity > 0 )
  //  LOG_INFO("SPECorrector") << "OD SPE file: " << _spe_filename;

  // check for existence of _spe_filename in a few places
  std::string filename = "./"+_spe_filename; //first check relative to pwd
  if (!file_exists(filename)) { // if not in pwd, check in build directory
    char const* tmp = std::getenv( "CETPKG_BUILD" );
    if (tmp!=NULL) filename = std::string(tmp) + "/" + _spe_filename;
  }
  if (!file_exists(filename)) { // check in installation dir
    char const* tmp = std::getenv( "DARKART_DIR" );
    if (tmp!=NULL) filename = std::string(tmp) + "/source/" + _spe_filename;
  }
  if (!file_exists(filename)) { // uh oh
    throw cet::exception("SPECorrector") << "ERROR: could not find " << _spe_filename<<std::endl;
  }
  
  std::ifstream spe_data(filename.c_str());
  if ((!spe_data) || !spe_data.is_open()){
    throw cet::exception("SPECorrector") << "ERROR: opening SPE file "<< spe_data << " (" << _spe_filename << ")" << std::endl;
  }
  else { if (_verbosity > 0) LOG_INFO("SPECorrector") << "OD SPE file: " << filename; }

  //fill channel and spe from file
  int ch = 0;
  float spe = 0.;

  while (spe_data >> ch >> spe){
    _ch_v.push_back(ch);
    _spe_mean_v.push_back(spe);

    if (_verbosity > 0) {
      if ( spe==0. )
        LOG_INFO("SPECorrector") << "Channel ID " << ch << " has 0 spe_mean.";
    }
  }
}

