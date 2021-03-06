function [arr] = thermal_parseMessage(msg)
% [str] = gps_readMessage_ubx(msg)
%
% Parses a UBX GPS message and returns the data. Currently only
% works with Navigational Geodetic Position messages (1,2).
%
% Arguments:
%   msg: a raw UBX GPS message
%
% Returns:
%   an array containing each field
%
% Messages:
%   NAV-POSLLH (0x01 0x02)
%   --
%   iTow: GPS millisecond time of week (ms)
%   lon: longitude (deg)
%   lat: latitude (deg)
%   height: height above ellipsoid (mm)
%   hMSL: height above mean sea level (mm)
%   hAcc: horizontal accuracy estimate (mm)
%   vAcc: vertical accuracy estimate (mm)
%

DEBUG = 1;
PAYLOAD_START_INDEX = 7;
pS = PAYLOAD_START_INDEX; % shorthand

if nargin < 1
    error('Missing argument ''msg''')
elseif isstr(msg{1})
    error('Expected ''msg'' as integers, but got strings.');
end
id = [msg{3} msg{4}];
arr = [];

% Navigation messages
if id(1) == 1
    %  NAV-POSLLH (0x01 0x02) - Geodetic Position Solution Message
    if id(2) == 2
        % iTow (ms) uint32_t - GPS Millisecond Time of Week
        arr(1) = uint32(bitshift(msg{pS + 3}, 24) + bitshift(msg{pS + 2}, 16) + bitshift(msg{pS + 1}, 8) + msg{pS});
        % lon (deg) int32_t - Longitude scaled 1e-7
        arr(2) = convertFromTwosComp32(bitshift(msg{pS + 7}, 24) + bitshift(msg{pS + 6}, 16) + bitshift(msg{pS + 5}, 8) + msg{pS + 4});
        % lat (deg) int32_t - Latitude scaled 1e-7
        arr(3) = convertFromTwosComp32(bitshift(msg{pS + 11}, 24) + bitshift(msg{pS + 10}, 16) + bitshift(msg{pS + 9}, 8) + msg{pS + 8});
        % height (mm) int32_t - Height above Ellipsoid
        arr(4) = convertFromTwosComp32(bitshift(msg{pS + 15}, 24) + bitshift(msg{pS + 14}, 16) + bitshift(msg{pS + 13}, 8) + msg{pS + 12});
        % hMSL (mm) int32_t - Height above Mean Sea Level
        arr(5) = convertFromTwosComp32(bitshift(msg{pS + 19}, 24) + bitshift(msg{pS + 18}, 16) + bitshift(msg{pS + 17}, 8) + msg{pS + 16});
        % hAcc (mm) uint32_t - Horizontal Accuracy Estimate
        arr(6) = uint32(bitshift(msg{pS + 23}, 24) + bitshift(msg{pS + 22}, 16) + bitshift(msg{pS + 21}, 8) + msg{pS + 20});
        % vAcc (mm) uint32_t - Vertical Accuracy Estimate
        arr(7) = uint32(bitshift(msg{pS + 27}, 24) + bitshift(msg{pS + 26}, 16) + bitshift(msg{pS + 25}, 8) + msg{pS + 24});
    elseif id(2) == 3
        % iTow (ms) uint32_t - GPS Millisecond Time of Week
        arr(1) = uint32(bitshift(msg{pS + 3}, 24) + bitshift(msg{pS + 2}, 16) + bitshift(msg{pS + 1}, 8) + msg{pS});
        % gpsFix (enum) uint8_t - GPSfix Type
        arr(2) = uint8(msg{pS + 4});
        % flags (bitfield) uint8_t - Navigation Status Flags
        arr(3) = msg{pS + 5};
        % diffStat (bitfield) int8_t - Differential Status
        arr(4) = uint8(msg{pS + 6});
        % res  - Reserved
        arr(5) = uint8(msg{pS + 7});
        % ttff (ms) uint32_t - Time to first fix (millisecond time tag)
        arr(6) = uint32(bitshift(msg{pS + 11}, 24) + bitshift(msg{pS + 10}, 16) + bitshift(msg{pS + 9}, 8) + msg{pS + 8});
        % msss (ms) uint32_t - Milliseconds since Startup / Reset
        arr(7) = uint32(bitshift(msg{pS + 15}, 24) + bitshift(msg{pS + 14}, 16) + bitshift(msg{pS + 13}, 8) + msg{pS + 12});
    else
        error(sprintf('Navigation message 0x%X not implemented.', id(2)));
    end
else
    error(sprintf('Message ID 0x%X 0x%X not implemented.',msg(3),msg(4)));
end

end % function

