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

/*
 * Network topology:
 *
 *           n0
 *           |
 *   -----------------
 *   | Circuit switch |
 *   -----------------
 *           |
 *           n1
 *
 */

#include "ns3/circuit-switch-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/network-module.h"
#include "ns3/ping-helper.h"
#include "ns3/point-to-point-module.h"

#include <algorithm>

/**
 * \file
 * \ingroup circuit-switch
 *
 * Simple circuit switch example with two connected nodes.
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("SimpleCircuitSwitchExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Create nodes");
    NodeContainer nodes(2);
    auto switchNode = CreateObject<Node>();

    NS_LOG_INFO("Build Topology");
    PointToPointHelper p2pHelp;
    p2pHelp.SetChannelAttribute("Delay", TimeValue{MilliSeconds(2)});
    p2pHelp.SetDeviceAttribute("DataRate", DataRateValue{5000000});

    NetDeviceContainer nodeDevs;
    NetDeviceContainer switchPorts;

    /* Connect every node (ToR switch) to the circuit switch: */
    std::for_each(nodes.Begin(), nodes.End(), [&](auto& node) {
        auto link = p2pHelp.Install(NodeContainer{node, switchNode});
        nodeDevs.Add(link.Get(0));
        switchPorts.Add(link.Get(1));
    });

    NS_LOG_INFO("Setup switch");
    CircuitSwitchHelper csHelp;
    csHelp.SetDeviceAttribute("ReconfigurationTime", TimeValue{MicroSeconds(10)});
    csHelp.Install(switchNode, switchPorts, {{1, 0}, {0, 1}});

    NS_LOG_INFO("Setup nodes");
    InternetStackHelper internetHelp;
    internetHelp.Install(nodes);

    Ipv4AddressHelper ipv4Help;
    ipv4Help.SetBase("10.1.1.0", "255.255.255.0");
    auto ipv4Ifaces = ipv4Help.Assign(nodeDevs);

    NS_LOG_INFO("Create ping application");
    PingHelper pingHelp{ipv4Ifaces.GetAddress(0)};
    pingHelp.SetAttribute("Count", UintegerValue{4});
    auto app = pingHelp.Install(nodes.Get(1));
    app.Start(Seconds(1));

    NS_LOG_INFO("Configure tracing");
    AsciiTraceHelper asciiHelp;
    p2pHelp.EnableAsciiAll(asciiHelp.CreateFileStream("simple-circuit-switch.tr"));
    p2pHelp.EnablePcapAll("simple-circuit-switch", false);

    NS_LOG_INFO("Run simulation");
    Simulator::Run();
    Simulator::Destroy();
}
