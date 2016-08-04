////////////////////////////////////////////////////////////////////////
// Class:       BaselineCorrector
// Module Type: Submodule of PulseCorrector
// File:        BaselineCorrector.cc
//
// Generated at Tue Feb  3 08:28:02 2015 by Shawn Westerdale.
////////////////////////////////////////////////////////////////////////

#include "art/Framework/Core/EDProducer.h"
#include "art/Framework/Core/ModuleMacros.h"
#include "art/Framework/Principal/Event.h"
#include "art/Framework/Principal/Handle.h"
#include "art/Framework/Principal/Run.h"
#include "art/Framework/Principal/SubRun.h"
#include "art/Utilities/InputTag.h"
#include "art/Framework/Services/Registry/ServiceHandle.h"

#include "darksidecore/ArtServices/DBInterface.hh"

#include "cetlib/exception.h"
#include "fhiclcpp/ParameterSet.h"
#include "messagefacility/MessageLogger/MessageLogger.h"

#include <memory>

#include "darkart/ODProducts/ODEventInfo.hh"
#include "darkart/ODProducts/ChannelData.hh"
#include "darkart/ODProducts/ChannelWFs.hh"

class SPECorrector {
public:
  // Constructor and Destructor
  SPECorrector(fhicl::ParameterSet const &p, const int verbosity=0);
  ~SPECorrector(){;}

  void normalizeWaveform(darkart::od::ChannelWFs::PulseWF & wf, const int ch_id);
  void normalizePulseMomenta(darkart::od::ChannelData::Pulse & pulse, const int ch_id); //momenta of a distribution, here: integral, amplitude, pedestal_rms

  float getPedestalRMS()   const { return _pedestal_rms;}
  float getPeakAmplitude() const { return _peak_amplitude;}
  float getIntegral()      const { return _integral;}

  void loadSPEVals(); //if from file, filename is set in fhicl file

private:
  float _integral;
  float _peak_amplitude;
  float _pedestal_rms; 
  //float _pedestal_mean; //not scaled by spe_mean
  int _verbosity;
  bool _read_SPEmeans_from_file;
  std::string _spe_filename;
  std::string _db_table_version; 
  int _run_num;

  std::vector<int> _ch_v;
  std::vector<float> _spe_mean_v;

  float _getSPE(const int channel_ID) const;
  
};
