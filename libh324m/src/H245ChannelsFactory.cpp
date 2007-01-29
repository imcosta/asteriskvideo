/* H324M library
 *
 * Copyright (C) 2006 Sergio Garcia Murillo
 *
 * sergio.garcia@fontventa.com
 * http://sip.fontventa.com
 * 
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * as published by the Free Software Foundation; either version 2
 * of the License, or (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.
 */
#include "H245ChannelsFactory.h"


H245ChannelsFactory::H245ChannelsFactory()
{
	//Set local capabilities only with layer 2
	local.audioWithAL2 = true;
	local.videoWithAL2 = true;

	//No media channels
	numChannels = 0;
}

H245ChannelsFactory::~H245ChannelsFactory()
{
}

int H245ChannelsFactory::Init(H223ALSender* controlSender,H223ALReceiver* controlReceiver)
{
	//Set local table with control channel
	localTable.SetEntry(0,"","0");
	
	//Set remote table with control channel
	remoteTable.SetEntry(0,"","0");

	//Set control channel
	demuxer.SetChannel(0,controlReceiver);
	muxer.SetChannel(0,controlSender);

	//Open demuxer
	demuxer.Open(&localTable);

	//Open muxer
	muxer.Open(&remoteTable);

	//OK
	return 1;
}

int H245ChannelsFactory::End()
{
	return 1;
}

int H245ChannelsFactory::Demultiplex(BYTE *buffer,int length)
{
	//DeMux
	for (int i=0;i<length;i++)
		demuxer.Demultiplex(buffer[i]);
	//Ok
	return 1;
}

int H245ChannelsFactory::Multiplex(BYTE *buffer,int length)
{
	//Mux
	for (int i=0;i<length;i++)
		buffer[i] = muxer.Multiplex();

	//Ok
	return 1;
}

int H245ChannelsFactory::CreateChannel(H324MMediaChannel::Type type)
{
	H324MMediaChannel* chan;

	//Dependin on channel type
	switch(type)
	{
		case H324MMediaChannel::e_Audio:
			//New audio channel
			chan = new H324MAudioChannel();
			break;
		case H324MMediaChannel::e_Video:
			//New audio channel
			chan = new H324MVideoChannel();
			break;
		default:
			return -1;
	}

	//Append to map
	channels[++numChannels] = chan;

	//Set  logical channel
	chan->localChannel = numChannels;

	//Create rep part of entry
	char rep[4];
	sprintf(rep,"%d",numChannels);
	//Add to local table
	localTable.SetEntry(numChannels,"",rep);

	//return channel id
	return numChannels;
}

H223ALSender* H245ChannelsFactory::GetSender(int id)
{
	//If it exist
	if (channels.find(id)==channels.end())
		return NULL;
	//Return sender
	return channels[id]->GetSender();
}

H223ALReceiver* H245ChannelsFactory::GetReceiver(int id)
{
	//If it exist
	if (channels.find(id)==channels.end())
		return NULL;
	//Return receiver
	return channels[id]->GetReceiver();
}

H223MuxTable* H245ChannelsFactory::GetLocalTable()
{
	return &localTable;
}

H223MuxTable* H245ChannelsFactory::GetRemoteTable()
{
	return &remoteTable;
}

int H245ChannelsFactory::SetRemoteTable(H223MuxTable* table)
{
	return true;
}


H245Capabilities* H245ChannelsFactory::GetLocalCapabilities()
{
	return &local;
}
H245Capabilities* H245ChannelsFactory::GetRemoteCapabilities()
{
	return &remote;
}

int H245ChannelsFactory::SetRemoteCapabilities(H245Capabilities* remoteCapabilities)
{
	//Save capabilites
	remote.audioWithAL1 = remoteCapabilities->audioWithAL1;
	remote.audioWithAL2 = remoteCapabilities->audioWithAL2;
	remote.audioWithAL3 = remoteCapabilities->audioWithAL3;
	remote.audioWithAL1 = remoteCapabilities->audioWithAL1;
	remote.audioWithAL2 = remoteCapabilities->audioWithAL2;
	remote.audioWithAL3 = remoteCapabilities->audioWithAL2;
	remote.h263Cap	= remoteCapabilities->h263Cap;
	remote.amrCap	= remoteCapabilities->amrCap;
	remote.g723Cap	= remoteCapabilities->g723Cap;
	remote.inputCap	= remoteCapabilities->inputCap;

	//Set layers for channels
	for(ChannelMap::iterator it = channels.begin(); it != channels.end(); it++)
	{
		//Get channel
		H324MMediaChannel *chan = it->second;
		
		//Here we should set an H245Channel to a H324MChannel
	}
	
	return 1;
}


int H245ChannelsFactory::OnEstablishIndication(int number, H245Channel *channel)
{
	int local = -1;

	//Search local channel for same media type
	ChannelMap::iterator it = channels.begin();

	//While not found
	while (it!=channels.end())
	{
		if (it->second->type == H324MMediaChannel::e_Audio && channel->GetType() == H245Channel::e_Audio)
		{
			//We found it 
			local = it->first;
			//Exit
			break;
		} else if (it->second->type == H324MMediaChannel::e_Audio && channel->GetType() == H245Channel::e_Audio) {
			//We found it 
			local = it->first;
			//Exit
			break;
		}
		//Next channel
		it++;
	}

	//If not found
	if (local==-1)
		//Reject channel
		return 0;

	//Get channel
	H324MMediaChannel * chan = it->second;

	//Asign remote channel
	chan->remoteChannel = number;

	//Set receiving layer
	chan->SetReceiverLayer(2);

	//Append to demuxer && accept
	return demuxer.SetChannel(number,chan->GetReceiver());
}

int H245ChannelsFactory::OnEstablishConfirm(int number)
{
	//Search channel 
	ChannelMap::iterator it = channels.find(number);

	//If not found
	if (it==channels.end())
		//exit
		return 0;

	//Get channel
	H324MMediaChannel * chan = it->second;

	//Set sender layer
	chan->SetSenderLayer(2);

	//Set muxer
	return muxer.SetChannel(number,chan->GetSender());
}