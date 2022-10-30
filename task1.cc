#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"

#include <stdio.h>

#define NSPOKES 4
#define NCSMA1 2

using namespace ns3;

int main(int argc, char* argv[]){
    //creazione stella formata da n0, n1, n2, n3, n4
	PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10us"));
    PointToPointStarHelper star(NSPOKES, pointToPoint);
    //fine stella
	
    //install internet stack
    InternetStackHelper internet;
    star.InstallStack(internet);
    //fine install internet stack

    //assegnazione ipv4
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.0.1.0", "255.255.255.0"));
    //fine assegnazione ipv4

    star.GetHubIpv4Address(0).Print(std::cout);
    star.GetSpokeIpv4Address(0).Print(std::cout);

    //abilitazione pcap
    //per abilitare pcap singoli: pointtopoint.EnablePcap("nomefile", node->GetId(), 0);
	pointToPoint.EnablePcapAll("pcapfold/task1"); //cambiare prima di consegnare
    //fine pcap

    Simulator::Run();
    Simulator::Destroy();
    
	return 0;
}
