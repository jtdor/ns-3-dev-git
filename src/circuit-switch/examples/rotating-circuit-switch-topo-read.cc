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
#include "ns3/topology-read-module.h"

#include <algorithm>

/**
 * \file
 * \ingroup circuit-switch
 *
 * Circuit switch example with the circuit configurations read from an external file.
 */

using namespace ns3;

NS_LOG_COMPONENT_DEFINE("RotatingCircuitSwitchTopologyReaderExample");

namespace
{

/**
 * \brief Create circuit configurations from a topology reader’s links.
 *
 * \note The links must be sorted by their weight in increasing order in the topology file, e.g.:
 * \code
 *   3 6
 *   0  0  0
 *   1  0  0
 *   2  0  0
 *   0  1  0
 *   1  2  0
 *   2  1  0
 *   0  2  1
 *   1  0  1
 *   2  1  1
 * \endcode
 *
 * \see Details on the Inet topology file format:
 * https://web.archive.org/web/20210421092910/http://topology.eecs.umich.edu/inet/inet-3.0.pdf
 *
 * \param first Iterator pointing to the first link (return value of TopologyReader::LinksBegin()).
 * \param last Iterator pointing past the last link (return value of TopologyReader::LinksEnd()).
 * \return A vector of circuit configurations.
 */
CircuitConfigurations
ConfigurationsFromLinks(TopologyReader::ConstLinksIterator first,
                        TopologyReader::ConstLinksIterator last)
{
    /* We repurpose the links from the topology file to define our rotating circuit configurations
     * for the switch. Each link specifies a circuit belonging to a single configuration.
     */
    CircuitConfiguration conf;
    CircuitConfigurations confs;
    auto prevConfI = 0;

    std::for_each(first, last, [&](const auto& link) {
        /* We repurpose the weight attribute to index our rotating configurations. Circuits with the
         * same index belong to the same configuration.
         */
        auto confI = std::stoi(link.GetAttribute("Weight"));
        if (confI != prevConfI)
        {
            /* A vector is guaranteed to be empty after moving from it, no clear() necessary. */
            confs.push_back(std::move(conf));
        }

        auto fromI = std::stoi(link.GetFromNodeName());
        auto toI = std::stoi(link.GetToNodeName());
        conf.emplace_back(fromI, toI);

        prevConfI = confI;
    });

    /* Don’t forget to add the last configuration, there was no changing index after it! */
    if (!std::empty(conf))
    {
        confs.push_back(std::move(conf));
    }

    return confs;
}

} // namespace

int
main(int argc, char* argv[])
{
    CommandLine cmd(__FILE__);
    cmd.Parse(argc, argv);

    TopologyReaderHelper topoReaderHelp;
    topoReaderHelp.SetFileType("Inet");
    topoReaderHelp.SetFileName(
        "src/circuit-switch/examples/rotating-circuit-switch-topo-read-tors.txt");
    auto topoReader = topoReaderHelp.GetTopologyReader();

    NS_LOG_INFO("Create nodes");
    auto nodes = topoReader->Read();
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
    auto circuitSwitch = csHelp.Install(switchNode, switchPorts);
    auto switchDev = circuitSwitch.Get(0)->GetObject<CircuitSwitchNetDevice>();

    CircuitConfigurationRotatorHelper rotHelp;
    /* With the above 10us reconfiguration time, the below results in a duty cycle of 0.9. */
    rotHelp.SetAttribute("ReconfigurationInterval", TimeValue{MicroSeconds(100)});
    rotHelp.Install(switchDev);

    auto rotator = switchDev->GetObject<CircuitConfigurationRotator>();
    rotator->AddConfigurations(
        ConfigurationsFromLinks(topoReader->LinksBegin(), topoReader->LinksEnd()));

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
    p2pHelp.EnableAsciiAll(asciiHelp.CreateFileStream("rotating-circuit-switch-topo-read.tr"));
    p2pHelp.EnablePcapAll("rotating-circuit-switch-topo-read", false);

    NS_LOG_INFO("Run simulation");

    /* Since the rotating switch configuration runs forever, we have to set an explicit stopping
     * time for the simulation.
     */
    Simulator::Stop(Seconds(6));
    Simulator::Run();
    Simulator::Destroy();
}
