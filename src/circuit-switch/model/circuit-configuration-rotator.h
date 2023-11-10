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
#ifndef CIRCUIT_CONFIGURATION_ROTATOR_H
#define CIRCUIT_CONFIGURATION_ROTATOR_H

#include "ns3/circuit-switch-net-device.h"
#include "ns3/nstime.h"
#include "ns3/object.h"
#include "ns3/ptr.h"

#include <vector>

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

/**
 * \ingroup circuit-switch
 *
 * A vector of configurations a ns3::CircuitConfigurationRotator will rotate over.
 */
using CircuitConfigurations = std::vector<CircuitConfiguration>;

/**
 * \ingroup circuit-switch
 *
 * A class rotating over multiple circuit configurations, periodically reconfiguring an
 * ns3::CircuitSwitchNetDevice.
 */
class CircuitConfigurationRotator : public Object
{
  public:
    static TypeId GetTypeId();

    /**
     * Add (append) a circuit configuration to the rotation.
     *
     * \param configuration The configuration to add.
     */
    void AddConfiguration(CircuitConfiguration configuration);

    /**
     * Add (append) multiple circuit configurations to the rotation.
     *
     * \param configurations The configurations to add.
     */
    void AddConfigurations(CircuitConfigurations configurations);

  protected:
    void DoDispose() override;
    void DoInitialize() override;
    void NotifyNewAggregate() override;

  private:
    void RotateConfig(bool initial);

    CircuitSwitchNetDevice::ReconfigurationTracedCallback m_traceRotate;

    CircuitConfigurations m_configs;
    CircuitConfigurations::const_iterator m_configIter;
    Ptr<CircuitSwitchNetDevice> m_dev;
    Time m_reconfInt;
    EventId m_rotateEv;
};

} // namespace ns3

#endif /* CIRCUIT_CONFIGURATION_ROTATOR_H */
