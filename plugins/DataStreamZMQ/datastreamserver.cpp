/*DataStreamServer PlotJuggler  Plugin license(Faircode)
*/
#include "datastreamserver.h"
#include <QTextStream>
#include <QFile>
#include <QMessageBox>
#include <QDebug>
#include <QThread>
#include <thread>
#include <mutex>
#include <thread>
#include <math.h>
#include <QInputDialog>
#include <functional>
#include <chrono>
#include <iostream>

DataStreamServer::DataStreamServer() :
        _running(false)
{
    _zmq_ctx = zmq_ctx_new();
}

DataStreamServer::~DataStreamServer()
{
    shutdown();
}

void DataStreamServer::getOD()
{
    std::string endpoint = "tcp://" + _server_ip + ":" + std::to_string(_port + 1);
    qDebug() << "Looking for OD at " << endpoint.c_str();
    void *od_socket = zmq_socket(_zmq_ctx, ZMQ_REQ);
    int rc = zmq_connect(od_socket, endpoint.c_str());
    assert(rc == 0);
    std::string hello = "Hello";
    zmq_msg_t dataMsg;
    rc = zmq_msg_init_size(&dataMsg, hello.size()+1);
    assert(rc == 0);
    memcpy(zmq_msg_data(&dataMsg), hello.data(), hello.size()+1);
    rc = zmq_msg_send(&dataMsg, od_socket, ZMQ_DONTWAIT);
    //get dicitionary
    zmq_msg_t od_data;
    rc = zmq_msg_init (&od_data);
    assert(rc == 0);
    rc = zmq_msg_recv (&od_data, od_socket, 0);
    if(rc > 0) {
        using namespace nlohmann;

        printf("Got OD resp %lu\n", zmq_msg_size(&od_data));
        std::string js_str = (char *)zmq_msg_data(&od_data);
        zmq_msg_close(&od_data);

        auto j3 = json::parse(js_str);
        auto map = j3["m_mapByName"];
        printf("GotMap\n");
        // iterate the array
        for (json::iterator it = map.begin(); it != map.end(); ++it) {
            std::string name = (*it)["key"];
            const auto &val = (*it)["value"];
            uint16_t idx = val["m_index"];
            _od_map[idx] = name;
            printf("%d %s\n", idx, name.c_str());
            //std::cout << *it << '\n';
        }
        _gotOD = true;
    } else {

    }

}

bool DataStreamServer::start()
{
    using namespace std::placeholders;

    if (!_running) {
        bool ok;
        _port = QInputDialog::getInt(nullptr, tr(""),
                                     tr("On which port should the server listen to:"), 3322, 100, 65535, 1, &ok);
        if (ok) {
            QString ip = QInputDialog::getText(nullptr, tr(""),
                                               tr("On which IP server is located:"), QLineEdit::Normal,
                                               "192.168.1.4", &ok);
            if (ok) {
                _server_ip = std::string(ip.toUtf8().constData());
                std::string endpoint = "tcp://" + _server_ip + ":" + std::to_string(_port);
                qDebug() << "ZMQ listening on " << endpoint.c_str();
                _socket = zmq_socket(_zmq_ctx, ZMQ_SUB);
                zmq_connect(_socket, endpoint.c_str());
                std::string topic = "data";
                if (zmq_setsockopt(_socket, ZMQ_SUBSCRIBE, topic.data(), topic.size())) {
                    std::cerr << "Error occurred during zmq_setsocketopt(): " << zmq_strerror(errno) << std::endl;
                    throw errno;
                }
                _thread = std::thread(&DataStreamServer::Listener, this); //(std::bind(&DataStreamServer::Listener, this, std::placeholders::_1));

                _running = true;
            }
        } else {
            qDebug() << "Couldn't open websocket on port " << _port;
            _running = false;
        }
    }
    else {
        qDebug() << "Server already running on port " << _port;
        QMessageBox::information(nullptr,"Info",QString("Server already running on port: %1").arg(_port));
    }
    return _running;
}

void DataStreamServer::Listener()
{
    using namespace std::chrono;
    static std::chrono::high_resolution_clock::time_point initial_time = high_resolution_clock::now();

    zmq_pollitem_t items[1];
    /* First item refers to ØMQ socket 'socket' */
    items[0].socket = _socket;
    items[0].events = ZMQ_POLLIN;
    while (1)
    {
        int rc = zmq_poll(items, 1, 20);  // 20ms wait since it is mostly waiting for messages here
        if (rc == 0) {
            continue;  // nothing arrived, keep waiting;
        }
        if(!_gotOD) {
            getOD();
        }
        { //get topic
            zmq_msg_t topic_msg;
            zmq_msg_init(&topic_msg);
            zmq_msg_recv(&topic_msg, _socket, ZMQ_DONTWAIT);
            std::string str = std::string(static_cast<char*>(zmq_msg_data(&topic_msg)), zmq_msg_size(&topic_msg));
            zmq_msg_close(&topic_msg);
        }
        zmq_msg_t part;
        rc = zmq_msg_init (&part);
        assert(rc == 0);
        rc = zmq_msg_recv (&part, _socket, ZMQ_DONTWAIT);
        if(rc > 0) {
            int msgSize = zmq_msg_size(&part);
            uint8_t*ptr = (uint8_t *) zmq_msg_data(&part);
            //get timestamp
            int64_t  timeStamp;
            memcpy(&timeStamp, ptr, sizeof(timeStamp));
            ptr += sizeof(timeStamp);
            msgSize -= sizeof(timeStamp);
            const double t = timeStamp * 0.000000001;
            while(msgSize >= 2+8) {
                //received message first 2 bytes is index
                //next 8 bytes is double value
                uint16_t varIdx;
                double   varData;

                memcpy(&varIdx, ptr, 2);
                ptr += 2;
                memcpy(&varData, ptr, 8);
                ptr += 8;

                //auto now =  high_resolution_clock::now();
                //const double t = duration_cast< duration<double>>( now - initial_time ).count() ;

                std::lock_guard<std::mutex> lock( mutex() );

                auto& numeric_plots = dataMap().numeric;

                const std::string name_str = _od_map[varIdx];
                {
                    auto plotIt = numeric_plots.find(name_str);
                    if (plotIt == numeric_plots.end()) {
                        dataMap().addNumeric(name_str);
                    } else {
                        plotIt->second.pushBack({t, varData});
                    }
                }
                msgSize -= 2+8;
            }
        }
    }
}

void DataStreamServer::shutdown()
{
    if (_running) {
        _running = false;
    }
}

#if 0
DataStreamServer::DataStreamServer() :
	_running(false)
{
    _mqHandle = new pff::mq::MqHandle("Control", "*", 6000);
}

DataStreamServer::~DataStreamServer()
{
	shutdown();
}

bool DataStreamServer::start()
{
    using namespace std::placeholders;

	if (!_running) {
		bool ok;
		_port = QInputDialog::getInt(nullptr, tr(""),
			tr("On wich port should the server listen to:"), 5000, 1111, 65535, 1, &ok);
		if (ok)
		{
			qDebug() << "ZMQ listening on port" << _port;

            _mqHandle->addMasterConfig("motor_master", "192.168.1.4", _port);
            std::string motorStr("Left");
            new pff::mq::Subscriber<pff::msgs::MotorState>(*_mqHandle, motorStr+"_MotorState", "motor_master", std::bind(&DataStreamServer::MotorStateCallback, this, _1));

			_running = true;
		}
		else {
			qDebug() << "Couldn't open websocket on port " << _port;
			_running = false;
		}
	}
	else {
		qDebug() << "Server already running on port " << _port;
		QMessageBox::information(nullptr,"Info",QString("Server already running on port: %1").arg(_port));		
	}
	return _running;
}

void DataStreamServer::shutdown()
{
	if (_running) {
		_running = false;
	}
}

void DataStreamServer::onNewConnection()
{
	qDebug() << "DataStreamServer: onNewConnection";
}


void DataStreamServer::MotorStateCallback(const pff::msgs::MotorState &msg)
{

    /*
    uint32_t shaftEncoder;
    float    shaftVelocity;
    float    estimatedShaftVelocity;
    uint32_t actuatorEncoder;
    float    actuatorVelocity;
    float    actuatorPosition;
    float    actuatorOdometry;
    float    current;
    float    estimatedCurrent;
    float    commandedCurrent;
    float    temperature;
    float    estimatedDisturbance;
    uint32_t shaftEncoderErrorCount;
    uint32_t positionEncoderErrorCount;
    uint8_t  commutationPhase;
    */

    std::lock_guard<std::mutex> lock( mutex() );

    auto& numeric_plots = dataMap().numeric;

    const std::string name_str = "leftMotorState.";
    {
        auto plotIt = numeric_plots.find(name_str + "shaft_encoder");

        if (plotIt == numeric_plots.end()) {
            dataMap().addNumeric(name_str + "shaft_encoder");
        } else {
            plotIt->second.pushBack({msg.header.timestamp, msg.shaftEncoder});
        }
    }
    {
        auto plotIt = numeric_plots.find(name_str + "shaft_velocity");

        if (plotIt == numeric_plots.end()) {
            dataMap().addNumeric(name_str + "shaft_velocity");
        } else {
            plotIt->second.pushBack({msg.header.timestamp, msg.shaftVelocity});
        }
    }
    /*
    printf("Shaft encoder\t%d\n", msg.shaftEncoder);
    printf("Shaft velocity\t%f\n", msg.shaftVelocity);
    printf("Estimated shaft velocity\t%f\n", msg.estimatedShaftVelocity);
    printf("Actuator encoder\t%d\n", msg.actuatorEncoder);
    printf("Actuator velocity\t%f\n", msg.actuatorVelocity);
    printf("Actuator position\t%f\n", msg.actuatorPosition);
    printf("Actuator odometry\t%f\n", msg.actuatorOdometry);
    printf("Currrent\t%f\n", msg.current);
    printf("Estimated current\t%f\n", msg.estimatedCurrent);
    printf("Commanded current\t%f\n", msg.commandedCurrent);
    printf("Temperature\t%f\n", msg.temperature);
    printf("Commutation phase\t%d\n", msg.commutationPhase);
    printf("\n");
    */
}

void DataStreamServer::processMessage(QString message)
{
    std::lock_guard<std::mutex> lock( mutex() );

	//qDebug() << "DataStreamServer: processMessage: "<< message;
	QStringList lst = message.split(':');
	if (lst.size() == 3) {
		QString key = lst.at(0);
		double time = lst.at(1).toDouble();
		double value = lst.at(2).toDouble();

        auto& numeric_plots = dataMap().numeric;

		const std::string name_str = key.toStdString();
        auto plotIt = numeric_plots.find(name_str);
		
        if (plotIt == numeric_plots.end())
        {
            dataMap().addNumeric(name_str);
		}
        else{
            plotIt->second.pushBack( {time, value} );
        }
	}	
}
#endif

