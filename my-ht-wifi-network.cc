/* -*-  Mode: C++; c-file-style: "gnu"; indent-tabs-mode:nil; -*- */
/*
 * Copyright (c) 2009 MIRKO BANCHI
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
 * Authors: Mirko Banchi <mk.banchi@gmail.com>
 *          Sebastien Deronne <sebastien.deronne@gmail.com>
 */

#include "ns3/core-module.h"
#include "ns3/network-module.h"
#include "ns3/applications-module.h"
#include "ns3/wifi-module.h"
#include "ns3/mobility-module.h"
#include "ns3/ipv4-global-routing-helper.h"
#include "ns3/internet-module.h"
#include "ns3/flow-monitor-module.h"
#include "ns3/netanim-module.h"

// This is a simple example in order to show how to configure an IEEE 802.11n Wi-Fi network.
//
// It ouputs the UDP or TCP goodput for every VHT bitrate value, which depends on the MCS value (0 to 7), the
// channel width (20 or 40 MHz) and the guard interval (long or short). The PHY bitrate is constant over all
// the simulation run. The user can also specify the distance between the access point and the station: the
// larger the distance the smaller the goodput.
//
// The simulation assumes a single station in an infrastructure network:
//
//  STA     AP
//    *     *
//    |     |
//   n1     n2
//
//Packets in this simulation aren't marked with a QosTag so they are considered
//belonging to BestEffort Access Class (AC_BE).

using namespace ns3;

NS_LOG_COMPONENT_DEFINE ("ht-wifi-network");

    //For checking the course if change
void CourseChange (std::string context, Ptr<const MobilityModel> model)
{
  Vector position = model->GetPosition ();
  NS_LOG_UNCOND (context << " x = " << position.x << ", y = " << position.y);
}

int main (int argc, char *argv[])
{
  bool udp = true;
  double simulationTime = 2; //seconds
  double distance = 1.0; //meters
  double frequency = 5.0; //whether 2.4 or 5.0 GHz
  uint32_t i = 0;   //MCS(0~7)
  uint32_t j = 20;  //channel width(20,40)
  uint32_t k = 0;   //GI(0~1)
  uint32_t nWifiStaNode = 3;

  CommandLine cmd;
  cmd.AddValue ("frequency", "Whether working in the 2.4 or 5.0 GHz band (other values gets rejected)", frequency);
  cmd.AddValue ("distance", "Distance in meters between the station and the access point", distance);
  cmd.AddValue ("simulationTime", "Simulation time in seconds", simulationTime);
  cmd.AddValue ("udp", "UDP if set to 1, TCP otherwise", udp);
  cmd.Parse (argc,argv);
  
  std::cout << "MCS value" << "\t\t" << "Channel width" << "\t\t" << "short GI" << "\t\t" << "Throughput" << '\n';
  
      
           
  uint32_t payloadSize; //1500 byte IP packet
  if (udp)
    {
      payloadSize = 1472; //bytes
    }
  else
    {
      payloadSize = 1448; //bytes
      Config::SetDefault ("ns3::TcpSocket::SegmentSize", UintegerValue (payloadSize));
    }

  NodeContainer wifiStaNode;
  wifiStaNode.Create (nWifiStaNode);
  NodeContainer wifiApNode;
  wifiApNode.Create (1);

  YansWifiChannelHelper channel = YansWifiChannelHelper::Default ();
  YansWifiPhyHelper phy = YansWifiPhyHelper::Default ();
  phy.SetChannel (channel.Create ());

  // Set guard interval
  phy.Set ("ShortGuardEnabled", BooleanValue (k));

  WifiMacHelper mac;
  WifiHelper wifi;
  if (frequency == 5.0)
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_5GHZ);
    }
  else if (frequency == 2.4)
    {
      wifi.SetStandard (WIFI_PHY_STANDARD_80211n_2_4GHZ);
      Config::SetDefault ("ns3::LogDistancePropagationLossModel::ReferenceLoss", DoubleValue (40.046));
    }
  else
    {
      std::cout<<"Wrong frequency value!"<<std::endl;
      return 0;
    }

  std::ostringstream oss;
  oss << "HtMcs" << i;
  wifi.SetRemoteStationManager ("ns3::ConstantRateWifiManager","DataMode", StringValue (oss.str ()),
                                "ControlMode", StringValue (oss.str ()));
    
  Ssid ssid = Ssid ("ns3-80211n");

  mac.SetType ("ns3::StaWifiMac",
               "Ssid", SsidValue (ssid));

  NetDeviceContainer staDevice;
  staDevice = wifi.Install (phy, mac, wifiStaNode);

  //mac.SetType ("ns3::ApWifiMac",
  //             "Ssid", SsidValue (ssid));

  //NetDeviceContainer apDevice;
  //apDevice = wifi.Install (phy, mac, wifiApNode);

  // Set channel width
  Config::Set ("/NodeList/*/DeviceList/*/$ns3::WifiNetDevice/Phy/ChannelWidth", UintegerValue (j));

  // mobility
  /*
  MobilityHelper mobility;
  
  mobility.SetPositionAllocator ("ns3::GridPositionAllocator",
                                 "MinX", DoubleValue (0.0),
                                 "MinY", DoubleValue (0.0),
                                 "DeltaX", DoubleValue (5.0),
                                 "DeltaY", DoubleValue (10.0),
                                 "GridWidth", UintegerValue (3),
                                 "LayoutType", StringValue ("RowFirst"));

  mobility.SetMobilityModel ("ns3::RandomWalk2dMobilityModel",
                             "Bounds", RectangleValue (Rectangle (-10, 10, -10, 10)));    //(xmin,xmax,ymin,ymax)
  mobility.Install (wifiStaNode);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");
  mobility.Install (wifiApNode);
  */
  
  
  MobilityHelper mobility;
  Ptr<ListPositionAllocator> positionAlloc = CreateObject<ListPositionAllocator> ();

  //positionAlloc->Add (Vector (0.0, 0.0, 0.0));
  positionAlloc->Add (Vector (100.0, 50.0, 0.0));
  positionAlloc->Add (Vector (10.0, 10.0, 0.0));
  positionAlloc->Add (Vector (10.0, 10.0, 0.0));
  mobility.SetPositionAllocator (positionAlloc);

  mobility.SetMobilityModel ("ns3::ConstantPositionMobilityModel");

  //mobility.Install (wifiApNode);
  mobility.Install (wifiStaNode);
  

  /* Internet stack*/
  InternetStackHelper stack;
  stack.Install (wifiApNode);
  stack.Install (wifiStaNode);

  Ipv4AddressHelper address;

  address.SetBase ("192.168.1.0", "255.255.255.0");
  Ipv4InterfaceContainer staNodeInterface;
  Ipv4InterfaceContainer apNodeInterface;

  staNodeInterface = address.Assign (staDevice);
  //apNodeInterface = address.Assign (apDevice);

  /* Setting applications */
  ApplicationContainer serverApp, sinkApp ,serverApp1;
  if (udp)
    {
      //UDP flow
      //UdpServerHelper myServer (9);
      //serverApp = myServer.Install (wifiApNode.Get (0));
      //serverApp.Start (Seconds (0.0));
      //serverApp.Stop (Seconds (0.0));
      UdpClientHelper myClient (staNodeInterface.GetAddress (0), 9);
      myClient.SetAttribute ("MaxPackets", UintegerValue (4294967295u));
      myClient.SetAttribute ("Interval", TimeValue (Time ("0.00001"))); //packets/s
      myClient.SetAttribute ("PacketSize", UintegerValue (payloadSize));

      ApplicationContainer clientApp = myClient.Install (wifiStaNode);
      clientApp.Start (Seconds (1.0));
      clientApp.Stop (Seconds (simulationTime + 1));
    }
  else
    {
      //TCP flow
      /*
      uint16_t port = 50000;
      Address apLocalAddress (InetSocketAddress (Ipv4Address::GetAny (), port));
      PacketSinkHelper packetSinkHelper ("ns3::TcpSocketFactory", apLocalAddress);
      sinkApp = packetSinkHelper.Install (wifiStaNode);
      
      sinkApp.Start (Seconds (0.0));
      sinkApp.Stop (Seconds (simulationTime + 1));

      OnOffHelper onoff ("ns3::TcpSocketFactory",Ipv4Address::GetAny ());
      onoff.SetAttribute ("OnTime",  StringValue ("ns3::ConstantRandomVariable[Constant=1]"));
      onoff.SetAttribute ("OffTime", StringValue ("ns3::ConstantRandomVariable[Constant=0]"));
      onoff.SetAttribute ("PacketSize", UintegerValue (payloadSize));
      onoff.SetAttribute ("DataRate", DataRateValue (1000000000)); //bit/s
      ApplicationContainer apps;

      AddressValue remoteAddress (InetSocketAddress (staNodeInterface.GetAddress (0), port));
      onoff.SetAttribute ("Remote", remoteAddress);
      apps.Add (onoff.Install (wifiApNode.Get (0)));
      apps.Start (Seconds (1.0));
      apps.Stop (Seconds (simulationTime + 1));
      */
    }

  Ipv4GlobalRoutingHelper::PopulateRoutingTables ();

  Simulator::Stop (Seconds (simulationTime + 1));
  //phy.EnablePcap("log",wifiStaNode); 

  
  std::ostringstream oss1;
  oss1 << "/NodeList/" << wifiStaNode.Get (0)->GetId () << "/$ns3::MobilityModel/CourseChange";
  Config::Connect (oss1.str (), MakeCallback (&CourseChange));

  
  AnimationInterface anim ("anim11.xml");
  //anim.SetConstantPosition(wifiApNode.Get(0), 0.0, 0.0);
  //anim.SetConstantPosition(wifiStaNode.Get(0), 50.0, 50.0);
  //anim.SetConstantPosition(wifiStaNode.Get(1), 50.0, 25.0);
  //anim.SetConstantPosition(wifiStaNode.Get(2), 50.0, 10.0);

  double throughput = 0;

  Simulator::Run ();
  

  if (udp)
    {
      //UDP
      //uint32_t totalPacketsThrough = DynamicCast<UdpServer> (serverApp.Get (0))->GetReceived ();
      //throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
      
    }
  else
    {
      //TCP
      uint32_t totalPacketsThrough = DynamicCast<PacketSink> (sinkApp.Get (1))->GetTotalRx ();
      throughput = totalPacketsThrough * payloadSize * 8 / (simulationTime * 1000000.0); //Mbit/s
    }
  std::cout << i << "\t\t\t" << j << " MHz\t\t\t" << k << "\t\t\t" << throughput << " Mbit/s" << std::endl;


  Simulator::Destroy ();

  
  
  
  
  return 0;
}
