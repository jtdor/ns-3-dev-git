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
 *   -----------------
 *   | Circuit switch |
 *   -----------------
 *      |  |  |  |
 *      n0 n1 n2 n3
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
 * Circuit switch example with four nodes and rotating matchings.
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RotatingCircuitSwitchExample");

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    NS_LOG_INFO("Create nodes");
    NodeContainer nodes(4);
    auto switchNode = CreateObject<Node>();

    NS_LOG_INFO("Build Topology");
    PointToPointHelper p2pHelp;
    p2pHelp.SetChannelAttribute("Delay", TimeValue{MilliSeconds(2)});
    p2pHelp.SetDeviceAttribute("DataRate", DataRateValue{5000000});

    NetDeviceContainer nodeDevs;
    NetDeviceContainer switchDevs;

    /* Connect every node (ToR switch) to the circuit switch: */
    std::for_each(nodes.Begin(), nodes.End(), [&](auto& node) {
        auto link = p2pHelp.Install(NodeContainer{node, switchNode});
        nodeDevs.Add(link.Get(0));
        switchDevs.Add(link.Get(1));
    });

    NS_LOG_INFO("Setup switch");
    CircuitSwitchHelper csHelp;
    csHelp.SetDeviceAttribute("ReconfigurationTime", TimeValue{MicroSeconds(10)});
    auto circuitSwitch = csHelp.Install(switchNode, switchDevs);

    auto switchDev = circuitSwitch.Get(0)->GetObject<CircuitSwitchNetDevice>();

    CircuitConfigurationRotatorHelper rotHelp;
    /* With the above 10us reconfiguration time, the below results in a duty cycle of 0.9. */
    rotHelp.SetAttribute("ReconfigurationInterval", TimeValue{MicroSeconds(100)});
    rotHelp.Install(switchDev,
                    {
                        {{0, 1}, {1, 2}, {2, 3}, {3, 0}},
                        {{0, 2}, {1, 3}, {2, 0}, {3, 1}},
                        {{0, 3}, {1, 0}, {2, 1}, {3, 2}},
                    });

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
    p2pHelp.EnableAsciiAll(asciiHelp.CreateFileStream("rotating-circuit-switch.tr"));
    p2pHelp.EnablePcapAll("rotating-circuit-switch", false);

    NS_LOG_INFO("Run simulation");

    /* Since the rotating switch configuration runs forever, we have to set an explicit stopping
     * time for the simulation.
     */
    Simulator::Stop(Seconds(6));
    Simulator::Run();
    Simulator::Destroy();
}
