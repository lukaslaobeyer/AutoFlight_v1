#include "mavlinkproxy.h"

#include <future>
#include <chrono>

#include <boost/date_time/posix_time/posix_time.hpp>

MAVLinkProxy::MAVLinkProxy() : _socket(_io_service, boost::asio::ip::udp::v4()), _heartbeat_timer(_io_service), _navdata_timer(_io_service)
{
    _mavlink_system.sysid = 0xAF;
    _mavlink_system.compid = MAV_COMP_ID_SYSTEM_CONTROL;

    boost::asio::ip::udp::resolver resolver(_io_service);
    boost::asio::ip::udp::resolver::query query("127.0.0.1", std::to_string(MAVLINK_PORT));
    _endpoint = *resolver.resolve(query);

    uint32_t sensors = MAV_SYS_STATUS_SENSOR_3D_GYRO
                     | MAV_SYS_STATUS_SENSOR_3D_ACCEL
                     | MAV_SYS_STATUS_SENSOR_3D_MAG
                     | MAV_SYS_STATUS_SENSOR_ABSOLUTE_PRESSURE
                     | MAV_SYS_STATUS_SENSOR_GPS
                     | MAV_SYS_STATUS_SENSOR_VISION_POSITION
                     | MAV_SYS_STATUS_SENSOR_ANGULAR_RATE_CONTROL
                     | MAV_SYS_STATUS_SENSOR_ATTITUDE_STABILIZATION
                     | MAV_SYS_STATUS_SENSOR_YAW_POSITION
                     | MAV_SYS_STATUS_SENSOR_Z_ALTITUDE_CONTROL;

    _sys_status.onboard_control_sensors_present = sensors;
    _sys_status.onboard_control_sensors_enabled = sensors & ~MAV_SYS_STATUS_SENSOR_GPS;
    _sys_status.onboard_control_sensors_health = sensors;
    _sys_status.load = 0;
    _sys_status.voltage_battery = 11100;
    _sys_status.current_battery = -1;
    _sys_status.drop_rate_comm = 0;
    _sys_status.errors_comm = 0;
    _sys_status.errors_count1 = 0;
    _sys_status.errors_count2 = 0;
    _sys_status.errors_count3 = 0;
    _sys_status.errors_count4 = 0;
    _sys_status.battery_remaining = 0;

    _attitude.pitch = 0;
    _attitude.pitchspeed = 0;
    _attitude.roll = 0;
    _attitude.rollspeed = 0;
    _attitude.yaw = 0;
    _attitude.yawspeed = 0;

    _gps.time_usec = 0;
    _gps.alt = 0;
    _gps.lat = 0;
    _gps.lon = 0;
    _gps.eph = UINT16_MAX;
    _gps.epv = UINT16_MAX;
    _gps.vel = UINT16_MAX;
    _gps.cog = UINT16_MAX;
    _gps.fix_type = 0;
    _gps.satellites_visible = 0;
}

void MAVLinkProxy::start()
{
    if(!_worker_running)
    {
        _io_service.reset();

        _worker_running = true;

        _worker.reset(new boost::thread(
                boost::bind(&boost::asio::io_service::run, &_io_service)
        ));

        _heartbeat_timer.expires_from_now(boost::posix_time::milliseconds(MAVLINK_HEARTBEAT_INTERVAL));
        _heartbeat_timer.async_wait(boost::bind(&MAVLinkProxy::heartbeat, this));

        _navdata_timer.expires_from_now(boost::posix_time::milliseconds(MAVLINK_NAVDATA_INTERVAL));
        _navdata_timer.async_wait(boost::bind(&MAVLinkProxy::navdata, this));

        _socket.async_receive_from(boost::asio::buffer(_received_msg_buf), _endpoint, boost::bind(&MAVLinkProxy::dataReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
    }
}

void MAVLinkProxy::stop()
{
    _io_service.stop();
    _worker->join();
    _worker.reset();

    _worker_running = false;
}

void MAVLinkProxy::navdataAvailable(std::shared_ptr<const drone::navdata> nd)
{
    unsigned long milliseconds_since_epoch =
            std::chrono::system_clock::now().time_since_epoch() /
            std::chrono::milliseconds(1);

    std::shared_ptr<const bebop::navdata> navdata = std::static_pointer_cast<const bebop::navdata>(nd);

    if(navdata->gps_fix)
    {
        _sys_status.onboard_control_sensors_enabled |= MAV_SYS_STATUS_SENSOR_GPS;
    }
    else
    {
        _sys_status.onboard_control_sensors_enabled &= ~MAV_SYS_STATUS_SENSOR_GPS;
    }

    _sys_status.battery_remaining = (int8_t) (navdata->batterystatus * 100.0f);

    if(navdata->full)
    {
        _sys_status.voltage_battery = (uint16_t) (navdata->full_navdata.vbat * 1000);
    }

    _attitude.pitch = navdata->attitude(0);
    _attitude.roll = navdata->attitude(1);
    _attitude.yaw = navdata->attitude(2);
    _attitude.time_boot_ms = (uint32_t) milliseconds_since_epoch;

    if(navdata->gps_fix || navdata->gps_sats > 4)
    {
        _gps.time_usec = milliseconds_since_epoch * 1000;
        _gps.lat = (int32_t) (navdata->latitude * 1.0E7);
        _gps.lon = (int32_t) (navdata->longitude * 1.0E7);
        _gps.alt = (int32_t) (navdata->gps_altitude * 1000.0);
        _gps.satellites_visible = (uint8_t) navdata->gps_sats;
        if(navdata->gps_altitude > 0)
        {
            _gps.fix_type = 3;
        }
        else
        {
            _gps.fix_type = 2;
        }

        if(!navdata->gps_fix)
        {
            _gps.fix_type = 0;
        }

        if(navdata->full)
        {
            if(navdata->full_navdata.gps_eph > 0 && navdata->full_navdata.gps_epv > 0)
            {
                _gps.eph = (uint16_t) (navdata->full_navdata.gps_eph * 100);
                _gps.epv = (uint16_t) (navdata->full_navdata.gps_epv * 100);
            }
            else
            {
                _gps.eph = UINT16_MAX;
                _gps.epv = UINT16_MAX;
            }
        }
    }
    else
    {
        _gps.time_usec = milliseconds_since_epoch * 1000;
        _gps.satellites_visible = 0;
        _gps.fix_type = 0;
    }
}

void MAVLinkProxy::connectionEstablished()
{
    _connected = true;
}

void MAVLinkProxy::connectionLost()
{
    _connected = false;
}

void MAVLinkProxy::heartbeat()
{
    static mavlink_message_t msg;
    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // System definition
    static uint8_t system_type = MAV_TYPE_QUADROTOR;
    static uint8_t autopilot_type = MAV_AUTOPILOT_GENERIC_WAYPOINTS_AND_SIMPLE_NAVIGATION_ONLY;

    uint8_t system_mode = MAV_MODE_PREFLIGHT;
    uint32_t custom_mode = 0;
    uint8_t system_state = MAV_STATE_STANDBY;

    if(_connected)
    {
        system_mode = MAV_MODE_STABILIZE_DISARMED;
    }

    if(_flying)
    {
        system_mode = MAV_MODE_STABILIZE_ARMED;
        system_state = MAV_STATE_ACTIVE;
    }

    mavlink_msg_heartbeat_pack(_mavlink_system.sysid, _mavlink_system.compid, &msg, system_type, autopilot_type, system_mode, custom_mode, system_state);
    mavlink_msg_to_send_buffer(buf, &msg);
    _socket.send_to(boost::asio::buffer(buf), _endpoint);

    mavlink_msg_sys_status_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg, &_sys_status);
    mavlink_msg_to_send_buffer(buf, &msg);
    _socket.send_to(boost::asio::buffer(buf), _endpoint);

    _heartbeat_timer.expires_from_now(boost::posix_time::milliseconds(MAVLINK_HEARTBEAT_INTERVAL));
    _heartbeat_timer.async_wait(boost::bind(&MAVLinkProxy::heartbeat, this));
}

void MAVLinkProxy::navdata()
{
    static mavlink_message_t msg;
    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    if(_attitude.time_boot_ms > 0)
    {
        mavlink_msg_attitude_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg, &_attitude);
        mavlink_msg_to_send_buffer(buf, &msg);
        _socket.send_to(boost::asio::buffer(buf), _endpoint);
    }

    if(_gps.time_usec > 0)
    {
        mavlink_msg_gps_raw_int_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg, &_gps);
        mavlink_msg_to_send_buffer(buf, &msg);
        _socket.send_to(boost::asio::buffer(buf), _endpoint);
    }

    _navdata_timer.expires_from_now(boost::posix_time::milliseconds(MAVLINK_NAVDATA_INTERVAL));
    _navdata_timer.async_wait(boost::bind(&MAVLinkProxy::navdata, this));
}

void MAVLinkProxy::dataReceived(const boost::system::error_code &error, size_t received_bytes)
{
    static mavlink_message_t msg;
    static mavlink_status_t status;

    for(int i = 0; i < (int) received_bytes; i++)
    {
        if(mavlink_parse_char(MAVLINK_COMM_0, _received_msg_buf[i], &msg, &status))
        {
            printf("Received packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
        }
    }

    // Listen for next packet
    _socket.async_receive_from(boost::asio::buffer(_received_msg_buf), _endpoint, boost::bind(&MAVLinkProxy::dataReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}