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

#ifndef NDN_FULL_SYNC_PRODUCER_APP_H
#define NDN_FULL_SYNC_PRODUCER_APP_H

#include <PSync/full-producer.hpp>

#include <ndn-cxx/name.hpp>

//#include <ndn-cxx/util/logger.hpp>
#include <ndn-cxx/util/random.hpp>

#include <iostream>

#include "ns3/application.h"

#include "ns3/string.h"
#include "ns3/uinteger.h"

#include "ns3/ndnSIM/model/ndn-common.hpp"
#include "ns3/ndnSIM/ndn-cxx/util/logger.hpp"

using namespace ns3::ndn;

using namespace ::ndn::time_literals;

class FSyncProducerApp : public ns3::Application
{
public:
	static ns3::TypeId
	GetTypeId();

	void
	DoInitialize();

	void
	DoDispose(void);

	uint32_t
	GetId() const;
  /**
   * @brief Initialize producer and schedule updates
   *
   * Set IBF size as 80 expecting 80 updates to IBF in a sync cycle
   * Set syncInterestLifetime and syncReplyFreshness to 1.6 seconds
   * userPrefix is the default user prefix, no updates are published on it in this example
   */
	FSyncProducerApp();

	virtual void
	StartApplication();

	virtual void
	StopApplication();

	void
	run();

private:
	void
	doUpdate(const Name& prefix);

	void
	processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates);

private:
	Name m_syncPrefix;
	std::string m_userPrefix;
	::ndn::Face m_face;
	::ndn::util::Scheduler m_scheduler;

	std::unique_ptr<psync::FullProducer> m_instance;

	uint32_t m_nSub;
	uint64_t m_maxNumPublish;
	uint32_t m_nTotalDS;
	uint32_t m_appId;

	::ndn::random::RandomNumberEngine& m_rng;
	std::uniform_int_distribution<> m_rangeUniformRandom;
};

#endif
