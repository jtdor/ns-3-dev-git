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

#include "circuit-switch-helper.h"

#include "ns3/circuit-switch-net-device.h"
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

NS_LOG_COMPONENT_DEFINE("CircuitSwitchHelper");

CircuitSwitchHelper::CircuitSwitchHelper()
{
    NS_LOG_FUNCTION(this);
    m_deviceFactory.SetTypeId("ns3::CircuitSwitchNetDevice");
}

void
CircuitSwitchHelper::SetDeviceAttribute(std::string n1, const AttributeValue& v1)
{
    NS_LOG_FUNCTION(this << n1);
    m_deviceFactory.Set(n1, v1);
}

NetDeviceContainer
CircuitSwitchHelper::Install(Ptr<Node> node,
                             const NetDeviceContainer& ports,
                             CircuitConfiguration configuration) const
{
    NS_LOG_FUNCTION(this << node);

    NS_ASSERT(node != nullptr);

    auto dev = m_deviceFactory.Create<CircuitSwitchNetDevice>();
    node->AddDevice(dev);
    std::for_each(ports.Begin(), ports.End(), [&dev](auto& port) { dev->AddSwitchPort(port); });
    if (!std::empty(configuration))
    {
        dev->Reconfigure(std::move(configuration), true);
    }

    return {dev};
}

NetDeviceContainer
CircuitSwitchHelper::Install(std::string nodeName,
                             const NetDeviceContainer& ports,
                             CircuitConfiguration configuration) const
{
    NS_LOG_FUNCTION(this << nodeName);
    auto node = Names::Find<Node>(nodeName);
    return Install(node, ports, std::move(configuration));
}

} // namespace ns3
