
#include <stdio.h>
#include <uWS.h>
#include "core/DataBase.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <time.h>
#include <chrono>
#include <thread>


using namespace std;
using namespace uWS;
int main()
{
    int max = 100;//Server
    int period = 1;//Server
    
    int clients = 1;//Client
    bool ourServer = true;//Client
    
    ///////////////////SERVER////////////////////////////////
    uWS::Hub h;
    
    SignalK::DataBase * pdb3 = new SignalK::DataBase("urn:mrn:signalk:uuid:705f5f1a-efaf-44aa-9cb8-a0fd6305567c", "v1.0.0");
    string init = "{\"updates\":[{\"source\":{\"sentence\":\"GLL\",\"talker\":\"GP\",\"type\":\"NMEA0183\",\"label\":\"nmeaFromFile\"},\"timestamp\":\"2017-05-03T10:17:52.000Z\",\"values\":[{\"path\":\"navigation.position\",\"value\":{\"longitude\":13.505133333333333,\"latitude\":50.0541}}]}],\"context\":\"vessels.urn:mrn:signalk:uuid:c0d79334-4e25-4245-8892-54e8ccc8021d\"}";
    
    std::ofstream results("data/Results.txt");
    pdb3->update(init);


    int times = 0;
    
    float processingTicks = 0;

    h.getDefaultGroup<uWS::CLIENT>().onMessage([pdb3, &times, max, period, &results, &processingTicks](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode x) {
        string msg(message, length);
        clock_t currTime;
        if (times < max)
        {
            currTime = clock();
            pdb3->update(msg);
            processingTicks += (float)(clock() - currTime);
            if (times % period == 0)
            {
                results << pdb3->subtree("");
                results << endl;
                results << endl;
            }
            times++;
        }
        else {
            ws->shutdown();
        }
        
        
    });
    
    h.onDisconnection([&results, pdb3, &processingTicks, &times, &h](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
        processingTicks = processingTicks / times;
        results << pdb3->subtree("");
        results << endl;
        results << endl;
        results << "average update time: "<< processingTicks/ CLOCKS_PER_SEC;
        std::cout << endl << "CLIENT CLOSE: " << code << std::endl;
        h.getDefaultGroup<uWS::SERVER>().close();
        
    });
    pdb3->SubscribeUpdate([&h](std::string x) {
        h.getDefaultGroup<uWS::SERVER>()
        .broadcast(x.c_str(), x.size(), uWS::OpCode::TEXT);
    });
    h.connect("ws://localhost:3000/signalk/v1/stream?subscribe=all", nullptr);
    h.listen(2000);
    ////////////CLIENTS///////////////////
    float messagesPerSecond = 0;
    int messageCount = 0;
    thread allClients([&messagesPerSecond, &messageCount, clients, ourServer]() {
        Hub hc;
        clock_t startTime;
        hc.getDefaultGroup<uWS::CLIENT>().onMessage([&messageCount,&startTime](uWS::WebSocket<uWS::CLIENT> *ws, char *message, size_t length, uWS::OpCode x) {
            if(messageCount == 0)
                startTime = clock();
            ////stmp////

            std::string cpps(message, length);
            cout << endl << cpps;


            messageCount++;
        });
        hc.onDisconnection([&messagesPerSecond, &messageCount, &startTime](uWS::WebSocket<uWS::CLIENT> *ws, int code, char *message, size_t length) {
            messagesPerSecond = (float)(clock() - startTime) / CLOCKS_PER_SEC;
            messagesPerSecond = messageCount/messagesPerSecond;
            ws->shutdown();
        });
        for (int i = 0; i < clients; i++) {
            hc.connect(ourServer ? "ws://localhost:2000/" : "ws://localhost:3000/signalk/v1/stream?subscribe=all",
                       nullptr);
        }
        hc.run();
    });
    ///////END/////////////////////////
    h.run();
    allClients.join();
    
    results <<endl << endl << "messages per second: " << messagesPerSecond;
    results << endl << endl << "total messages: " << messageCount;
    delete pdb3;
    //cin.get();

    cout << "average update time: "<< (processingTicks/ CLOCKS_PER_SEC)*1000<<" ms";
    cout <<endl << endl << "messages per second: " << messagesPerSecond;
    cout << endl << endl << "total messages: " << messageCount/clients;

    return 0;
}

