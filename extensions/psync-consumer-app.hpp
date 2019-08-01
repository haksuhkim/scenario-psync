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

#ifndef NDN_PSYNCCONSUMER_APP_H
#define NDN_PSYNCCONSUMER_APP_H

#include <ns3/ndnSIM/ndn-cxx/name.hpp>
#include <ns3/ndnSIM/ndn-cxx/util/random.hpp>

#include <PSync/consumer.hpp>

#include <memory>

#include "ns3/application.h"

class PSyncConsumerApp : public ns3::Application
{
public:
	  static ns3::TypeId
	  GetTypeId();

	  void
	  DoInitialize();

	  void DoDispose(void);

	  uint32_t GetId() const;

  /**
   * @brief Initialize consumer and start hello process
   *
   * 0.001 is the false positive probability of the bloom filter
   *
   * @param syncPrefix should be the same as producer
   * @param nSub number of subscriptions is used for the bloom filter (subscription list) size
   */
  PSyncConsumerApp();

  virtual void
  StartApplication();

  virtual void
  StopApplication();

  void
  run();

private:
  void
  afterReceiveHelloData(const std::vector<::ndn::Name>& availSubs);

  void
  processSyncUpdate(const std::vector<psync::MissingDataInfo>& updates);

private:
  ::ndn::Name m_syncPrefix;
  ::ndn::Face m_face;
  uint32_t m_nSub;

  std::unique_ptr<psync::Consumer> m_instance;
  ::ndn::random::RandomNumberEngine& m_rng;
  uint32_t m_appId;
};

#endif
