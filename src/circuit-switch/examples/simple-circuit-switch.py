#
# This program is free software; you can redistribute it and/or modify
# it under the terms of the GNU General Public License version 2 as
# published by the Free Software Foundation
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program; if not, write to the Free Software
# Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
#
# Author: Jörn-Thorben Hinz
#

#
# Network topology:
#
#           n0
#           |
#   -----------------
#   | Circuit switch |
#   -----------------
#           |
#           n1
#
#

## \file
#  \ingroup circuit-switch
#  Simple circuit switch example with two connected nodes.

try:
    from ns import ns
except ModuleNotFoundError:
    raise SystemExit(
        "Error: ns3 Python module not found; Python bindings may not be enabled or your PYTHONPATH might not be properly configured"
    )


def main(argv):
    cmd = ns.core.CommandLine()
    cmd.Parse(argv)

    nodes = ns.network.NodeContainer(4)
    switch = ns.network.NodeContainer(1)
    switchNode = switch.Get(0)

    p2p = ns.point_to_point.PointToPointHelper()
    p2p.SetDeviceAttribute(
        "DataRate", ns.network.DataRateValue(ns.network.DataRate(5000000))
    )
    p2p.SetChannelAttribute("Delay", ns.core.TimeValue(ns.core.MilliSeconds(2)))

    nodeDevs = ns.network.NetDeviceContainer()
    switchPorts = ns.network.NetDeviceContainer()

    for i in range(nodes.GetN()):
        link = p2p.Install(nodes.Get(i), switchNode)
        nodeDevs.Add(link.Get(0))
        switchPorts.Add(link.Get(1))

    cs = ns.circuit_switch.CircuitSwitchHelper()
    circuitSwitch = cs.Install(switchNode, switchPorts)
    switchDev = circuitSwitch.Get(0)
    switchDev.AssignConfiguration([(1, 0), (0, 1)])

    internet = ns.internet.InternetStackHelper()
    internet.Install(nodes)

    ipv4 = ns.internet.Ipv4AddressHelper()
    ipv4.SetBase(
        ns.network.Ipv4Address("10.1.1.0"), ns.network.Ipv4Mask("255.255.255.0")
    )
    ipv4.Assign(nodeDevs)

    asciiTrace = ns.network.AsciiTraceHelper()
    p2p.EnableAsciiAll(asciiTrace.CreateFileStream("p2p-bridge.tr"))
    p2p.EnablePcapAll("p2p-bridge", False)

    ns.core.Simulator.Run()
    ns.core.Simulator.Destroy()


if __name__ == "__main__":
    import sys

    main(sys.argv)
