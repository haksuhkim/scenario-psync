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

#include <extensions/fsync-producer-app.hpp>
#include <PSync/full-producer.hpp>

#include <ndn-cxx/name.hpp>

//#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/random.hpp>

#include <iostream>

#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/ndn-cxx/util/logger.hpp"


using namespace ns3::ndn;

using namespace ::ndn::time_literals;

//NDN_LOG_INIT(examples.FullSyncApp);
NS_LOG_COMPONENT_DEFINE("FSyncProducerApp");

NS_OBJECT_ENSURE_REGISTERED(FSyncProducerApp);

ns3::TypeId
FSyncProducerApp::GetTypeId() {
	static ns3::TypeId tid = ns3::TypeId("FSyncProducerApp")
						.SetGroupName("Ndn")
						.SetParent<ns3::Application>()
						.AddConstructor<FSyncProducerApp>()
						.AddAttribute("Prefix", "Prefix, for which producer has the data", ns3::StringValue("/"),
										MakeNameAccessor(&FSyncProducerApp::m_syncPrefix), MakeNameChecker())

						.AddAttribute("UserPrefix", "Prefix, for which producer has the data", ns3::StringValue("/"),
										ns3::MakeStringAccessor(&FSyncProducerApp::m_userPrefix), ns3::MakeStringChecker())

						.AddAttribute("MaxPublishMessage", "Number of publish messages", ns3::UintegerValue(100),
										ns3::MakeUintegerAccessor(&FSyncProducerApp::m_maxNumPublish), ns3::MakeUintegerChecker<uint64_t>())

						.AddAttribute("TotalDataStream", "Number of Data Stream", ns3::UintegerValue(200),
										ns3::MakeUintegerAccessor(&FSyncProducerApp::m_nTotalDS), ns3::MakeUintegerChecker<uint32_t>())
						;

	return tid;
}

void
FSyncProducerApp::DoInitialize() {
	NS_LOG_FUNCTION_NOARGS();

	// find out what is application id on the node
	for (uint32_t id = 0; id < GetNode()->GetNApplications(); ++id) {
		if (GetNode()->GetApplication(id) == this) {
			m_appId = id;
		}
	}

	Application::DoInitialize();
}

void
FSyncProducerApp::DoDispose(void) {
	NS_LOG_FUNCTION_NOARGS();

	// Unfortunately, this causes SEGFAULT
	// The best reason I see is that apps are freed after ndn stack is removed
	// StopApplication ();
	Application::DoDispose();
}

uint32_t
FSyncProducerApp::GetId() const {
	return m_appId;
}
  /**
   * @brief Initialize producer and schedule updates
   *
   * Set IBF size as 80 expecting 80 updates to IBF in a sync cycle
   * Set syncInterestLifetime and syncReplyFreshness to 1.6 seconds
   * userPrefix is the default user prefix, no updates are published on it in this example
   */
FSyncProducerApp::FSyncProducerApp()
	: m_scheduler(m_face.getIoService())
	, m_nSub(200)
	, m_maxNumPublish(std::numeric_limits<uint64_t>::max())
	, m_nTotalDS(200)
	, m_appId(std::numeric_limits<uint32_t>::max())
	, m_rng(ndn::random::getRandomNumberEngine())
	, m_rangeUniformRandom(0, 60000)
  {
  }

void
FSyncProducerApp::StartApplication() {
//	m_instance.reset(new psync::FullProducer(80, m_face, m_syncPrefix, m_userPrefix,
//					std::bind(&FSyncProducerApp::processSyncUpdate, this, _1),
//					1600_ms, 1600_ms));
	m_instance.reset(new psync::FullProducer(m_nTotalDS, m_face, m_syncPrefix, m_userPrefix,
					std::bind(&FSyncProducerApp::processSyncUpdate, this, _1),
					1600_ms, 1600_ms));

	// Add user prefixes and schedule updates for them in specified interval
	for (uint32_t i = 0; i < m_nTotalDS; i++) {
		::ndn::Name prefix(m_userPrefix + "-" + ndn::to_string(i));

		m_instance->addUserNode(prefix);

		m_scheduler.scheduleEvent(ndn::time::milliseconds(m_rangeUniformRandom(m_rng)),
										[this, prefix] {
											doUpdate(prefix);
										});
	}
}

void
FSyncProducerApp::StopApplication() {
	NDN_LOG_INFO("StopApplication");
	m_scheduler.cancelAllEvents();
	m_instance.reset();
}

void
FSyncProducerApp::run() {
	m_face.processEvents();
}

void
FSyncProducerApp::doUpdate(const Name& prefix) {
	m_instance->publishName(prefix);

	uint64_t seqNo = m_instance->getSeqNo(prefix).value();
	NDN_LOG_INFO("Publish: " << prefix << "/" << seqNo);

	if (seqNo < m_maxNumPublish) {
		m_scheduler.scheduleEvent(ndn::time::milliseconds(m_rangeUniformRandom(m_rng)),
									[this, prefix] {
										doUpdate(prefix);
									});
	}
}

void
FSyncProducerApp::processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates) {
	for (const auto& update : updates) {
		for (uint64_t i = update.lowSeq; i <= update.highSeq; i++) {
			NDN_LOG_INFO("Update " << update.prefix << "/" << i);
		}
	}
}

#if 0
int
main(int argc, char* argv[])
{
  if (argc != 5) {
    std::cout << "usage: " << argv[0] << " <syncPrefix> <user-prefix> "
              << "<number-of-user-prefixes> <max-number-of-updates-per-user-prefix>"
              << std::endl;
    return 1;
  }

  try {
	  FSyncProducerApp producer(argv[1], argv[2], std::stoi(argv[3]), std::stoi(argv[4]));
    producer.run();
  }
  catch (const std::exception& e) {
    NDN_LOG_ERROR(e.what());
  }
}

#endif
