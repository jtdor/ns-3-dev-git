/*
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 * Author: Jörn-Thorben Hinz
 */
#ifndef CIRCUIT_CONFIGURATION_ROTATOR_HELPER_H
#define CIRCUIT_CONFIGURATION_ROTATOR_HELPER_H

#include "ns3/circuit-configuration-rotator.h"
#include "ns3/circuit-switch-net-device.h"
#include "ns3/object-factory.h"
#include "ns3/ptr.h"

#include <string>

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

class AttributeValue;
class Node;

/**
 * \ingroup circuit-switch
 *
 * Helper for installing a CircuitConfigurationRotator on a CircuitSwitchNetDevice.
 */
class CircuitConfigurationRotatorHelper
{
  public:
    CircuitConfigurationRotatorHelper();

    /**
     * Set an attribute on each CircuitConfigurationRotator created by
     * CircuitConfigurationRotatorHelper::Install.
     *
     * \param n1 the name of the attribute to set
     * \param v1 the value of the attribute to set
     */
    void SetAttribute(std::string n1, const AttributeValue& v1);

    /**
     * Create an CircuitConfigurationRotator with the attributes set by
     * CircuitConfigurationRotatorHelper::SetAttribute, aggregate the rotator with the device, and
     * add the given configurations to the rotator.
     *
     * \param dev The device to aggregate the rotator with.
     * \param configurations Optional circuit configurations to add to the rotator.
     */
    void Install(Ptr<CircuitSwitchNetDevice> dev, CircuitConfigurations configurations = {}) const;

  private:
    ObjectFactory m_factory;
};

} // namespace ns3

#endif /* CIRCUIT_CONFIGURATION_ROTATOR_HELPER_H */
