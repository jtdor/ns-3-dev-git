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

#include "circuit-configuration-rotator-helper.h"

#include "ns3/circuit-configuration-rotator.h"
#include "ns3/log.h"
#include "ns3/names.h"
#include "ns3/net-device-container.h"
#include "ns3/node.h"

#include <algorithm>
#include <utility>

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CircuitConfigurationRotatorHelper");

CircuitConfigurationRotatorHelper::CircuitConfigurationRotatorHelper()
{
    NS_LOG_FUNCTION_NOARGS();
    m_deviceFactory.SetTypeId("ns3::CircuitConfigurationRotator");
}

void
CircuitConfigurationRotatorHelper::SetAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);
    m_deviceFactory.Set(n1, v1);
}

void
CircuitConfigurationRotatorHelper::Install(Ptr<CircuitSwitchNetDevice> dev,
                                           CircuitConfigurations configurations) const
{
    NS_LOG_FUNCTION(this << dev);

    auto rot = m_deviceFactory.Create<CircuitConfigurationRotator>();
    rot->AddConfigurations(std::move(configurations));
    dev->AggregateObject(rot);
}

} // namespace ns3
