/*DataStreamServer PlotJuggler  Plugin license(Faircode)

Copyright(C) 2018 Philippe Gauthier - ISIR - UPMC
Permission is hereby granted to any person obtaining a copy of this software and associated documentation files(the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and / or sell copies("Use") of the Software, and to permit persons to whom the Software is furnished to do so.
The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.
THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.
*/
#ifndef DATASTREAMSERVER_H
#define DATASTREAMSERVER_H

#include <QWebSocketServer>
#include <QWebSocket>
#include <QList>

#include <QtPlugin>
#include <thread>
#include "PlotJuggler/datastreamer_base.h"

#include "pff_msgs/navigation.h"
#include "pff_msgs/motor_state.h"
#include "pff_msgs/motor_limits.h"
#include "pff_msgs/motor_command.h"
#include "pff_mq/mq_publisher.h"
#include "pff_mq/mq_subscriber.h"

#include <nlohmann/json.hpp>

//#include <tao/json.hpp>
//#include <tao/json/contrib/traits.hpp>

class DataStreamServer : public DataStreamer {
Q_OBJECT
    Q_PLUGIN_METADATA(IID
                              "fr.upmc.isir.stech.plotjugglerplugins.DataStreamerServerZMQ")
    Q_INTERFACES(DataStreamer)

public:

    DataStreamServer();

    void getOD();

    virtual bool start() override;

    virtual void shutdown() override;

    virtual bool isRunning() const override { return _running; }

    virtual ~DataStreamServer();

    virtual const char *name() const override { return "DataStreamServerZMQ"; }

    virtual bool isDebugPlugin() override { return false; }

private:
    void              *_zmq_ctx  = {nullptr};
    void              *_socket   = {nullptr};
    std::thread        _thread;
    bool               _gotOD = {false};

    std::map<uint16_t, std::string> _od_map;

    quint16     _port;
    std::string _server_ip;

    bool _running;

private slots:

    void onNewConnection();

    void processMessage(QString message);

    void Listener();

};

#endif // DATASTREAMSERVER_H
