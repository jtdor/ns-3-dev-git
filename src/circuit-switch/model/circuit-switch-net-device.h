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
#ifndef CIRCUIT_SWITCH_NET_DEVICE_H
#define CIRCUIT_SWITCH_NET_DEVICE_H

#include "ns3/mac48-address.h"
#include "ns3/net-device.h"
#include "ns3/nstime.h"
#include "ns3/traced-callback.h"

#include <cstdint>
#include <string>
#include <utility>
#include <vector>

/**
 * \file
 * \ingroup circuit-switch
 */

namespace ns3
{

class BridgeChannel;
class Channel;
class Node;
class Packet;

/**
 * \defgroup circuit-switch Circuit switch module
 * This module provides models and helpers for simulating a circuit switch.
 */

/*
 * \ingroup circuit-switch
 *
 * Zero-based index of a port of a circuit switch.
 */
using CircuitSwitchPortIndex = std::size_t;

/*
 * \ingroup circuit-switch
 *
 * Specifies a circuit between two ports of a circuit switch.
 */
using SwitchCircuit = std::pair<CircuitSwitchPortIndex, CircuitSwitchPortIndex>;

/**
 * \ingroup circuit-switch
 *
 * Circuit configuration (aka matching) of a circuit switch.
 */
using CircuitConfiguration = std::vector<SwitchCircuit>;

/**
 * \ingroup circuit-switch
 *
 * A virtual NetDevice that circuit-switches multiple ports.
 */
class CircuitSwitchNetDevice : public NetDevice
{
  public:
    using ReconfigurationTracedCallback = TracedCallback<const CircuitConfiguration&>;

    /**
     * Get the type ID.
     *
     * \return the object TypeId
     */
    static TypeId GetTypeId();

    CircuitSwitchNetDevice();

    ~CircuitSwitchNetDevice() override;

    // Delete copy constructor and assignment operator to avoid misuse.
    CircuitSwitchNetDevice(const CircuitSwitchNetDevice&) = delete;
    CircuitSwitchNetDevice& operator=(const CircuitSwitchNetDevice&) = delete;

    /**
     * Add a port to a circuit switch.
     *
     * \param port The NetDevice to add.
     * \attention The NetDevice that is being added as a switch port must _not_ have an IP address.
     */
    void AddSwitchPort(Ptr<NetDevice> port);

    /**
     * Return the currently applied circuit configuration.
     *
     * \return The currently applied circuit configuration.
     */
    const CircuitConfiguration& GetConfiguration() const;

    /**
     * Get the number of switch ports, i.e., the NetDevices currently added to the switch.
     *
     * \return The number of switch ports.
     */
    uint32_t GetNSwitchPorts() const;

    /**
     * Get the n-th switch port.
     *
     * \param n Index of the port to return.
     * \return The n-th switch NetDevice.
     */
    Ptr<NetDevice> GetSwitchPort(uint32_t n) const;

    /**
     * Reconfigure the switch to the given circuit configuration.
     *
     * \param configuration The configuration to apply.
     * \param immediately If true, ignore ReconfigurationTime and apply the configuration
     * immeditately.
     */
    void Reconfigure(const CircuitConfiguration& configuration, bool immediately = false);

    // Inherited from the NetDevice base class
    void AddLinkChangeCallback(Callback<void> callback) override;
    Address GetAddress() const override;
    Address GetBroadcast() const override;
    Ptr<Channel> GetChannel() const override;
    Ptr<Node> GetNode() const override;
    uint32_t GetIfIndex() const override;
    uint16_t GetMtu() const override;
    Address GetMulticast(Ipv4Address multicastGroup) const override;
    Address GetMulticast(Ipv6Address addr) const override;
    bool IsBridge() const override;
    bool IsBroadcast() const override;
    bool IsLinkUp() const override;
    bool IsMulticast() const override;
    bool IsPointToPoint() const override;
    bool NeedsArp() const override;
    bool Send(Ptr<Packet> packet, const Address& dest, uint16_t protocolNumber) override;
    bool SendFrom(Ptr<Packet> packet,
                  const Address& source,
                  const Address& dest,
                  uint16_t protocolNumber) override;
    void SetAddress(Address address) override;
    void SetIfIndex(const uint32_t index) override;
    bool SetMtu(const uint16_t mtu) override;
    void SetNode(Ptr<Node> node) override;
    void SetPromiscReceiveCallback(NetDevice::PromiscReceiveCallback cb) override;
    void SetReceiveCallback(NetDevice::ReceiveCallback cb) override;
    bool SupportsSendFrom() const override;

  protected:
    /**
     * Receive a packet from one switch port.
     *
     * \param device the originating port
     * \param packet the received packet
     * \param protocol the packet protocol
     * \param source the packet source
     * \param destination the packet destination
     * \param packetType the packet type
     */
    void ReceiveFromDevice(Ptr<NetDevice> device,
                           Ptr<const Packet> packet,
                           uint16_t protocol,
                           const Address& source,
                           const Address& destination,
                           PacketType packetType);

    void ReconfigurationDone();

    // Inherited from the NetDevice base class
    void DoDispose() override;
    void DoInitialize() override;

  private:
    NetDevice::PromiscReceiveCallback m_promiscRxCallback;
    NetDevice::ReceiveCallback m_rxCallback;
    ReconfigurationTracedCallback m_traceReconf;
    ReconfigurationTracedCallback m_traceReconfDone;

    Address m_address;
    Ptr<BridgeChannel> m_channel;
    uint32_t m_ifIndex;
    uint16_t m_mtu;
    Ptr<Node> m_node;

    /* TODO: Possibly remove this flag and allow bidirectional connections through the assignment of
     * the two circuits A -> B and B -> A?
     */
    bool m_bidir;
    CircuitConfiguration m_circuits;
    std::vector<Ptr<NetDevice>> m_ports;

    bool m_isReconfiguring;
    EventId m_reconfEv;
    Time m_reconfTime;
};

} // namespace ns3

#endif /* CIRCUIT_SWITCH_NET_DEVICE_H */
