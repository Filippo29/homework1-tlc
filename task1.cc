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
#define NCSMA1 2 // e c'è da aggiungere n4 che è stato creato con la stella
#define NCSMA2 2 // e c'è da aggiungere n7 che è stato creato con il link tra le 2 csma p2pNodes.Get(0)

using namespace ns3;

void printIP(PointToPointStarHelper star, Ipv4InterfaceContainer csma1Interfaces, Ipv4InterfaceContainer p2pInterfaces, Ipv4InterfaceContainer csma2Interfaces){
    printf("STAR:\nhub:\n");
    star.GetHubIpv4Address(0).Print(std::cout);
    printf("\nnodes:\n");
    for(int i = 0; i < NSPOKES; i++){
        star.GetSpokeIpv4Address(i).Print(std::cout);
        printf("\n");
    }

    printf("\nCSMA1:\n");
    for(int i = 0; i < NCSMA1+1; i++){
        csma1Interfaces.Get(i).first->GetAddress(1,0).GetLocal().Print(std::cout);
        printf("\n");
    }

    printf("\nLink n6-n7:\n");
    for(int i = 0; i < 2; i++){
        p2pInterfaces.Get(i).first->GetAddress(1,0).GetLocal().Print(std::cout);
        printf("\n");
    }

    printf("\nCSMA2:\n");
    for(int i = 0; i < NCSMA2+1; i++){
        csma2Interfaces.Get(i).first->GetAddress(1,0).GetLocal().Print(std::cout);
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

    //creazione link tra n6 e n7
    NodeContainer p2pNodes;
    p2pNodes.Create(1);

    PointToPointHelper pointToPoint2;
    pointToPoint2.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    pointToPoint2.SetChannelAttribute("Delay", StringValue("10us"));
    //fine creazione link tra n6 e n7

    //creazione csma2
    NodeContainer csma2Nodes;
    csma2Nodes.Create(NCSMA2);

    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("30Mbps"));
    csma2.SetChannelAttribute("Delay", TimeValue(NanoSeconds(20)));
    //fine csma2
	
    //install internet stack
    InternetStackHelper internet;
    star.InstallStack(internet);

    internet.Install(csma1Nodes);
    csma1Nodes.Add(star.GetSpokeNode(3)); //csma1Nodes.Get(2) è n4
    
    internet.Install(p2pNodes);
    p2pNodes.Add(csma1Nodes.Get(1)); //csma1Nodes.Get(1) è n6

    internet.Install(csma2Nodes);
    csma2Nodes.Add(p2pNodes.Get(0)); //p2pNodes.Get(0) è n7
    //fine install internet stack

    //assegnazione ipv4
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.0.1.0", "255.255.255.240"));

    NetDeviceContainer csmaDevices = csma1.Install(csma1Nodes);
    Ipv4AddressHelper address;
    address.SetBase("192.118.1.0", "255.255.255.0");
    Ipv4InterfaceContainer csma1Interfaces = address.Assign(csmaDevices);

    NetDeviceContainer p2pDevices = pointToPoint2.Install(p2pNodes);
    address.SetBase("10.0.2.0", "255.255.255.252");
    Ipv4InterfaceContainer p2pInterfaces = address.Assign(p2pDevices);

    NetDeviceContainer csma2Devices = csma2.Install(csma2Nodes);
    address.SetBase("192.118.2.0", "255.255.255.0");
    Ipv4InterfaceContainer csma2Interfaces = address.Assign(csma2Devices);
    //fine assegnazione ipv4

    printIP(star, csma1Interfaces, p2pInterfaces, csma2Interfaces);

    //abilitazione pcap
    //per abilitare pcap singoli: pointtopoint.EnablePcap("nomefile", node->GetId(), 0);
	pointToPoint.EnablePcapAll("pcapfold/task1"); //cambiare prima di consegnare
    //fine pcap

    Simulator::Run();
    Simulator::Destroy();
    
	return 0;
}
