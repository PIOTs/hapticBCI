#pragma once
#include "c:\users\charm_stars\documents\darreldeo\neurotouch2\oscpack\osc\oscpacketlistener.h"
class OSC_Listener :
	public osc::OscPacketListener
{
public:
	OSC_Listener(void);
	void queryEmoState(float& Cog_Right, float& Cog_Left, float& Cog_Neutral);
	~OSC_Listener(void);

protected:
	 virtual void ProcessMessage( const osc::ReceivedMessage& m, 
				const IpEndpointName& remoteEndpoint );
};

