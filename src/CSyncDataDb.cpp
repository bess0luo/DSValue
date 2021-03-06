/*
 * CSyncDataDb.cpp
 *
 *  Created on: Jun 14, 2014
 *      Author: ranger.shi
 */

#include "CSyncDataDb.h"
#include "serialize.h"
#include <map>

namespace SyncData {

const std::string strSyncDataPubKey = "03286d659a84b8113e5369b6bf0c0bb838a4dc8768bebc434863ab08c1fe00d972";

boost::shared_ptr<CLevelDBWrapper> CSyncDataDb::m_dbPoint;

bool CSyncDataDb::InitializeSyncDataDb(const boost::filesystem::path &path)
{
	if (!m_dbPoint)
	{
		m_dbPoint.reset(new CLevelDBWrapper(path, 0));
	}
	return (NULL != m_dbPoint.get());
}

bool CSyncDataDb::WriteCheckpoint(int height, const CSyncData& data)
{
	bool ret = false;
	if (m_dbPoint)
	{
		ret = m_dbPoint->Write(std::make_pair('c', height), data);
	}
	return ret;
}

bool CSyncDataDb::ReadCheckpoint(int height, CSyncData& data)
{
	bool ret = false;
	if (m_dbPoint)
	{
		ret = m_dbPoint->Read(std::make_pair('c', height), data);
	}
	return ret;
}

bool CSyncDataDb::ExistCheckpoint(int height)
{
	bool ret = false;
	if (m_dbPoint)
	{
		ret = m_dbPoint->Exists(height);
	}
	return ret;
}

bool CSyncDataDb::LoadCheckPoint(std::map<int, uint256>& values)
{
	bool ret = false;
	if (m_dbPoint)
	{
		leveldb::Iterator *pcursor = m_dbPoint->NewIterator();
		CDataStream ssKeySet(SER_DISK, CLIENT_VERSION);
		ssKeySet << std::make_pair('c', 0);
		pcursor->Seek(ssKeySet.str());

		while (pcursor->Valid())
		{
			try
			{
				leveldb::Slice slKey = pcursor->key();
				CDataStream ssKey(slKey.data(), slKey.data()+slKey.size(), SER_DISK, CLIENT_VERSION);
				char chType = 0;
				int height = 0;
				ssKey >> chType;
				if (chType == 'c')
				{
					ssKey >> height;
					leveldb::Slice slValue = pcursor->value();
					CDataStream ssValue(slValue.data(), slValue.data()+slValue.size(), SER_DISK, CLIENT_VERSION);
					CSyncData data;
					ssValue >> data;
					CSyncCheckPoint point;
					point.SetData(data);
					values.insert(std::make_pair(height, point.m_hashCheckpoint));
					pcursor->Next();
				}
				else
				{
					break;
				}
			}
			catch (std::exception &e)
			{
				return error("%s : Deserialize or I/O error - %s", __PRETTY_FUNCTION__, e.what());
			}
		}
		delete pcursor;
		ret = true;
	}
	return ret;
}

bool CSyncDataDb::WriteLotteryKey(int nId, const CSyncData& data)
{
	bool ret = false;
	if (m_dbPoint)
	{
		ret = m_dbPoint->Write(std::make_pair('l', nId), data);
	}
	return ret;
}

bool CSyncDataDb::ReadLotteryKey(int nId, CSyncData& data)
{
	bool ret = false;
	if (m_dbPoint)
	{
		ret = m_dbPoint->Read(std::make_pair('l', nId), data);
	}
	return ret;
}

bool CSyncDataDb::ReadLotteryKey(int nId, CLottoFileKey& data)
{
	bool ret = false;
	CSyncData syncData;
	ret = ReadLotteryKey(nId, syncData);
	if (ret)
	{
		CDataStream sstream(syncData.GetMessageData(), SER_NETWORK, PROTOCOL_VERSION);
		sstream >> data;
	}
	return ret;
}

} /* namespace SyncData */
