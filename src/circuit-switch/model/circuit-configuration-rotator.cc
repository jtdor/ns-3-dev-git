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

#include "circuit-configuration-rotator.h"

#include "ns3/circuit-switch-net-device.h"
#include "ns3/simulator.h"

#include <algorithm>
#include <iterator>
#include <utility>

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CircuitConfigurationRotator");

NS_OBJECT_ENSURE_REGISTERED(CircuitConfigurationRotator);

TypeId
CircuitConfigurationRotator::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CircuitConfigurationRotator")
            .SetParent<Object>()
            .SetGroupName("CircuitSwitch")
            .AddConstructor<CircuitConfigurationRotator>()
            .AddAttribute("ReconfigurationInterval",
                          "Time interval between the reconfigurations of the circuit switch.",
                          TimeValue{},
                          MakeTimeAccessor(&CircuitConfigurationRotator::m_reconfInt),
                          MakeTimeChecker())
            .AddTraceSource("RotateConfiguration",
                            "Trace when the circuit configuration is rotated.",
                            MakeTraceSourceAccessor(&CircuitConfigurationRotator::m_traceRotate),
                            "ns3::CircuitSwitchNetDevice::ReconfigurationTracedCallback");
    return tid;
}

void
CircuitConfigurationRotator::AddConfiguration(CircuitConfiguration configuration)
{
    NS_LOG_FUNCTION(this);

    /* TODO: Update the remembered iterator appropriately---or only allow AddConfiguration() before
     * the simulation has started?
     */
    m_configs.push_back(std::move(configuration));
}

void
CircuitConfigurationRotator::AddConfigurations(CircuitConfigurations configurations)
{
    NS_LOG_FUNCTION(this);

    /* TODO: Update the remembered iterator appropriately---or only allow AddConfigurations() before
     * the simulation has started?
     */

    if (std::empty(m_configs))
    {
        m_configs = std::move(configurations);
    }
    else
    {
        m_configs.reserve(std::size(m_configs) + std::size(configurations));
        std::move(std::begin(configurations),
                  std::end(configurations),
                  std::back_inserter(m_configs));
    }
}

void
CircuitConfigurationRotator::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_dev = nullptr;

    Object::DoDispose();
}

void
CircuitConfigurationRotator::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(m_reconfInt.IsStrictlyPositive(),
                  "ReconfigurationInterval must be greater than 0s");
    NS_ASSERT(!std::empty(m_configs));

    m_configIter = std::end(m_configs) - 1;
    RotateConfig(true);

    Object::DoInitialize();
}

void
CircuitConfigurationRotator::NotifyNewAggregate()
{
    NS_LOG_FUNCTION(this);

    if (!m_dev)
    {
        m_dev = GetObject<CircuitSwitchNetDevice>();
    }

    Object::NotifyNewAggregate();
}

void
CircuitConfigurationRotator::RotateConfig(bool initial)
{
    NS_LOG_FUNCTION(this << initial);

    NS_ASSERT(!std::empty(m_configs));
    NS_ASSERT(m_dev != nullptr);

    ++m_configIter;
    if (m_configIter == std::end(m_configs))
    {
        m_configIter = std::begin(m_configs);
    }

    m_traceRotate(*m_configIter);
    m_dev->Reconfigure(*m_configIter, initial);
    m_rotateEv =
        Simulator::Schedule(m_reconfInt, &CircuitConfigurationRotator::RotateConfig, this, false);
}

} // namespace ns3
