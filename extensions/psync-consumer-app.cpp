/* -*- Mode:C++; c-file-style:"gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2014-2019,  The University of Memphis
 *
 * This file is part of PSync.
 * See AUTHORS.md for complete list of PSync authors and contributors.
 *
 * PSync is free software: you can redistribute it and/or modify it under the terms
 * of the GNU Lesser General Public License as published by the Free Software Foundation,
 * either version 3 of the License, or (at your option) any later version.
 *
 * PSync is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY;
 * without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR
 * PURPOSE.  See the GNU Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public License along with
 * PSync, e.g., in COPYING.md file.  If not, see <http://www.gnu.org/licenses/>.
 **/

#include <extensions/psync-consumer-app.hpp>

#include <ndn-cxx/name.hpp>
#include <ndn-cxx/util/random.hpp>

#include <iostream>
#include <vector>

#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/ndn-cxx/util/logger.hpp"


using namespace ns3::ndn;

//NDN_LOG_INIT(examples.PartialSyncConsumerApp);
NS_LOG_COMPONENT_DEFINE("PSyncConsumerApp");

NS_OBJECT_ENSURE_REGISTERED(PSyncConsumerApp);

ns3::TypeId
PSyncConsumerApp::GetTypeId() {
  static ns3::TypeId tid = ns3::TypeId("PSyncConsumerApp")
			.SetGroupName("Ndn")
			.SetParent<Application>()
    		.AddConstructor<PSyncConsumerApp>()
            .AddAttribute("Prefix", "Prefix, for which producer has the data", ns3::StringValue("/"),
						  MakeNameAccessor(&PSyncConsumerApp::m_syncPrefix), MakeNameChecker())

		    .AddAttribute("NumSubscribeMessage", "Number of subscribe messages", ns3::UintegerValue(100),
						  ns3::MakeUintegerAccessor(&PSyncConsumerApp::m_nSub), ns3::MakeUintegerChecker<uint32_t>())
					;

  return tid;
}

void
PSyncConsumerApp::DoInitialize()
{
    NS_LOG_FUNCTION_NOARGS();

    // find out what is application id on the node
    for (uint32_t id = 0; id < GetNode()->GetNApplications(); ++id) {
        if (GetNode()->GetApplication(id) == this) {
	        m_appId = id;
        }
    }

    Application::DoInitialize();
}

void PSyncConsumerApp::DoDispose(void)
{
//NS_LOG_FUNCTION_NOARGS();
	NDN_LOG_DEBUG("");

// Unfortunately, this causes SEGFAULT
// The best reason I see is that apps are freed after ndn stack is removed
// StopApplication ();
    Application::DoDispose();
}

uint32_t PSyncConsumerApp::GetId() const
{
    return m_appId;
}

  /**
   * @brief Initialize consumer and start hello process
   *
   * 0.001 is the false positive probability of the bloom filter
   *
   * @param syncPrefix should be the same as producer
   * @param nSub number of subscriptions is used for the bloom filter (subscription list) size
   */
PSyncConsumerApp::PSyncConsumerApp()
    : m_nSub(0)
    , m_rng(ndn::random::getRandomNumberEngine())
    , m_appId(std::numeric_limits<uint32_t>::max())
{
}

void
PSyncConsumerApp::StartApplication() {
  m_instance.reset(new psync::Consumer(m_syncPrefix, m_face,
		  std::bind(&PSyncConsumerApp::afterReceiveHelloData, this, _1),
		  std::bind(&PSyncConsumerApp::processSyncUpdate, this, _1),
		  m_nSub, 0.001));

	// This starts the consumer side by sending a hello interest to the producer
	// When the producer responds with hello data, afterReceiveHelloData is called
  m_instance->sendHelloInterest();

//  m_face.processEvents();
}

void
PSyncConsumerApp::StopApplication() {
	NS_LOG_DEBUG("StopApplication");
	m_instance->stop();
	m_instance.reset();
}

void
PSyncConsumerApp::run()
{
    m_face.processEvents();
}

void
PSyncConsumerApp::afterReceiveHelloData(const std::vector<::ndn::Name>& availSubs)
{
    // Randomly subscribe to m_nSub prefixes
    std::vector<::ndn::Name> sensors = availSubs;

    std::shuffle(sensors.begin(), sensors.end(), m_rng);

    for (uint32_t i = 0; i < m_nSub; i++) {
        NDN_LOG_INFO("Subscribing to: " << sensors[i]);
        m_instance->addSubscription(sensors[i]);
    }

    // After setting the subscription list, send the sync interest
    // The sync interest contains the subscription list
    // When new data is received for any subscribed prefix, processSyncUpdate is called
    m_instance->sendSyncInterest();
}

void
PSyncConsumerApp::processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates)
{
    for (const auto& update : updates) {
        for (uint64_t i = update.lowSeq; i <= update.highSeq; i++) {
            // Data can now be fetched using the prefix and sequence
            NDN_LOG_INFO("PUpdate: " << update.prefix << "/" << i);
        }
    }
}

#if 0
int
main(int argc, char* argv[])
{
  if (argc != 3) {
    std::cout << "usage: " << argv[0] << " "
              << "<sync-prefix> <number-of-subscriptions>" << std::endl;
    return 1;
  }

  try {
    PSyncConsumerApp consumer(argv[1], std::stoi(argv[2]));
    consumer.run();
  }
  catch (const std::exception& e) {
    NDN_LOG_ERROR(e.what());
  }
}

#endif
