#include "mavlinkproxy.h"

#include <future>

#include <boost/date_time/posix_time/posix_time.hpp>

MAVLinkProxy::MAVLinkProxy() : _socket(_io_service, boost::asio::ip::udp::v4()), _heartbeat_timer(_io_service), _navdata_timer(_io_service)
{
    _mavlink_system.sysid = 0xAF;
    _mavlink_system.compid = MAV_COMP_ID_SYSTEM_CONTROL;

    boost::asio::ip::udp::resolver resolver(_io_service);
    boost::asio::ip::udp::resolver::query query("0.0.0.0", std::to_string(MAVLINK_PORT));
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

    _attitude.pitch = navdata->attitude(0);
    _attitude.roll = navdata->attitude(1);
    _attitude.yaw = navdata->attitude(2);
}

void MAVLinkProxy::connectionEstablished()
{

}

void MAVLinkProxy::connectionLost()
{

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

    mavlink_msg_attitude_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg, &_attitude);
    mavlink_msg_to_send_buffer(buf, &msg);
    _socket.send_to(boost::asio::buffer(buf), _endpoint);

    _navdata_timer.expires_from_now(boost::posix_time::milliseconds(MAVLINK_NAVDATA_INTERVAL));
    _navdata_timer.async_wait(boost::bind(&MAVLinkProxy::navdata, this));
}