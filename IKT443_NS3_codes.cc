/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 University of Washington
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation;
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 *
 */



/*
    IKT443 Exercise Part B Assignment : Solution based on wifi-simple-adhoc-grid.cc

    1. Setup a wireless adhoc network. You may use examples/wireless/wifi-simple-adhoc-grid.cc as a base.

    2. Install a routing protocol.

    3. Setup three UDP traffic flows.

    4. Setup the ns-3 flow monitor for each of these flows.

    5. Now using the flow monitor, observe the throughput of each of the
    UDP flows. 

    7. Repeat the experiment for different number of nodes, flows, topologies, protocols, rates, packet sizes, mobility models with RTS/CTS enabled on the wifi devices.

    8. Show the difference in throughput and packet drops if any.



// The default layout is like this, on a 2-D grid.

        n20  n21  n22  n23  n24
        n15  n16  n17  n18  n19
        n10  n11  n12  n13  n14
        n5   n6   n7   n8   n9
        n0   n1   n2   n3   n4

// the layout is affected by the parameters given to GridPositionAllocator;
// by default, GridWidth is 5 and numNodes is 25..

         Flow 1: 0->24
         Flow 2: 20->4
         Flow 3: 10->4

*/

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/mobility-module.h"
#include "ns3/config-store-module.h"
#include "ns3/wifi-module.h"
#include "ns3/internet-module.h"
#include "ns3/ipv4-flow-classifier.h"
#include "ns3/applications-module.h"

// routing protocols
#include "ns3/aodv-module.h"
#include "ns3/olsr-module.h"
#include "ns3/dsdv-module.h"
#include "ns3/dsr-module.h"
#include "ns3/olsr-helper.h"

// header file specific to this exercise
#include "myapp.h"

// added for flow monitoring
#include "ns3/gnuplot.h"
#include "ns3/flow-monitor-module.h"
#include <ns3/flow-monitor-helper.h>

// animator
#include "ns3/netanim-module.h"
// std library files
#include <iomanip>
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <sstream>

NS_LOG_COMPONENT_DEFINE ("IKT443_Exercise_Part_B");

using namespace ns3;
//added
void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet);

uint32_t MacTxDropCount, PhyTxDropCount, PhyRxDropCount;


void
MacTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  MacTxDropCount++;
}

void
PrintDrop()
{
  std::cout << Simulator::Now().GetSeconds() << "\t" << MacTxDropCount << "\t"<< PhyTxDropCount << "\t" << PhyRxDropCount << "\n";
  Simulator::Schedule(Seconds(5.0), &PrintDrop);
}

void
PhyTxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyTxDropCount++;
}
void
PhyRxDrop(Ptr<const Packet> p)
{
  NS_LOG_INFO("Packet Drop");
  PhyRxDropCount++;
}
int main (int argc, char *argv[])
{
  std::string phyMode ("DsssRate1Mbps");
  double distance = 500;  // m
  double interval = 0.001; // seconds
  uint32_t packetSize = 600; // bytes
  uint32_t numPackets = 10000000;
  std::string rtslimit = "1500";

  int       m_xSize = 5;
  int       m_ySize= 5;
  double    m_step = 100;

  bool m_traceMobility = true;
/* // enable for mobility ------------
  
  int nodeSpeed = 30; //in m/s
  int nodePause = 0; //in s */
//-----------------------------------

  uint32_t m_protocol = 1 ;    // 1=OLSR;2=AODV;3=DSDV;4=DSR

  CommandLine cmd;

  cmd.AddValue ("x-size", "Number of nodes in a row grid. [6]", m_xSize);
  cmd.AddValue ("y-size", "Number of rows in a grid. [6]", m_ySize);
  cmd.AddValue ("step",   "Size of edge in our grid, meters. [100 m]", m_step);

  cmd.AddValue ("phyMode", "Wifi Phy mode", phyMode);
  cmd.AddValue ("distance", "distance (m)", distance);
  cmd.AddValue ("packetSize", "distance (m)", packetSize);
  cmd.AddValue ("rtslimit", "RTS/CTS Threshold (bytes)", rtslimit);

  cmd.AddValue ("traceMobility", "Enable mobility tracing", m_traceMobility);
  cmd.AddValue ("protocol", "1=OLSR;2=AODV;3=DSDV;4=DSR", m_protocol);

  cmd.Parse (argc, argv);

  // load the topology
  NS_LOG_DEBUG ("Grid:" << m_xSize << "*" << m_ySize);

  // Convert to time object
  Time interPacketInterval = Seconds (interval);

  // turn off RTS/CTS for frames below 2200 bytes
  Config::SetDefault ("ns3::WifiRemoteStationManager::RtsCtsThreshold", StringValue (rtslimit));
  // Fix non-unicast data rate to be the same as that of unicast
  Config::SetDefault ("ns3::WifiRemoteStationManager::NonUnicastMode", StringValue (phyMode));


  // Create m_ySize*m_xSize stations to form a grid topology
  NodeContainer c;
  c.Create (m_ySize*m_xSize);

  // The below set of helpers will help us to put together the wifi NICs we want
  WifiHelper wifi;

  YansWifiPhyHelper wifiPhy =  YansWifiPhyHelper::Default ();
  // set it to zero; otherwise, gain will be added
  wifiPhy.Set ("RxGain", DoubleValue (-10) ); 
  // ns-3 supports RadioTap and Prism tracing extensions for 802.11b
  wifiPhy.SetPcapDataLinkType (YansWifiPhyHelper::DLT_IEEE802_11_RADIO); 

  YansWifiChannelHelper wifiChannel;
  wifiChannel.SetPropagationDelay ("ns3::ConstantSpeedPropagationDelayModel");
  wifiChannel.AddPropagationLoss ("ns3::FriisPropagationLossModel");
  wifiPhy.SetChannel (wifiChannel.Create ());

  // Add a non-QoS upper mac, and disable rate control
  //NqosWifiMacHelper wifiMac = NqosWifiMacHelper::Default (); \\old
  WifiMacHelper wifiMac; // updated 2022
  wifi.SetStandard (WIFI_PHY_STANDARD_80211b);
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager",
                                "DataMode",StringValue (phyMode),
                                "ControlMode",StringValue (phyMode));
  // Set it to adhoc mode
  wifiMac.SetType ("ns3::AdhocWifiMac");
  NetDeviceContainer devices = wifi.Install (wifiPhy, wifiMac, c);

  MobilityHelper mobility;
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (distance),
                                 "DeltaY", DoubleValue (distance),
                                 "GridWidth", UintegerValue (m_xSize),
                                 "LayoutType", StringValue ("RowFirst"));

   //Enable stattionary grid (no mobility)
 // /*
  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (c);

// */


  /*
// Enable Randomwaypoint mobility model ------------------------------------------------------

int64_t streamIndex = 0; // used to get consistent mobility across scenarios

ObjectFactory pos;
  pos.SetTypeId ("ns3::RandomRectanglePositionAllocator");
  pos.Set ("X", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));
  pos.Set ("Y", StringValue ("ns3::UniformRandomVariable[Min=0.0|Max=500.0]"));

  Ptr<PositionAllocator> taPositionAlloc = pos.Create ()->GetObject<PositionAllocator> ();
  streamIndex += taPositionAlloc->AssignStreams (streamIndex);

std::stringstream ssSpeed;
  ssSpeed << "ns3::UniformRandomVariable[Min=0.0|Max=" << nodeSpeed << "]";
  std::stringstream ssPause;
  ssPause << "ns3::ConstantRandomVariable[Constant=" << nodePause << "]";
  mobility.SetMobilityModel ("ns3::RandomWaypointMobilityModel",
                                  "Speed", StringValue (ssSpeed.str ()),
                                  "Pause", StringValue (ssPause.str ()),
                                  "PositionAllocator", PointerValue (taPositionAlloc));
  mobility.SetPositionAllocator (taPositionAlloc);
  mobility.Install (c);
  streamIndex += mobility.AssignStreams (c, streamIndex);
  //mobilityAdhoc.Install(sinkNodes);
  //streamIndex += mobilityAdhoc.AssignStreams (sinkNodes, streamIndex);
*/
//-----------------------------------------------------------------------------------------

//enable routing protocol -------------------------------------------

  AodvHelper aodv;
  OlsrHelper olsr;
  DsdvHelper dsdv;
  DsrHelper dsr;
  DsrMainHelper dsrMain;
  Ipv4ListRoutingHelper list;
  InternetStackHelper internet;

  switch (m_protocol)
    {
    case 1:
      list.Add (olsr, 100);
     // m_protocolName = "OLSR";
      break;
    case 2:
      list.Add (aodv, 100);
     // m_protocolName = "AODV";
      break;
    case 3:
      list.Add (dsdv, 100);
      //m_protocolName = "DSDV";
      break;
    case 4:
     // m_protocolName = "DSR";
      break;
    default:
      NS_FATAL_ERROR ("No such protocol:" << m_protocol);
    }

  if (m_protocol < 4)
    {
      internet.SetRoutingHelper (list);
      //internet.Install (sinkNodes);
      internet.Install (c);
    }
  else if (m_protocol == 4)
    {
      internet.Install (c);
      //internet.Install (sinkNodes);
      dsrMain.Install (dsr, c);
      //dsrMain.Install (dsr, sinkNodes);
    }
//-------------------------------------------------------------------
  
// Assigning ip addresses to nodes


  Ipv4AddressHelper ipv4;
  NS_LOG_INFO ("Assign IP Addresses.");
  ipv4.SetBase ("10.1.1.0", "255.255.255.0");
  Ipv4InterfaceContainer ifcont = ipv4.Assign (devices);

  // Create Apps

  uint16_t sinkPort = 6; // use the same for all apps

  // UDP connection from N0 to N24

   Address sinkAddress1 (InetSocketAddress (ifcont.GetAddress (24), sinkPort)); // interface of n24
   PacketSinkHelper packetSinkHelper1 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
   ApplicationContainer sinkApps1 = packetSinkHelper1.Install (c.Get (24)); //n2 as sink
   sinkApps1.Start (Seconds (0.));
   sinkApps1.Stop (Seconds (100.));

   Ptr<Socket> ns3UdpSocket1 = Socket::CreateSocket (c.Get (0), UdpSocketFactory::GetTypeId ()); //source at n0

   // Create UDP application at n0
   Ptr<MyApp> app1 = CreateObject<MyApp> ();
   app1->Setup (ns3UdpSocket1, sinkAddress1, packetSize, numPackets, DataRate ("1Mbps"));
   c.Get (0)->AddApplication (app1);
   app1->SetStartTime (Seconds (31.));
   app1->SetStopTime (Seconds (100.));

   // UDP connection from N10 to N14

    Address sinkAddress2 (InetSocketAddress (ifcont.GetAddress (14), sinkPort)); // interface of n14
    PacketSinkHelper packetSinkHelper2 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
    ApplicationContainer sinkApps2 = packetSinkHelper2.Install (c.Get (14)); //n14 as sink
    sinkApps2.Start (Seconds (0.));
    sinkApps2.Stop (Seconds (100.));

    Ptr<Socket> ns3UdpSocket2 = Socket::CreateSocket (c.Get (10), UdpSocketFactory::GetTypeId ()); //source at n10

    // Create UDP application at n10
    Ptr<MyApp> app2 = CreateObject<MyApp> ();
    app2->Setup (ns3UdpSocket2, sinkAddress2, packetSize, numPackets, DataRate ("1Mbps"));
    c.Get (10)->AddApplication (app2);
    app2->SetStartTime (Seconds (31.5));
    app2->SetStopTime (Seconds (100.));

    // UDP connection from N20 to N4

     Address sinkAddress3 (InetSocketAddress (ifcont.GetAddress (4), sinkPort)); // interface of n4
     PacketSinkHelper packetSinkHelper3 ("ns3::UdpSocketFactory", InetSocketAddress (Ipv4Address::GetAny (), sinkPort));
     ApplicationContainer sinkApps3 = packetSinkHelper3.Install (c.Get (4)); //n2 as sink
     sinkApps3.Start (Seconds (0.));
     sinkApps3.Stop (Seconds (100.));

     Ptr<Socket> ns3UdpSocket3 = Socket::CreateSocket (c.Get (20), UdpSocketFactory::GetTypeId ()); //source at n20

     // Create UDP application at n20
     Ptr<MyApp> app3 = CreateObject<MyApp> ();
     app3->Setup (ns3UdpSocket3, sinkAddress3, packetSize, numPackets, DataRate ("1Mbps"));
     c.Get (20)->AddApplication (app3);
     app3->SetStartTime (Seconds (32.));
     app3->SetStopTime (Seconds (100.));

        //Gnuplot parameters added
      
    std::string fileNameWithNoExtension = "IKT443_FlowVSThroughput_NO_RTS";
    std::string graphicsFileName        = fileNameWithNoExtension + ".png";
    std::string plotFileName            = fileNameWithNoExtension + ".plt";
    std::string plotTitle               = "Flow vs Throughput";
    std::string dataTitle               = "Throughput";

    // Instantiate the plot and set its title.
    Gnuplot gnuplot (graphicsFileName);
    gnuplot.SetTitle (plotTitle);

    // Make the graphics file, which the plot file will be when it
    // is used with Gnuplot, be a PNG file.
    gnuplot.SetTerminal ("png");

    // Set the labels for each axis.
    gnuplot.SetLegend ("Flow", "Throughput");

     
   Gnuplot2dDataset dataset;
   dataset.SetTitle (dataTitle);
   dataset.SetStyle (Gnuplot2dDataset::LINES_POINTS);
/*
  // Install FlowMonitor on all nodes
  FlowMonitorHelper flowmon;
  Ptr<FlowMonitor> monitor = flowmon.InstallAll(); */

// Install FlowMonitor on all nodes
  FlowMonitorHelper fmHelper;
  Ptr<FlowMonitor> monitor = fmHelper.InstallAll();

// call the flow monitor function
  ThroughputMonitor(&fmHelper, monitor, dataset); 
 

  // Trace Collisions
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Mac/MacTxDrop", MakeCallback(&MacTxDrop));
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyRxDrop", MakeCallback(&PhyRxDrop));
  Config::ConnectWithoutContext("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/PhyTxDrop", MakeCallback(&PhyTxDrop));

  Simulator::Schedule(Seconds(5.0), &PrintDrop);

  Simulator::Stop (Seconds (100.0));
  AnimationInterface anim ("animation.xml");
  Simulator::Run ();

  PrintDrop();

  monitor->CheckForLostPackets ();
 
//Gnuplot ...continued
  gnuplot.AddDataset (dataset);
  // Open the plot file.
  std::ofstream plotFile (plotFileName.c_str());
  // Write the plot file.
  gnuplot.GenerateOutput (plotFile);
  // Close the plot file.
  plotFile.close ();

  Simulator::Destroy ();

  return 0;
}




void ThroughputMonitor (FlowMonitorHelper *fmhelper, Ptr<FlowMonitor> flowMon,Gnuplot2dDataset DataSet)
	{
        double localThrou=0;
		std::map<FlowId, FlowMonitor::FlowStats> flowStats = flowMon->GetFlowStats();
		Ptr<Ipv4FlowClassifier> classing = DynamicCast<Ipv4FlowClassifier> (fmhelper->GetClassifier());
		for (std::map<FlowId, FlowMonitor::FlowStats>::const_iterator stats = flowStats.begin (); stats != flowStats.end (); ++stats)
		{
			Ipv4FlowClassifier::FiveTuple fiveTuple = classing->FindFlow (stats->first);
			std::cout<<"Flow ID			: " << stats->first <<" ; "<< fiveTuple.sourceAddress <<" -----> "<<fiveTuple.destinationAddress<<std::endl;
			std::cout<<"Tx Packets = " << stats->second.txPackets<<std::endl;
			std::cout<<"Rx Packets = " << stats->second.rxPackets<<std::endl;
            std::cout<<"Duration		: "<<(stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())<<std::endl;
			std::cout<<"Last Received Packet	: "<< stats->second.timeLastRxPacket.GetSeconds()<<" Seconds"<<std::endl;
			std::cout<<"Throughput: " << stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024  << " Mbps"<<std::endl;
            localThrou=(stats->second.rxBytes * 8.0 / (stats->second.timeLastRxPacket.GetSeconds()-stats->second.timeFirstTxPacket.GetSeconds())/1024/1024);
			// updata gnuplot data
            DataSet.Add((double)Simulator::Now().GetSeconds(),(double) localThrou);
			std::cout<<"---------------------------------------------------------------------------"<<std::endl;
		}
			Simulator::Schedule(Seconds(1),&ThroughputMonitor, fmhelper, flowMon,DataSet);
   //if(flowToXml)
      {
	flowMon->SerializeToXmlFile ("ThroughputMonitor.xml", true, true);
      }

	} 


