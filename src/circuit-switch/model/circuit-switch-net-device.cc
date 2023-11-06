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

#include "circuit-switch-net-device.h"

#include "ns3/boolean.h"
#include "ns3/bridge-channel.h"
#include "ns3/log.h"
#include "ns3/net-device-container.h"
#include "ns3/node.h"
#include "ns3/packet.h"
#include "ns3/simulator.h"
#include "ns3/uinteger.h"

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

NS_LOG_COMPONENT_DEFINE("CircuitSwitchNetDevice");

NS_OBJECT_ENSURE_REGISTERED(CircuitSwitchNetDevice);

TypeId
CircuitSwitchNetDevice::GetTypeId()
{
    static TypeId tid =
        TypeId("ns3::CircuitSwitchNetDevice")
            .SetParent<NetDevice>()
            .SetGroupName("CircuitSwitch")
            .AddConstructor<CircuitSwitchNetDevice>()
            .AddAttribute("Bidirectional",
                          "Whether circuits allow bidirectional transmission.",
                          BooleanValue{false},
                          MakeBooleanAccessor(&CircuitSwitchNetDevice::m_bidir),
                          MakeBooleanChecker())
            .AddAttribute(
                "ReconfigurationTime",
                "Time it takes to reconfigure the switch to a different circuit configuration.",
                TimeValue{},
                MakeTimeAccessor(&CircuitSwitchNetDevice::m_reconfTime),
                MakeTimeChecker())
            .AddTraceSource("Reconfiguring",
                            "Trace the start of a reconfiguration.",
                            MakeTraceSourceAccessor(&CircuitSwitchNetDevice::m_traceReconf),
                            "ns3::CircuitSwitchNetDevice::ReconfigurationTracedCallback")
            .AddTraceSource("ReconfigurationDone",
                            "Trace the end of a reconfiguration.",
                            MakeTraceSourceAccessor(&CircuitSwitchNetDevice::m_traceReconfDone),
                            "ns3::CircuitSwitchNetDevice::ReconfigurationTracedCallback");
    return tid;
}

CircuitSwitchNetDevice::CircuitSwitchNetDevice()
    : m_channel{CreateObject<BridgeChannel>()},
      m_ifIndex{0},
      m_mtu(-1),
      m_bidir{false},
      m_isReconfiguring{false}
{
    NS_LOG_FUNCTION(this);
}

CircuitSwitchNetDevice::~CircuitSwitchNetDevice()
{
    NS_LOG_FUNCTION(this);
}

void
CircuitSwitchNetDevice::AddLinkChangeCallback(Callback<void> callback)
{
}

void
CircuitSwitchNetDevice::AddSwitchPort(Ptr<NetDevice> port)
{
    NS_LOG_FUNCTION(this << port);

    NS_ASSERT(port != nullptr);
    NS_ASSERT(port != this);

    NS_LOG_DEBUG("RegisterProtocolHandler for " << port->GetInstanceTypeId().GetName());
    m_node->RegisterProtocolHandler(MakeCallback(&CircuitSwitchNetDevice::ReceiveFromDevice, this),
                                    0,
                                    port,
                                    true);
    m_ports.push_back(port);
    m_channel->AddChannel(port->GetChannel());
}

void
CircuitSwitchNetDevice::DoDispose()
{
    NS_LOG_FUNCTION(this);

    m_channel = nullptr;
    m_node = nullptr;
    m_ports.clear();

    NetDevice::DoDispose();
}

void
CircuitSwitchNetDevice::DoInitialize()
{
    NS_LOG_FUNCTION(this);

    NS_ASSERT_MSG(m_reconfTime.IsStrictlyPositive(), "ReconfigurationTime must be greater than 0s");

    NetDevice::DoInitialize();
}

Address
CircuitSwitchNetDevice::GetAddress() const
{
    NS_LOG_FUNCTION(this);
    return m_address;
}

Address
CircuitSwitchNetDevice::GetBroadcast() const
{
    NS_LOG_FUNCTION(this);
    NS_FATAL_ERROR("CircuitSwitchNetDevice::GetBroadcast() was called");
}

Ptr<Channel>
CircuitSwitchNetDevice::GetChannel() const
{
    NS_LOG_FUNCTION(this);
    return m_channel;
}

const CircuitConfiguration&
CircuitSwitchNetDevice::GetConfiguration() const
{
    return m_circuits;
}

uint32_t
CircuitSwitchNetDevice::GetIfIndex() const
{
    NS_LOG_FUNCTION(this);
    return m_ifIndex;
}

uint16_t
CircuitSwitchNetDevice::GetMtu() const
{
    NS_LOG_FUNCTION(this);
    return m_mtu;
}

Address
CircuitSwitchNetDevice::GetMulticast(Ipv4Address multicastGroup) const
{
    NS_LOG_FUNCTION(this << multicastGroup);
    NS_FATAL_ERROR("CircuitSwitchNetDevice::GetMulticast() was called");
}

Address
CircuitSwitchNetDevice::GetMulticast(Ipv6Address addr) const
{
    NS_LOG_FUNCTION(this << addr);
    NS_FATAL_ERROR("CircuitSwitchNetDevice::GetMulticast() was called");
}

Ptr<Node>
CircuitSwitchNetDevice::GetNode() const
{
    NS_LOG_FUNCTION(this);
    return m_node;
}

uint32_t
CircuitSwitchNetDevice::GetNSwitchPorts() const
{
    NS_LOG_FUNCTION(this);
    return std::size(m_ports);
}

Ptr<NetDevice>
CircuitSwitchNetDevice::GetSwitchPort(uint32_t n) const
{
    NS_LOG_FUNCTION(this << n);
    return m_ports[n];
}

bool
CircuitSwitchNetDevice::NeedsArp() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
CircuitSwitchNetDevice::IsBridge() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
CircuitSwitchNetDevice::IsBroadcast() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
CircuitSwitchNetDevice::IsLinkUp() const
{
    NS_LOG_FUNCTION(this);
    return true;
}

bool
CircuitSwitchNetDevice::IsMulticast() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

bool
CircuitSwitchNetDevice::IsPointToPoint() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

void
CircuitSwitchNetDevice::ReceiveFromDevice(Ptr<NetDevice> incomingPort,
                                          Ptr<const Packet> packet,
                                          uint16_t protocol,
                                          const Address& src,
                                          const Address& dst,
                                          PacketType packetType)
{
    NS_LOG_FUNCTION(this << incomingPort << packet << protocol << src << dst << packetType);

    NS_LOG_DEBUG("UID is " << packet->GetUid());

    if (m_isReconfiguring)
    {
        NS_LOG_LOGIC("Cannot receive while reconfiguring.");
        return;
    }

    if (!m_promiscRxCallback.IsNull())
    {
        m_promiscRxCallback(this, packet, protocol, src, dst, packetType);
    }

    auto inPortIt = std::find(std::begin(m_ports), std::end(m_ports), incomingPort);
    if (inPortIt == std::end(m_ports))
    {
        NS_FATAL_ERROR("Received packet on unknown port.");
    }

    std::size_t inPortIdx = inPortIt - std::begin(m_ports);
    auto inPortMatchIt =
        std::find_if(std::begin(m_circuits),
                     std::end(m_circuits),
                     [bidir = m_bidir, inPortIdx](const auto& c) {
                         return inPortIdx == c.first || (bidir && inPortIdx == c.second);
                     });
    if (inPortMatchIt == std::end(m_circuits))
    {
        NS_LOG_DEBUG("Dropping packet on unmatched port.");
        return;
    }

    auto outPortIdx =
        inPortIdx == inPortMatchIt->first ? inPortMatchIt->second : inPortMatchIt->first;
    NS_ASSERT(outPortIdx < std::size(m_ports));
    NS_ASSERT(inPortIdx != outPortIdx);
    auto outPort = m_ports[outPortIdx];
    NS_ASSERT(*inPortIt != outPort);

#if 0
    switch (packetType)
    {
    case PACKET_HOST:
        m_rxCallback(this, packet, protocol, src);
        break;
    case PACKET_BROADCAST:
        [[fallthrough]];
    case PACKET_MULTICAST:
        [[fallthrough]];
    case PACKET_OTHERHOST:
        NS_LOG_DEBUG("Forwarding packet from port " << inPortIdx << " to port " << outPortIdx);
        m_rxCallback(this, packet, protocol, src);
        outPort->Send(packet->Copy(), dst, protocol);
        break;
    }
#else
    NS_LOG_DEBUG("Forwarding packet from port " << inPortIdx << " to port " << outPortIdx);
    // m_rxCallback(this, packet, protocol, src);
    outPort->Send(packet->Copy(), dst, protocol);
#endif
}

void
CircuitSwitchNetDevice::ReconfigurationDone()
{
    NS_LOG_FUNCTION(this);

    m_isReconfiguring = false;

    if (g_log.IsEnabled(ns3::LOG_DEBUG))
    {
        NS_LOG_DEBUG("Reconfiguration done");

        std::ostringstream oss;
        oss << "New configuration:";
        if (std::empty(m_circuits))
        {
            oss << " empty";
        }
        const auto circStr = m_bidir ? "<->" : "->";
        for (const auto& [in, out] : m_circuits)
        {
            oss << ' ' << in << circStr << out;
        }
        NS_LOG_DEBUG(oss.str());
    }

    m_traceReconfDone(m_circuits);
}

void
CircuitSwitchNetDevice::Reconfigure(const CircuitConfiguration& configuration, bool immediately)
{
    NS_LOG_FUNCTION(this);

    m_reconfEv.Cancel();
    m_circuits.clear();

    for (const auto& circuit : configuration)
    {
        NS_ASSERT(circuit < std::make_pair(std::size(m_ports), std::size(m_ports)));

        auto [in, out] = circuit;

        auto it = std::find_if(std::begin(m_circuits), std::end(m_circuits), [in](const auto& c) {
            return in == c.first;
        });
        if (it != std::end(m_circuits))
        {
            NS_FATAL_ERROR("Port " << in << " matched twice in configuration.");
        }

        if (m_bidir)
        {
            auto it = std::find_if(std::begin(m_circuits),
                                   std::end(m_circuits),
                                   [out](const auto& c) { return out == c.first; });
            if (it != std::end(m_circuits))
            {
                NS_FATAL_ERROR("Port " << out << " matched twice in bidirectional configuration.");
            }
        }

        m_circuits.push_back(circuit);
    }

    if (!immediately)
    {
        m_isReconfiguring = true;
        NS_LOG_DEBUG("Reconfiguration started");
        m_reconfEv =
            Simulator::Schedule(m_reconfTime, &CircuitSwitchNetDevice::ReconfigurationDone, this);
        m_traceReconf(m_circuits);
    }
    else
    {
        ReconfigurationDone();
    }
}

void
CircuitSwitchNetDevice::SetIfIndex(const uint32_t index)
{
    NS_LOG_FUNCTION(this << index);
    m_ifIndex = index;
}

void
CircuitSwitchNetDevice::SetAddress(Address address)
{
    NS_LOG_FUNCTION(this << address);
    m_address = address;
}

bool
CircuitSwitchNetDevice::SetMtu(const uint16_t mtu)
{
    NS_LOG_FUNCTION(this << mtu);
    m_mtu = mtu;
    return true;
}

bool
CircuitSwitchNetDevice::Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << dest << protocolNumber);
    NS_FATAL_ERROR("CircuitSwitchNetDevice::Send() was called");
}

bool
CircuitSwitchNetDevice::SendFrom(Ptr<Packet> packet,
                                 const Address& src,
                                 const Address& dest,
                                 uint16_t protocolNumber)
{
    NS_LOG_FUNCTION(this << packet << src << dest << protocolNumber);
    NS_FATAL_ERROR("CircuitSwitchNetDevice::SendFrom() was called");
}

void
CircuitSwitchNetDevice::SetNode(Ptr<Node> node)
{
    NS_LOG_FUNCTION(this << node);
    m_node = node;
}

void
CircuitSwitchNetDevice::SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb)
{
    NS_LOG_FUNCTION(this);
    m_promiscRxCallback = cb;
}

void
CircuitSwitchNetDevice::SetReceiveCallback(NetDevice::ReceiveCallback cb)
{
    NS_LOG_FUNCTION(this);
    m_rxCallback = cb;
}

bool
CircuitSwitchNetDevice::SupportsSendFrom() const
{
    NS_LOG_FUNCTION(this);
    return false;
}

} // namespace ns3
