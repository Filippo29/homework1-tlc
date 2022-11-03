#include "ns3/applications-module.h"
#include "ns3/core-module.h"
#include "ns3/csma-module.h"
#include "ns3/internet-module.h"
#include "ns3/netanim-module.h"
#include "ns3/network-module.h"
#include "ns3/point-to-point-layout-module.h"
#include "ns3/point-to-point-module.h"
#include <iostream>
#include <inttypes.h>
#include <stdio.h>

#define NSPOKES 4
#define NCSMA1 2

using namespace ns3;
NS_LOG_COMPONENT_DEFINE("Task_1_Team_36");

void printIP(PointToPointStarHelper star, Ipv4InterfaceContainer csma1Interfaces, Ipv4InterfaceContainer p2p, Ipv4InterfaceContainer csma2){
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
    
    printf("P2P:\n");
    p2p.GetAddress(0).Print(std::cout);
    printf("\n");
    p2p.GetAddress(1).Print(std::cout);
    printf("\n");

    printf("CSMA2:\n");
    for(int i = 0; i < NCSMA1+1; i++){
        csma2.Get(i).first->GetAddress(1,0).GetLocal().Print(std::cout);
        printf("\n");
    }
}

int main(int argc, char* argv[]){

    //switch per configuration
    int configuration = 0;
    CommandLine cmd;
    cmd.AddValue("configuration", "Configuration number to be set: 0 - 1 - 2", configuration);
    cmd.Parse(argc, argv);
    //fine configuration

    //creazione stella formata da n0, n1, n2, n3, n4
	PointToPointHelper pointToPoint;
    pointToPoint.SetDeviceAttribute("DataRate", StringValue("80Mbps"));
    pointToPoint.SetChannelAttribute("Delay", StringValue("10us"));
    PointToPointStarHelper star(NSPOKES, pointToPoint);
    //fine stella

    //creazione csma-link 1
    NodeContainer csma1Nodes;
    csma1Nodes.Add(star.GetSpokeNode(3));
    csma1Nodes.Create(NCSMA1);
    
    CsmaHelper csma1;
    csma1.SetChannelAttribute("DataRate", StringValue("25Mbps"));
    csma1.SetChannelAttribute("Delay", StringValue("10us"));

    //install NetDeviceContainer csma1
    NetDeviceContainer csma1Devices;
    csma1Devices = csma1.Install(csma1Nodes);
    
    //fine csma1

    //node container per i 2 nodi p2p tra le 2 csma
    NodeContainer p2pNodes;
    p2pNodes.Add(csma1Nodes.Get(2));
    p2pNodes.Create(1);

    //creazione csma-link2
    NodeContainer csma2Nodes;
    csma2Nodes.Add(p2pNodes.Get(1));
    csma2Nodes.Create(NCSMA1);
    
 
 
    //setup point2point n6-n7
    PointToPointHelper pointToPoint2;
    pointToPoint2.SetDeviceAttribute ("DataRate", StringValue ("80Mbps"));
    pointToPoint2.SetChannelAttribute ("Delay", StringValue("10us"));

    NetDeviceContainer p2pDevices;
    p2pDevices = pointToPoint2.Install(p2pNodes);

    //setup csma2
    CsmaHelper csma2;
    csma2.SetChannelAttribute("DataRate", StringValue("30Mbps"));
    csma2.SetChannelAttribute("Delay", StringValue("20us"));

    //NetDeviceContainer per csma2 link
    NetDeviceContainer csma2Devices;
    csma2Devices = csma2.Install(csma2Nodes);

    //fine setup 


    //install internet stack
    InternetStackHelper internet;
    star.InstallStack(internet);
    internet.Install(csma2Nodes);
    internet.Install(csma1Nodes.Get(1));
    internet.Install(csma1Nodes.Get(2));
    //fine install internet stack


    //assegnazione ipv4 CSMA1
    Ipv4AddressHelper ipv4Csma1;
    ipv4Csma1.SetBase ("192.118.1.0", "/24", "0.0.0.1");
    Ipv4InterfaceContainer csma1Ic = ipv4Csma1.Assign (csma1Devices);

    //assegnazione ipv4 CSMA2
    Ipv4AddressHelper ipv4Csma2;
    ipv4Csma2.SetBase ("192.118.2.0", "/24", "0.0.0.1");
    Ipv4InterfaceContainer csma2Ic = ipv4Csma2.Assign (csma2Devices);
  
    //assegnazione ipv4 STAR
    star.AssignIpv4Addresses(Ipv4AddressHelper("10.0.1.0", "255.255.255.240"));

    //assegnazione ipv4 P2P n6-n7
    Ipv4AddressHelper ipv4P2P;
    ipv4P2P.SetBase("10.0.2.0", "/30", "0.0.0.1");
    Ipv4InterfaceContainer interfaces = ipv4P2P.Assign(p2pDevices);

    //fine assegnazione ipv4


    //CONFIGURATION 0
    if (configuration == 0){


        //PACKET SINK n1
        uint16_t portN1 = 2600;
        Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), portN1));
        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
        ApplicationContainer sinkApp = packetSinkHelper.Install (star.GetSpokeNode(0));

        //TCP OnOff Client sul nodo n9
        OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
        onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(1500));
        AddressValue remoteAddress(InetSocketAddress(star.GetSpokeIpv4Address(0), portN1));
        onOffHelper.SetAttribute("Remote", remoteAddress);
        ApplicationContainer nodeApp = onOffHelper.Install (csma2Nodes.Get(2));
        nodeApp.Start (Seconds (3.0));
        nodeApp.Stop (Seconds (15.0));

        //abilitazione ASCII tracer
        pointToPoint.EnableAscii("task1-0-1.tr", star.GetSpokeNode(0)->GetDevice(0),true);
        csma2.EnableAscii("task1-0-9.tr", csma2Devices.Get(2),true);

        //abilitazione pcap
        NodeContainer hub = star.GetHub();
        csma1.EnablePcap("task1-0", csma1Devices.Get(1),true); // Pcap su n5
        pointToPoint2.EnablePcap("task1-0", p2pDevices.Get(1),true); // Pcap su n7
        pointToPoint.EnablePcap("task1-0", hub,true); // Pcap su n0
        //fine pcap
        

    }
    else if (configuration == 1){
        //PACKET SINK n1
        uint16_t portN1 = 2600;
        Address n1LocalAddress (InetSocketAddress (Ipv4Address::GetAny (), portN1));
        PacketSinkHelper n1SinkHelper ("ns3::TcpSocketFactory", n1LocalAddress);
        ApplicationContainer sink1App = n1SinkHelper.Install (star.GetSpokeNode(0));

        //PACKET SINK n2
        uint16_t portN2 = 7777;
        Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), portN2));
        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
        ApplicationContainer sinkApp = packetSinkHelper.Install (star.GetSpokeNode(1));

        //TCP OnOff Client sul nodo n9 che manda dati a n1
        OnOffHelper onOffHelper ("ns3::TcpSocketFactory", Address ());
        onOffHelper.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper.SetAttribute("PacketSize", UintegerValue(2500));
        AddressValue remoteN1(InetSocketAddress(star.GetSpokeIpv4Address(0), portN1));
        onOffHelper.SetAttribute("Remote", remoteN1);
        ApplicationContainer node9App = onOffHelper.Install (csma2Nodes.Get(2));
        node9App.Start (Seconds (5.0));
        node9App.Stop (Seconds (15.0));

        //TCP OnOff Client sul nodo n8 che manda dati a n2
        OnOffHelper onOffHelperN8 ("ns3::TcpSocketFactory", Address ());
        onOffHelperN8.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelperN8.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelperN8.SetAttribute("PacketSize", UintegerValue(5000));
        AddressValue remoteN2(InetSocketAddress(star.GetSpokeIpv4Address(1), portN2));
        onOffHelperN8.SetAttribute("Remote", remoteN2);
        ApplicationContainer node8App = onOffHelperN8.Install (csma2Nodes.Get(1));
        node8App.Start (Seconds (2.0));
        node8App.Stop (Seconds (9.0));

        //abilitazione ascii tracing
        pointToPoint.EnableAscii("task1-1-1.tr", star.GetSpokeNode(0)->GetDevice(0),true);
        pointToPoint.EnableAscii("task1-1-2.tr", star.GetSpokeNode(1)->GetDevice(0),true);
        csma2.EnableAscii("task1-1-8.tr", csma2Devices.Get(1),true);
        csma2.EnableAscii("task1-1-9.tr", csma2Devices.Get(2),true);
        //fine abilitazione ascii tracing


        //abilitazione pcap
        NodeContainer hub = star.GetHub();
        csma1.EnablePcap("task1-1", csma1Devices.Get(1),true); // Pcap su n5
        pointToPoint2.EnablePcap("task1-1", p2pDevices.Get(1),true); // Pcap su n7
        pointToPoint.EnablePcap("task1-1", hub,true); // Pcap su n0
    
        //fine pcap


    }
    else if (configuration == 2){
        
        //UDP Echo Server su n2
        UdpEchoServerHelper echoServer (63);
        ApplicationContainer udpApps = echoServer.Install (star.GetSpokeNode(1));

        //UDP Echo Client su n8
        UdpEchoClientHelper echoClient (star.GetSpokeIpv4Address(1), 63);
        echoClient.SetAttribute ("MaxPackets", UintegerValue (5));
        echoClient.SetAttribute ("Interval", TimeValue (Seconds (2.0))); //PACCHETTI DA INVIARE 3s,5s,7s,9s,11s
        echoClient.SetAttribute("PacketSize", UintegerValue(2560));

        std::string matricola = "Matricola: 779780 |";
        int size = 2559 - matricola.size();
        for( int i = 0; i<size ; i++){
            matricola += char((configuration*5)*i);
        }

        udpApps = echoClient.Install (csma2Nodes.Get (1)); 
        echoClient.SetFill (udpApps.Get(0), matricola);
        udpApps.Start (Seconds (3.0));
        udpApps.Stop (Seconds (12.0));

        //TCP SINK n1
        uint16_t portN1 = 2600;
        Address sinkLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), portN1));
        PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", sinkLocalAddress);
        ApplicationContainer sinkApp = packetSinkHelper.Install (star.GetSpokeNode(0));

        //UDP SINK n3
        uint16_t portN3 = 2500;
        Address udpSink (InetSocketAddress (Ipv4Address::GetAny (), portN3));
        PacketSinkHelper udpSinkHelper ("ns3::UdpSocketFactory", udpSink);
        ApplicationContainer udpApp = udpSinkHelper.Install (star.GetSpokeNode(2));

        //TCP OnOff Client sul nodo n9 che manda dati a n1
        OnOffHelper onOffHelper1 ("ns3::TcpSocketFactory", Address ());
        onOffHelper1.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper1.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper1.SetAttribute("PacketSize", UintegerValue(3000));
        AddressValue remoteN1(InetSocketAddress(star.GetSpokeIpv4Address(0), portN1));
        onOffHelper1.SetAttribute("Remote", remoteN1);
        ApplicationContainer node9App = onOffHelper1.Install (csma2Nodes.Get(2));
        node9App.Start (Seconds (3.0));
        node9App.Stop (Seconds (9.0));

        //Udp OnOff Client sul nodo n8 che manda dati a n3
        OnOffHelper onOffHelper2 ("ns3::UdpSocketFactory", Address ());
        onOffHelper2.SetAttribute ("OnTime", StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
        onOffHelper2.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
        onOffHelper2.SetAttribute("PacketSize", UintegerValue(3000));
        AddressValue remoteN2(InetSocketAddress(star.GetSpokeIpv4Address(2), portN3));
        onOffHelper2.SetAttribute("Remote", remoteN2);
        ApplicationContainer node8App = onOffHelper1.Install (csma2Nodes.Get(1));
        node8App.Start (Seconds (5.0));
        node8App.Stop (Seconds (15.0));

        //abilitazione pcap
        NodeContainer hub = star.GetHub();
        csma1.EnablePcap("task1-2", csma1Devices.Get(1),true); // Pcap su n5
        pointToPoint2.EnablePcap("task1-2", p2pDevices.Get(1),true); // Pcap su n7
        pointToPoint.EnablePcap("task1-2", hub,true); // Pcap su n0
    
        //fine pcap

        //abilitazione ascii tracing
        pointToPoint.EnableAscii("task1-2-1.tr", star.GetSpokeNode(0)->GetDevice(0),true);
        pointToPoint.EnableAscii("task1-2-2.tr", star.GetSpokeNode(1)->GetDevice(0),true);
        pointToPoint.EnableAscii("task1-2-3.tr", star.GetSpokeNode(2)->GetDevice(0),true);
        csma2.EnableAscii("task1-2-8.tr", csma2Devices.Get(1),true);
        csma2.EnableAscii("task1-2-9.tr", csma2Devices.Get(2),true);
        //fine abilitazione ascii tracing


    }

    Ipv4GlobalRoutingHelper::PopulateRoutingTables();

    
    Simulator::Stop(Seconds(20));
    Simulator::Run(); 
    
    Simulator::Destroy();
    
	return 0;
}
