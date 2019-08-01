/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/**
 * Copyright (c) 2011-2015  Regents of the University of California.
 *
 * This file is part of ndnSIM. See AUTHORS for complete list of ndnSIM authors and
 * contributors.
 *
 * ndnSIM is free software: you can redistribute it and/or modify it under the terms
 * of the GNU General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * ndnSIM is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along with
 * ndnSIM, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

// push-simple.cpp
// tuple

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-module.h"
#include "ns3/ndnSIM-module.h"

#include "extensions/utils.hpp"

namespace ns3 {

/**
 * This scenario simulates a very simple network topology:
 *
 *
 *      +----------+     1Mbps      +--------+     1Mbps      +----------+
 *      | consumer | <------------> | router | <------------> | producer |
 *      +----------+         10ms   +--------+          10ms  +----------+
 *
 *
 * Consumer requests data from producer with frequency 10 interests per second
 * (interests contain constantly increasing sequence number).
 *
 * For every received interest, producer replies with a data packet, containing
 * 1024 bytes of virtual payload.
 *
 * To run scenario and see what is happening, use the following command:
 *
 *     NS_LOG=ndn.Consumer:ndn.Producer ./waf --run=ndn-simple
 */

uint64_t g_numberOfPublishMessages = std::numeric_limits<uint32_t>::max();
uint32_t g_numberOfDataStream = 200;

int g_simulationTime = 300;

double g_nPStart = 5.0;
double g_nCStart = 8.0;

int parse_arguments(int argc, char *argv[]) {
	// Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	CommandLine cmd;
	cmd.AddValue ("pm", "Number of max publish messages", g_numberOfPublishMessages);
	cmd.AddValue ("ds", "Number of Data Stream", g_numberOfDataStream);
	cmd.AddValue ("duration", "Duration of simulation", g_simulationTime);
	cmd.AddValue ("c_start", "Frequency of topic generation / interest packets", g_nCStart);
	cmd.AddValue ("p_start", "Frequency of topic generation / interest packets", g_nPStart);
	cmd.Parse(argc, argv);

	NS_LOG_UNCOND("program arguments:");
	NS_LOG_UNCOND("--pm            : " << g_numberOfPublishMessages);
	NS_LOG_UNCOND("--ds            : " << g_numberOfDataStream);
	NS_LOG_UNCOND("--duration      : " << g_simulationTime);
	NS_LOG_UNCOND("--c_start       : " << g_nPStart);
	NS_LOG_UNCOND("--p_start       : " << g_nCStart);

	return 0;
}

int
main(int argc, char* argv[])
{
	int retval;

	// setting default parameters for PointToPoint links and channels
	Config::SetDefault("ns3::PointToPointNetDevice::DataRate", StringValue("1Mbps"));
	Config::SetDefault("ns3::PointToPointChannel::Delay", StringValue("10ms"));
	Config::SetDefault("ns3::QueueBase::MaxSize", StringValue("20p"));

  // Read optional command-line parameters (e.g., enable visualizer with ./waf --run=<> --visualize
	if ((retval = parse_arguments(argc, argv)) != 0) {
		return retval;
	}

	// Creating nodes
	NodeContainer nodes;
	nodes.Create(3);

	// Connecting nodes using two links
	PointToPointHelper p2p;
	p2p.Install(nodes.Get(0), nodes.Get(1));
	p2p.Install(nodes.Get(1), nodes.Get(2));

	NodeContainer producerContainer;
	producerContainer.Add(nodes.Get(0));
	producerContainer.Add(nodes.Get(2));

	// Install NDN stack on all nodes
	ndn::StackHelper ndnHelper;
	ndnHelper.SetDefaultRoutes(true);
	ndnHelper.InstallAll();

	// Installing applications
	std::string syncPrefix = "/prefix";

	// Choosing forwarding strategy
//	ndn::StrategyChoiceHelper::InstallAll(syncPrefix, "/localhost/nfd/strategy/best-route");
	ndn::StrategyChoiceHelper::InstallAll(syncPrefix, "/localhost/nfd/strategy/multicast");

	// Installing global routing interface on all nodes
	ndn::GlobalRoutingHelper ndnGlobalRoutingHelper;
	ndnGlobalRoutingHelper.InstallAll();

	// Producer
	ndn::AppHelper producerHelper("FSyncProducerApp");
	producerHelper.SetPrefix(syncPrefix);
	producerHelper.SetAttribute("MaxPublishMessage", UintegerValue(g_numberOfPublishMessages)); // max uint64
	producerHelper.SetAttribute("TotalDataStream", UintegerValue(g_numberOfDataStream));
	//producerHelper.Install(nodes.Get(2)).Start(Seconds(2.0)); // last node


	//producerApp.Install(producerContainer).Start(Seconds(5));
	NodeContainer::Iterator producerIter = producerContainer.Begin();
	for (; producerIter != producerContainer.End(); producerIter++) {
		producerHelper.SetAttribute("UserPrefix", StringValue(stringf("topic%03u", (*producerIter)->GetId())));
		ApplicationContainer container = producerHelper.Install(*producerIter);
		container.Start(Seconds(g_nPStart));
		container.Stop(Seconds(g_simulationTime-5.0));
	}

	// Add /prefix origins to ndn::GlobalRouter
	ndnGlobalRoutingHelper.AddOrigins(syncPrefix, producerContainer);

	// Calculate and install FIBs
	ndn::GlobalRoutingHelper::CalculateRoutes();

	Simulator::Stop(Seconds(g_simulationTime));

	Simulator::Run();
	Simulator::Destroy();

	return 0;
}

} // namespace ns3

int
main(int argc, char* argv[])
{
  return ns3::main(argc, argv);
}
