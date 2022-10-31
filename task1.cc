#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"

#include <inttypes.h>
#include <stdio.h>

#define NSPOKES 4
#define NCSMA1 2

using namespace ns3;

void printIP(PointToPointStarHelper star, Ipv4InterfaceContainer csma1Interfaces){
    printf("STAR:\nhub:\n");
    star.GetHubIpv4Address(0).Print(std::cout);
    printf("\nnodes:\n");
    for(int i = 0; i < NSPOKES; i++){
        star.GetSpokeIpv4Address(i).Print(std::cout);
        printf("\n");
    }

    printf("CSMA1:\n");
    for(int i = 0; i < NCSMA1+1; i++){
        csma1Interfaces.Get(i).first->GetAddress(1,0).GetLocal().Print(std::cout);
        printf("\n");
    }
}

int main(int argc, char* argv[]){
    //creazione stella formata da n0, n1, n2, n3, n4
	PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10us"));
    PointToPointStarHelper star(NSPOKES, pointToPoint);
    //fine stella

    //creazione csma1
    NodeContainer csma1Nodes;
    csma1Nodes.Create(NCSMA1);

    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("25Mbps"));
    csma1.SetChannelAttribute("Delay", TimeValue(NanoSeconds(10)));

    //fine csma1
	
    //install internet stack
    InternetStackHelper internet;
    star.InstallStack(internet);

    //internet.Install(star.GetSpokeNode(3));
    internet.Install(csma1Nodes);
    csma1Nodes.Add(star.GetSpokeNode(3)); //csma1Nodes.Get(2) è n4
    //fine install internet stack

    //assegnazione ipv4
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.0.1.0", "255.255.255.240"));

    NetDeviceContainer csmaDevices;
    csmaDevices = csma1.Install(csma1Nodes);
    Ipv4AddressHelper address;
    address.SetBase("192.118.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csma1Interfaces;
    csma1Interfaces = address.Assign(csmaDevices);
    //fine assegnazione ipv41

    printIP(star, csma1Interfaces);

    //abilitazione pcap
    //per abilitare pcap singoli: pointtopoint.EnablePcap("nomefile", node->GetId(), 0);
	pointToPoint.EnablePcapAll("pcapfold/task1"); //cambiare prima di consegnare
    //fine pcap

    Simulator::Run();
    Simulator::Destroy();
    
	return 0;
}
