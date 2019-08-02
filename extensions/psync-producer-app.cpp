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

#include <extensions/psync-producer-app.hpp>

#include <ndn-cxx/util/random.hpp>

#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/ndn-cxx/util/logger.hpp"

using namespace ns3::ndn;

//NDN_LOG_INIT(examples.PartialSyncProducerApp);
NS_LOG_COMPONENT_DEFINE("PSyncProducerApp");

NS_OBJECT_ENSURE_REGISTERED(PSyncProducerApp);

ns3::TypeId
PSyncProducerApp::GetTypeId() {
  static ns3::TypeId tid = ns3::TypeId("PSyncProducerApp")
			.SetGroupName("Ndn")
			.SetParent<Application>()
			.AddConstructor<PSyncProducerApp>()
			.AddAttribute("Prefix", "Prefix, for which producer has the data", ns3::StringValue("/"),
						   MakeNameAccessor(&PSyncProducerApp::m_syncPrefix), MakeNameChecker())

			.AddAttribute("UserPrefix", "Prefix, for which producer has the data", ns3::StringValue("/"),
						   MakeStringAccessor(&PSyncProducerApp::m_userPrefix), ns3::MakeStringChecker())

			.AddAttribute("MaxPublishMessage", "Number of publish messages", ns3::UintegerValue(100),
						   ns3::MakeUintegerAccessor(&PSyncProducerApp::m_maxNumPublish), ns3::MakeUintegerChecker<uint64_t>())

			.AddAttribute("TotalDataStream", "Number of Data Stream", ns3::UintegerValue(200),
						   ns3::MakeUintegerAccessor(&PSyncProducerApp::m_nTotalDS), ns3::MakeUintegerChecker<uint32_t>())
					;

  return tid;
}

void
PSyncProducerApp::DoInitialize()
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

void PSyncProducerApp::DoDispose(void)
{
//NS_LOG_FUNCTION_NOARGS();
	NDN_LOG_DEBUG("");

// Unfortunately, this causes SEGFAULT
// The best reason I see is that apps are freed after ndn stack is removed
// StopApplication ();
Application::DoDispose();
}

uint32_t PSyncProducerApp::GetId() const
{
return m_appId;
}

  /**
   * @brief Initialize producer and schedule updates
   *
   * IBF size is set to 40 in m_producer as the expected number of update to IBF in a sync cycle
   */
PSyncProducerApp::PSyncProducerApp()
    : m_scheduler(m_face.getIoService())
	, m_nTotalDS(200)
	, m_maxNumPublish(std::numeric_limits<uint64_t>::max())
	, m_appId(std::numeric_limits<uint32_t>::max())
	, m_rng(::ndn::random::getRandomNumberEngine())
	, m_rangeUniformRandom(0, 60000)
  {
  }

void
PSyncProducerApp::StartApplication() {

//  m_instance.reset(new psync::PartialProducer(40, m_face, m_syncPrefix, m_userPrefix + "-0"));
  m_instance.reset(new psync::PartialProducer(m_nTotalDS, m_face, m_syncPrefix, m_userPrefix + "-0"));

  // Add user prefixes and schedule updates for them
  for (uint32_t i = 0; i < m_nTotalDS; i++) {
    ndn::Name updateName(m_userPrefix + "-" + ndn::to_string(i));

    // Add the user prefix to the producer
    // Note that this does not add the already added userPrefix-0 in the constructor
    m_instance->addUserNode(updateName);

    // Each user prefix is updated at random interval between 0 and 60 second
    m_scheduler.scheduleEvent(ndn::time::milliseconds(m_rangeUniformRandom(m_rng)),
                              [this, updateName] {
                                doUpdate(updateName);
                              });
  }
}

void
PSyncProducerApp::StopApplication() {

	NDN_LOG_INFO("StopApplication");
	m_scheduler.cancelAllEvents();
	m_instance.reset();


	// print counters
}

void
PSyncProducerApp::run()
{
  m_face.processEvents();
}

void
PSyncProducerApp::doUpdate(const ndn::Name& updateName)
{
  // Publish an update to this user prefix
  m_instance->publishName(updateName);

  uint64_t seqNo =  m_instance->getSeqNo(updateName).value();
  NDN_LOG_INFO("Publish: " << updateName << "/" << seqNo);

  if (seqNo < m_maxNumPublish) {
    // Schedule the next update for this user prefix b/w 0 and 60 seconds
	 m_scheduler.scheduleEvent(ndn::time::milliseconds(m_rangeUniformRandom(m_rng)),
                              [this, updateName] {
                                doUpdate(updateName);
                              });
  }
}

#if 0
int
main(int argc, char* argv[])
{
  if (argc != 5) {
    std::cout << "usage: " << argv[0] << " <sync-prefix> <user-prefix> "
              << "<number-of-user-prefixes> <max-number-of-updates-per-user-prefix>"
              << std::endl;
    return 1;
  }

  try {
    PSyncProducerApp producer(argv[1], argv[2], std::stoi(argv[3]), std::stoi(argv[4]));
    producer.run();
  }
  catch (const std::exception& e) {
    NDN_LOG_ERROR(e.what());
  }
}
#endif

