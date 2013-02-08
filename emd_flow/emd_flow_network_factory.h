#ifndef __EMD_FLOW_NETWORK_FACTORY_H__
#define __EMD_FLOW_NETWORK_FACTORY_H__

#include "emd_flow_network.h"

#include <memory>
#include <string>

class EMDFlowNetworkFactory {
 public:
  enum EMDFlowNetworkType {
    kLemonCostScaling,
    kLemonNetworkSimplex,
    kLemonCapacityScaling,
    kUnknownType,
  };

  static std::unique_ptr<EMDFlowNetwork> create_EMD_flow_network(
      const std::vector<std::vector<double> >& amplitudes,
      EMDFlowNetworkType type);

  static EMDFlowNetworkType parse_type(const std::string& name);
};

#endif
