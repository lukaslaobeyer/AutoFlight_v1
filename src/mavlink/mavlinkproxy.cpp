#include "mavlinkproxy.h"

#include <future>
#include <chrono>
#include <sstream>
#include <fstream>

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

bool MAVLinkProxy::saveFlightPlan(std::string path)
{
    if(_mission_ready)
    {
        return false;
    }

    std::ofstream out(path);
    out << _mission;
    out.close();
    return true;
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

    // Response message and buffer
    static mavlink_message_t msg_response;
    static uint8_t buf[MAVLINK_MAX_PACKET_LEN];

    // Waypoint reception variables
    static int waypoints_to_receive = 0;
    static uint16_t waypoints_received = 0;
    static int waypoint_timeout = 200;
    static unsigned long last_waypoint_time = 0;

    for(int i = 0; i < (int) received_bytes; i++)
    {
        if(mavlink_parse_char(MAVLINK_COMM_0, _received_msg_buf[i], &msg, &status))
        {
            // Handle received messages
            msg_response.magic = 0;

            // Waypoint reception
            if(waypoints_to_receive > 0 && (last_waypoint_time + waypoint_timeout <= (std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1))))
            {
                // Timeout
                printf("Timeout receiving waypoints!\n");
                waypoints_to_receive = 0;
                last_waypoint_time = 0;
            }

            if(waypoints_to_receive > 0 && msg.msgid == MAVLINK_MSG_ID_MISSION_ITEM)
            {
                // Got waypoint
                waypoints_received++;
                waypoints_to_receive--;

                mavlink_mission_item_t waypoint;
                mavlink_msg_mission_item_decode(&msg, &waypoint);
                handleWaypoint(&waypoint);

                last_waypoint_time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

                if(waypoints_to_receive > 0)
                {
                    mavlink_mission_request_t request;
                    request.seq = waypoints_received;
                    request.target_system = msg.sysid;
                    request.target_component = msg.compid;

                    mavlink_msg_mission_request_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg_response, &request);
                }
                else
                {
                    mavlink_mission_ack_t ack;
                    ack.target_system = msg.sysid;
                    ack.target_component = msg.compid;
                    ack.type = MAV_MISSION_ACCEPTED;

                    mavlink_msg_mission_ack_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg_response, &ack);

                    processPartialMission();
                }
            }
            else if(msg.msgid == MAVLINK_MSG_ID_MISSION_REQUEST_LIST)
            {
                printf("Waypoint count request\n");
                uint16_t mission_length = 0;

                mavlink_msg_mission_count_pack(_mavlink_system.sysid, _mavlink_system.compid, &msg_response, msg.sysid, msg.compid, mission_length);
            }
            else if(msg.msgid == MAVLINK_MSG_ID_PARAM_REQUEST_LIST)
            {
                printf("Parameter request\n");
                mavlink_param_value_t param;
                param.param_count = 0;
                param.param_id[0] = '\0';
                param.param_index = 0;
                param.param_type = MAV_PARAM_TYPE_UINT8;
                param.param_value = 0;

                mavlink_msg_param_value_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg_response, &param);
            }
            else if(msg.msgid == MAVLINK_MSG_ID_MISSION_COUNT)
            {
                mavlink_mission_count_t mission_count;
                mavlink_msg_mission_count_decode(&msg, &mission_count);
                printf("%d waypoints available\n", mission_count.count);

                _mission_ready = false;
                _partial_mission.clear();
                waypoints_to_receive = mission_count.count;
                waypoints_received = 0;
                last_waypoint_time = std::chrono::system_clock::now().time_since_epoch() / std::chrono::milliseconds(1);

                mavlink_mission_request_t request;
                request.seq = 0;
                request.target_system = msg.sysid;
                request.target_component = msg.compid;

                mavlink_msg_mission_request_encode(_mavlink_system.sysid, _mavlink_system.compid, &msg_response, &request);
            }
            else if(msg.msgid == MAVLINK_MSG_ID_HEARTBEAT)
            {
                // Heartbeat from MAVLink ground station
            }
            else
            {
                // Unhandled message type
                printf("Unknown MAVLink packet: SYS: %d, COMP: %d, LEN: %d, MSG ID: %d\n", msg.sysid, msg.compid, msg.len, msg.msgid);
            }

            // Send a response if the message has been populated
            if(msg_response.magic != 0)
            {
                mavlink_msg_to_send_buffer(buf, &msg_response);
                _socket.send_to(boost::asio::buffer(buf), _endpoint);
            }
        }
    }

    // Listen for next packet
    _socket.async_receive_from(boost::asio::buffer(_received_msg_buf), _endpoint, boost::bind(&MAVLinkProxy::dataReceived, this, boost::asio::placeholders::error, boost::asio::placeholders::bytes_transferred));
}

void MAVLinkProxy::handleWaypoint(mavlink_mission_item_t *waypoint)
{
#define FREEFLIGHT_ACCEPTANCE_RADIUS 5

    // Ensure the waypoint type is compatible with the Bebop and modify parameters if necessary
    uint16_t command;
    double param1 = 0, param2 = 0, param3 = 0, param4 = 0, x = 0, y = 0, z = 0;

    command = waypoint->command;

    switch(command)
    {
    case MAV_CMD_NAV_WAYPOINT:
        // Use the same acceptance radius FreeFlight does, just in case.
        param2 = FREEFLIGHT_ACCEPTANCE_RADIUS;
        param4 = waypoint->param4;
        x = waypoint->x;
        y = waypoint->y;
        z = waypoint->z;
        break;
    case MAV_CMD_NAV_TAKEOFF:
        // FreeFlight FlightPlan does not use takeoff commands for some reason.
        // Convert a takeoff command to a normal waypoint.
        command = MAV_CMD_NAV_WAYPOINT;
        param2 = FREEFLIGHT_ACCEPTANCE_RADIUS;
        param4 = waypoint->param4;
        x = waypoint->x;
        y = waypoint->y;
        z = waypoint->z;
        break;
    case MAV_CMD_NAV_LAND:
        param2 = 0;
        param4 = waypoint->param4;
        x = waypoint->x;
        y = waypoint->y;
        z = waypoint->z;
        break;
    case MAV_CMD_DO_CHANGE_SPEED:
        param2 = waypoint->param2;
        param3 = -1;
        break;
    case MAV_CMD_IMAGE_START_CAPTURE:
        // Just use defaults.
        // RAW: Minimum interval: 8
        // JPEG: Minimum interval: 6.2
        // Snapshot: Minimum interval: 1
        // JPEG 180: Minimum interval 6.2
        param1 = 10;
        param2 = 0;
        param3 = 14;
        break;
    case MAV_CMD_IMAGE_STOP_CAPTURE:
        // Leave everything at 0
        break;
    case MAV_CMD_VIDEO_START_CAPTURE:
        param2 = 30; // 30FPS
        param3 = 2073600; // Mystery parameter used by FreeFlight
    case MAV_CMD_VIDEO_STOP_CAPTURE:
        // Leave everything at 0
        break;
    case MAV_CMD_PANORAMA_CREATE:
        param1 = waypoint->param1;
        if(waypoint->param3 >= 180) // Max 180
        {
            param3 = 180;
        }
        else if(waypoint->param3 <= 5) // Min 5
        {
            param3 = 5;
        }
        else
        {
            param3 = waypoint->param3;
        }

        break;
    default:
        printf("Unsupported command type %d\n", command);
        return;
    }

    _partial_mission.push_back({0, 3, command, param1, param2, param3, param4, x, y, z});
}

void MAVLinkProxy::processPartialMission()
{
    // FreeFlight missions always begin with this change speed command. Add it, just in case.
    _partial_mission.insert(_partial_mission.begin(), {0, 3, MAV_CMD_DO_CHANGE_SPEED, 0, 6, -1, 0, 0, 0});

    // Check that pictures and video are not being taken at the same time
    bool video = false;
    bool pictures = false;
    for(int i = 0; i < _partial_mission.size(); i++)
    {
        switch(_partial_mission[i].command)
        {
        case MAV_CMD_IMAGE_START_CAPTURE:
            pictures = true;
            break;
        case MAV_CMD_IMAGE_STOP_CAPTURE:
            pictures = false;
            break;
        case MAV_CMD_VIDEO_START_CAPTURE:
            video = true;
            break;
        case MAV_CMD_VIDEO_STOP_CAPTURE:
            video = false;
            break;
        default:
            break;
        }

        if(video && pictures)
        {
            // Remove the current item because it would mean video and pictures must be taken at the same time
            _partial_mission.erase(_partial_mission.begin() + i);
        }
    }

    // Add stop video/stop picture command if video/pictures are still being taken at the end of the mission
    if(video)
    {
        _partial_mission.push_back({0, 3, MAV_CMD_VIDEO_STOP_CAPTURE, 0, 0, 0, 0, 0, 0});
    }

    if(pictures)
    {
        _partial_mission.push_back({0, 3, MAV_CMD_IMAGE_STOP_CAPTURE, 0, 0, 0, 0, 0, 0});
    }

    // Format mission into string
    std::stringstream mission;

    mission << std::fixed << std::setprecision(6);

    mission << "QGC WPL 120\n";

    for(int i = 0; i < _partial_mission.size(); i++)
    {
        mission << i << "\t";
        mission << _partial_mission[i].curr_wp << "\t";
        mission << _partial_mission[i].coord_frame << "\t";
        mission << _partial_mission[i].command << "\t";
        mission << _partial_mission[i].param1 << "\t";
        mission << _partial_mission[i].param2 << "\t";
        mission << _partial_mission[i].param3 << "\t";
        mission << _partial_mission[i].param4 << "\t";
        mission << _partial_mission[i].x << "\t";
        mission << _partial_mission[i].y << "\t";
        mission << _partial_mission[i].z << "\t";
        mission << 1 << "\n";
    }

    _partial_mission.clear();
    _mission = mission.str();
    _mission_ready = true;

    for(IFlightPlanListener *l : _listeners)
    {
        l->flightPlanAvailable(_mission);
    }
}

void MAVLinkProxy::addFlightPlanListener(IFlightPlanListener *l)
{
    _listeners.push_back(l);
}

void MAVLinkProxy::removeFlightPlanListener(IFlightPlanListener *l)
{
    _listeners.erase(remove(_listeners.begin(), _listeners.end(), l), _listeners.end());
}
