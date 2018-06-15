
#include <stdio.h>
#include "DataBase.h"
#include <iostream>
#include <fstream>
#include <streambuf>
#include <time.h>
#include <chrono>
#include <thread>


using namespace std;

int main()
{
    /*
    SignalK::DataBase db1;
    std::ifstream t("jsontest.txt");
    if (t.is_open()) {

        db1.load(t, false, false);
        SignalK::DataBase db2 = db1;
        cout << db2;
        cin.get();
        cout << endl;
        cout << db2.subtree("vessels.urn:mrn:signalk:uuid:705f5f1a-efaf-44aa-9cb8-a0fd6305567c.navigation.position");
        cin.get();
        cout << db2.getVesselProperty("urn:mrn:signalk:uuid:705f5f1a-efaf-44aa-9cb8-a0fd6305567c", "navigation.position");
        cin.get();
        cout << db2.getSource("ttyUSB0");
        cin.get();
        SignalK::DataBase db3("urn:mrn:signalk:uuid:705f5f1a-efaf-44aa-9cb8-a0fd6305567c", "v1.0.0");
        cout << db3.toJson();
        cin.get();
        cout << db3.getSelf()<< " " << db3.getVersion()<< endl;
        cin.get();
        std::ifstream d("jsondelta.txt");
        std::string delta((std::istreambuf_iterator<char>(d)),
            std::istreambuf_iterator<char>());
        db2.update(delta);
        cout << db2.subtree("");
        cin.get();
    }
    */
    SignalK::DataBase * pdb3 = new SignalK::DataBase("urn:mrn:signalk:uuid:705f5f1a-efaf-44aa-9cb8-a0fd6305567c", "v1.0.0");
    std::string init = "{\"updates\":[{\"source\":{\"sentence\":\"GLL\",\"talker\":\"GP\",\"type\":\"NMEA0183\",\"label\":\"nmeaFromFile\"},\"timestamp\":\"2017-05-03T10:17:52.000Z\",\"values\":[{\"path\":\"navigation.position\",\"value\":{\"longitude\":13.505133333333333,\"latitude\":50.0541}}]}],\"context\":\"vessels.urn:mrn:signalk:uuid:c0d79334-4e25-4245-8892-54e8ccc8021d\"}";
    std::ifstream updates("JsonTests.txt");
    std::ofstream results("Results.txt");
    pdb3->update(init);
    std::string result = pdb3->readUpdate(updates);
    const int print = 5;
    const int readNumbers = 10;
    int count = 0;
    float processingTicks = 0;

    clock_t currTime;
    thread reader([pdb3, readNumbers]() {
        clock_t currReadTime;
        float processingReadTicks = 0;
        std::ofstream readerResults("ReaderResults.txt");
        
        for (int i = 0; i < readNumbers; i++) {
            this_thread::sleep_for(chrono::milliseconds(3));
            readerResults << "read result: ";
            readerResults << endl;
            currReadTime = clock();
            string res= pdb3->subtree("vessels.urn:mrn:signalk:uuid:c0d79334-4e25-4245-8892-54e8ccc8021d.navigation.position");
            processingReadTicks += (float)(clock() - currReadTime);
            readerResults << res;
            readerResults << endl;
            readerResults << endl;
        }
        processingReadTicks = processingReadTicks / readNumbers;
        readerResults << "average read time in seconds: ";
        readerResults << processingReadTicks / CLOCKS_PER_SEC;
        readerResults.close();
    });
    while (result != "")
    {
        result=pdb3->readUpdate(updates);
        currTime= clock();
        pdb3->update(result);
        processingTicks += (float)(clock()-currTime);
        if ( count % 5 == 0) {
            results << pdb3->subtree("");
            results << endl;
            results << endl;
        }
        count++;
    }
    processingTicks = processingTicks / count;
    results << endl;
    results << endl;
    results << "average update time in seconds: ";
    results << processingTicks / CLOCKS_PER_SEC;
    reader.join();
    delete pdb3;
    return 0;
}

