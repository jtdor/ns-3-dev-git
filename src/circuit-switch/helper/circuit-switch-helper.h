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
#ifndef CIRCUIT_SWITCH_HELPER_H
#define CIRCUIT_SWITCH_HELPER_H

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
class NetDeviceContainer;
class Node;

/**
 * \ingroup circuit-switch
 *
 * Helper for installing a CircuitSwitchNetDevice on a node.
 */
class CircuitSwitchHelper
{
  public:
    CircuitSwitchHelper();

    /**
     * Set an attribute on each ns3::CircuitSwitchNetDevice created by CircuitSwitchHelper::Install.
     *
     * \param n1 The name of the attribute to set.
     * \param v1 The value of the attribute to set.
     */
    void SetDeviceAttribute(std::string n1, const AttributeValue& v1);

    /**
     * Create an ns3::CircuitSwitchNetDevice with the attributes set by
     * CircuitSwitchHelper::SetDeviceAttribute, add the device to the node, and attach the given
     * devices as ports to the circuit switch.
     *
     * \param node The node to install the device in.
     * \param ports Container of NetDevices to add as switch ports.
     * \param circuit Optional initial circuit configuration to apply.
     * \returns A container holding the created CircuitSwitchNetDevice.
     */
    NetDeviceContainer Install(Ptr<Node> node,
                               const NetDeviceContainer& ports,
                               CircuitConfiguration configuration = {}) const;

    /**
     * This method creates a ns3::CircuitSwitchNetDevice with the attributes set by
     * CircuitSwitchHelper::SetDeviceAttribute, add the device to the node, and attach the given
     * devices as ports to the circuit switch.
     *
     * \param nodeName Name of the node to install the device in.
     * \param ports Container of NetDevices to add as switch ports.
     * \param circuit Optional initial circuit configuration to apply.
     * \returns A container holding the created CircuitSwitchNetDevice.
     */
    NetDeviceContainer Install(std::string nodeName,
                               const NetDeviceContainer& ports,
                               CircuitConfiguration configuration = {}) const;

  private:
    ObjectFactory m_deviceFactory;
};

} // namespace ns3

#endif /* CIRCUIT_SWITCH_HELPER_H */
