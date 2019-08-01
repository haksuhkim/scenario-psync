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

#ifndef NDN_PSYNCPRODUCER_APP_H
#define NDN_PSYNCPRODUCER_APP_H

#include <ns3/ndnSIM/ndn-cxx/name.hpp>
#include <ns3/ndnSIM/ndn-cxx/util/random.hpp>

#include <PSync/partial-producer.hpp>

#include <string>
#include <memory>

#include "ns3/application.h"

class PSyncProducerApp : public ns3::Application
{
public:
	  static ns3::TypeId
	  GetTypeId();

	  void
	  DoInitialize();

	  void DoDispose(void);

	  uint32_t GetId() const;
  /**
   * @brief Initialize producer and schedule updates
   *
   * IBF size is set to 40 in m_producer as the expected number of update to IBF in a sync cycle
   */
  PSyncProducerApp();

  virtual void
  StartApplication();

  virtual void
  StopApplication();

  void
  run();

private:
  void
  doUpdate(const ::ndn::Name& updateName);

private:
  ::ndn::Name m_syncPrefix;
  std::string m_userPrefix;
  ::ndn::Face m_face;
  ::ndn::util::Scheduler m_scheduler;

  std::unique_ptr<psync::PartialProducer> m_instance;

  uint32_t m_nTotalDS;
  uint64_t m_maxNumPublish;
  uint32_t m_appId;

  ::ndn::random::RandomNumberEngine& m_rng;
  std::uniform_int_distribution<int> m_rangeUniformRandom;
};
#endif
