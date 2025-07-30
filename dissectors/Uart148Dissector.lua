-- L I C E N S E --------------------------------------------------------------
--
-- MIT License
-- 
-- Copyright (c) 2025 michael0710
-- 
-- Permission is hereby granted, free of charge, to any person obtaining a copy
-- of this software and associated documentation files (the "Software"), to
-- deal in the Software without restriction, including without limitation the
-- rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
-- sell copies of the Software, and to permit persons to whom the Software is
-- furnished to do so, subject to the following conditions:
-- 
-- The above copyright notice and this permission notice shall be included in
-- all copies or substantial portions of the Software.
-- 
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
-- AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
-- LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
-- FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
-- IN THE SOFTWARE.
--
-------------------------------------------------------------------------------

---------------------------------------------------------------------------------------------------
-- variable and function definitions for the logging component ------------------------------------

-- Define constants to be used within this script. Changing this constants during operation can
-- lead to unexpected behaviour
-- local SeverityType_STRINGS = {"VERBOSE", "DEBUG  ", "INFO   ", "WARNING", "ERROR  ", "FAILURE"}
-- local SeverityType_VERBOSE = 1
-- local SeverityType_DEBUG   = 2
-- local SeverityType_INFO    = 3
-- local SeverityType_WARNING = 4
-- local SeverityType_ERROR   = 5
-- local SeverityType_FAILURE = 6
-- -- configuration variables for the logging component
-- local LogFilename = "C:/Users/Michael/Documents/Bastel_Projekte/CAPTURino/logs/DebugDissector.log"
-- local SeverityLogLimit = SeverityType_VERBOSE
-- 
-- local function WriteLog(severity, logmsg)
--     if (severity >= SeverityLogLimit) then
--         local LogFile     = io.open(LogFilename, "a")
--         LogFile:write(os.date() .. " :: " .. SeverityType_STRINGS[severity] .. " :: " .. logmsg .. "\n")
--         LogFile:close()
--     end
-- end

---------------------------------------------------------------------------------------------------
-- local function definitions ---------------------------------------------------------------------

---- Define the menu entry's callback
--local function dialog_menu()
--    WriteLog(SeverityType_VERBOSE, "function dialog_menu() called");
--    local function dialog_func(person,eyes,hair)
--        local window = TextWindow.new("Person Info");
--        local message = string.format("Person %s with %s eyes and %s hair.", person, eyes, hair);
--        window:set(message);
--    end
--
--    new_dialog("Dialog Test",dialog_func,"A Person","Eyes","Hair")
--end

function flip_bits_16 (a)
    z = 0
    z = z | ((a & 0x8000) >> 15) | ((a & 0x0001) << 15)
    z = z | ((a & 0x4000) >> 13) | ((a & 0x0002) << 13)
    z = z | ((a & 0x2000) >> 11) | ((a & 0x0004) << 11)
    z = z | ((a & 0x1000) >>  9) | ((a & 0x0008) <<  9)
    z = z | ((a & 0x0800) >>  7) | ((a & 0x0010) <<  7)
    z = z | ((a & 0x0400) >>  5) | ((a & 0x0020) <<  5)
    z = z | ((a & 0x0200) >>  3) | ((a & 0x0040) <<  3)
    z = z | ((a & 0x0100) >>  1) | ((a & 0x0080) <<  1)
    return z
end

function flip_bits_5 (a)
    z = 0
    z = z | ((a & 0x10) >> 4) | ((a & 0x01) << 4)
    z = z | ((a & 0x08) >> 2) | ((a & 0x02) << 2)
    z = z |  (a & 0x04)
    return z
end

function flip_bits_6 (a)
    z = 0
    z = z | ((a & 0x20) >> 5) | ((a & 0x01) << 5)
    z = z | ((a & 0x10) >> 3) | ((a & 0x02) << 3)
    z = z | ((a & 0x08) >> 1) | ((a & 0x04) << 1)
    return z
end

function flip_bits_7 (a)
    z = 0
    z = z | ((a & 0x40) >> 6) | ((a & 0x01) << 6)
    z = z | ((a & 0x20) >> 4) | ((a & 0x02) << 4)
    z = z | ((a & 0x10) >> 2) | ((a & 0x04) << 2)
    z = z |  (a & 0x08)
    return z
end

function flip_bits_8 (a)
    z = 0
    z = z | ((a & 0x80) >> 7) | ((a & 0x01) << 7)
    z = z | ((a & 0x40) >> 5) | ((a & 0x02) << 5)
    z = z | ((a & 0x20) >> 3) | ((a & 0x04) << 3)
    z = z | ((a & 0x10) >> 1) | ((a & 0x08) << 1)
    return z
end

function is_timeout_frame(uintdata)
    return (uintdata & (1<<15)) ~= 0x0
end

function get_frame_data(buffer)
    local frame_length_bit = buffer(0,1):uint()
    local frame_info = buffer(1,1):uint()
    local captured_data = buffer(2,2):uint()
    local bits_after_data = 1
 
    if ((frame_info & 0xf0) ~= (0 << 4)) then
	-- any kind of parity is used
	bits_after_data = bits_after_data + 1
    end
    if ((frame_info & 0x0f) == 0x4) then
	-- a second stoppbit is used
	bits_after_data = bits_after_data + 1
    end
    
    if (frame_length_bit == 5) then
	return flip_bits_5((captured_data >> bits_after_data) & 0x1f)
    elseif (frame_length_bit == 6) then
	return flip_bits_6((captured_data >> bits_after_data) & 0x3f)
    elseif (frame_length_bit == 7) then
	return flip_bits_7((captured_data >> bits_after_data) & 0x7f)
    elseif (frame_length_bit == 8) then
        return flip_bits_8((captured_data >> bits_after_data) & 0xff)
    end
end

function add_raw_data_2_tree(subtree, buffer)
    local captured_data = buffer(2,2):uint()
    subtree:add_le(buffer(2,2), string.format("Captured rawdata:"))
    subtree:add_le(buffer(2,2), "  " .. ((captured_data >> 15) & 0x01) .. "... .... .... ....  Timeout Flag")
    subtree:add_le(buffer(2,2), "  ." .. ((captured_data >> 14) & 0x01)
	    			     .. ((captured_data >> 13) & 0x01)
				     .. ((captured_data >> 12) & 0x01)
			     .. " "  .. ((captured_data >> 11) & 0x01)
			     	     .. ((captured_data >> 10) & 0x01)
				     .. ((captured_data >>  9) & 0x01)
				     .. ((captured_data >>  8) & 0x01)
	                     .. " "  .. ((captured_data >>  7) & 0x01)
				     .. ((captured_data >>  6) & 0x01)
			             .. ((captured_data >>  5) & 0x01)
		                     .. ((captured_data >>  4) & 0x01)
			     .. " "  .. ((captured_data >>  3) & 0x01)
			             .. ((captured_data >>  2) & 0x01)
				     .. ((captured_data >>  1) & 0x01)
				     .. ((captured_data >>  0) & 0x01) .. "  Content")
end

-- declare our protocol
uart_proto = Proto("uart","UART Protocol")
mbreass_proto = Proto("mbreass","MODBUS Reassembly Protocol")

local uart_fields =
{
    data = ProtoField.uint8("uart.data", "Data", base.HEX)
}

uart_proto.fields = uart_fields

uart_proto.prefs.dissecthighlvl = Pref.bool("Dissect High-Level Protocol", false, "Specify if the dissector shall hand over the captured data to any higher-level dissector")

local DISSECT_AS_MODBUSRTU = 0;
local DISSECT_AS_MODBUSASCII = 1;

local highlvl_protos_pref = {
    { 1, "Modbus RTU",   DISSECT_AS_MODBUSRTU   },
    { 2, "Modbus ASCII", DISSECT_AS_MODBUSASCII },
}
uart_proto.prefs.highlvl2dissect = Pref.enum(
    "High-Level Protocol",
    DISSECT_AS_MODBUSRTU,
    "High-level protocol that shall be dissected",
    highlvl_protos_pref,
    false
)

-- create a function to dissect it
function uart_proto.dissector(buffer,pinfo,tree)
--    if (uart_proto.prefs.dissecthighlvl == true) then
--	dissect_highlvl_protocol(buffer, pinfo, tree)
--    else
	pinfo.cols.protocol = "UART"
	captured_frame_length = buffer(0,1):uint()
	captured_frame_info = buffer(1,1):uint()
	captured_data = buffer(2,2):uint()
	time_to_prev_frame = buffer(4,4):uint()

	if (is_timeout_frame(captured_data)) then
	    dissect_timeout_frame(captured_frame_length, captured_frame_info, captured_data, time_to_prev_frame, buffer, pinfo, tree);
	else
	    dissect_data_frame(captured_frame_length, captured_frame_info, captured_data, time_to_prev_frame, buffer, pinfo, tree);
	end
--    end
 
    if (uart_proto.prefs.dissecthighlvl == true) then
	dissect_highlvl_protocol(buffer, pinfo, tree)
    end
end

function dissect_timeout_frame(captured_frame_length, captured_frame_info, captured_data, time_to_prev_frame, buffer, pinfo, tree);
    pinfo.cols.info = "Rx timeout was exceeded after last frame"
    local subtree = tree:add(uart_proto,buffer(), "UART Timeout")
    subtree:add_le(buffer(0,8), "Rx timeout was exceeded after last frame")
    subtree:add_le(buffer(4,4), "Timeout value was " .. (time_to_prev_frame+.0)/100 .. "us")
    add_raw_data_2_tree(subtree, buffer)
end

function dissect_data_frame(captured_frame_length, captured_frame_info, captured_data, time_to_prev_frame, buffer, pinfo, tree)
    parity_string = ""
    if ((captured_frame_info & 0xf0) == (0 << 4)) then
	parity_string = "none"
    elseif ((captured_frame_info & 0xf0) == (1 << 4)) then
	parity_string = "odd"
    elseif ((captured_frame_info & 0xf0) == (2 << 4)) then
	parity_string = "even"
    elseif ((captured_frame_info & 0xf0) == (3 << 4)) then
	parity_string = "stick low"
    elseif ((captured_frame_info & 0xf0) == (4 << 4)) then
	parity_string = "stick high"
    end
    
    time_to_large_char = ""
    if time_to_prev_frame >= 0xffffffff then
        time_to_large_char = ">"	
    end

    flipped_captured_data = get_frame_data(buffer)

    pinfo.cols.info = "Data: " ..
                      string.format("0x%02x", flipped_captured_data) .. 
		      ", Time to previous frame: " ..
		      time_to_large_char ..
		      tostring((time_to_prev_frame + .0)/100000000 .. "s")
    local subtree = tree:add(uart_proto,buffer(),"UART Protocol Data")
    subtree:add_le(buffer(0,1), "Databits: " .. captured_frame_length)
    subtree:add_le(buffer(1,1), "Parity: " .. parity_string)
    subtree:add_le(buffer(1,1), string.format("Stopbits: %.1f", ((captured_frame_info & 0x0f) + 0.0)/2.0))
    subtree:add_le(buffer(2,2), string.format("Data content: %02x ", flipped_captured_data))
    add_raw_data_2_tree(subtree, buffer)
    tree:add(uart_fields.data, flipped_captured_data)
    --[[(subtree:add_le(buffer(2,2), string.format("Captured rawdata:"))
    subtree:add_le(buffer(2,2), "  " .. ((captured_data >> 15) & 0x01) .. "... .... .... ....  Timeout Flag")
    subtree:add_le(buffer(2,2), "  ." .. ((captured_data >> 14) & 0x01)
	    			     .. ((captured_data >> 13) & 0x01)
				     .. ((captured_data >> 12) & 0x01)
			     .. " "  .. ((captured_data >> 11) & 0x01)
			     	     .. ((captured_data >> 10) & 0x01)
				     .. ((captured_data >>  9) & 0x01)
				     .. ((captured_data >>  8) & 0x01)
	                     .. " "  .. ((captured_data >>  7) & 0x01)
				     .. ((captured_data >>  6) & 0x01)
			             .. ((captured_data >>  5) & 0x01)
		                     .. ((captured_data >>  4) & 0x01)
			     .. " "  .. ((captured_data >>  3) & 0x01)
			             .. ((captured_data >>  2) & 0x01)
				     .. ((captured_data >>  1) & 0x01)
				     .. ((captured_data >>  0) & 0x01) .. "  Content")) ]]
end

function dissect_highlvl_protocol(buffer, pinfo, tree)
    if (uart_proto.prefs.highlvl2dissect == DISSECT_AS_MODBUSRTU) then
	dissect_modbusrtu_protocol(buffer, pinfo, tree)
    elseif (uart_proto.prefs.highlvl2dissect == DISSECT_AS_MODBUSASCII) then
	dissect_modbusascii_protocol(buffer, pinfo, tree)
    end
end

-- TODO implement this sequence in the UART provider!
-- hackyreassemblybuffer = {0x01, 0x04, 0x02, 0xFF, 0xFF, 0xB8, 0x80}
local hackyreassemblybuffer = {}
--hackyTempBuf = ByteArray.new()
--hackyTempBuf:set_size(0)

function dissect_modbusrtu_protocol(buffer, pinfo, tree)
    --if (pinfo.visited == false) then
    if (is_timeout_frame(buffer(2,2):uint())) then
	-- take the stored data and hand it over to the modbus dissector
	-- clear the stored data
	
    	local mbrtu_dissector = Dissector.get("mbrtu")
	local hackyTempBuf = ByteArray.new()
	hackyTempBuf:set_size(#hackyreassemblybuffer)
	for i=1,#hackyreassemblybuffer,1 do
	    hackyTempBuf:set_index(i-1, hackyreassemblybuffer[i])
	end
	mbdatatvb = hackyTempBuf:tvb("Modbus RTU")
	-- add a failure warning to the info field
	-- if the modbus dissector succeeds, it will be overwritten by that information
	-- if the modbus dissector fails, it writes nothing to the info field, so the
	-- failure message appears
	pinfo.cols.info:prepend("[FAILED to dissect MODBUS frame] ")
	pinfo.cols.info:append("reassembled data: " .. table.concat(hackyreassemblybuffer, " "))
        local subtree = tree:add(uart_proto,buffer(),"Reassembled MODBUS Data")
        --subtree:add_le("Databytes: " .. table.concat(hackyreassemblybuffer))
        subtree:add_le("Databytes: " .. tostring(hackyTempBuf))
	hackyTempBuf = nil
	--mbrtu_dissector:call(mbdatatvb, pinfo, tree)
	--if(mbrtu_dissector:call(mbdatatvb, pinfo, tree) == 0) then
	    -- dissection failed
	--end
	hackyreassemblybuffer = {}
    else
  	    -- store the received data to the previously received data
	    local msg_content = get_frame_data(buffer)
	    table.insert(hackyreassemblybuffer, msg_content)
	    pinfo.cols.info:prepend("[frame saved for MODBUS reassembly] ")
	    --local subtree = tree:add(uart_proto,buffer(),"MODBUS reassembly data")
	    --subtree:add_le(buffer(0,8), "hackyBufStr: " .. table.concat(hackyreassemblybuffer, ", ")) --tostring(hackyTempBuf))
    end
    --end
end

function dissect_modbusascii_protocol(buffer, pinfo, tree)

end

function mbreass_proto.dissector(buffer, pinfo, tree)

end

-- Register the dissector for LINKTYPE_USER0
local wtap_encap_table = DissectorTable.get("wtap_encap")
wtap_encap_table:add(wtap.USER1, uart_proto)
--wtap_encap_table

--[[
---------------------------------------------------------------------------------------------------
-- beginning of the script ------------------------------------------------------------------------
--
--WriteLog(SeverityType_INFO, "*** Lua script for CapturinoDissector started ***")
--WriteLog(SeverityType_INFO, "Wireshark version " .. get_version())
--
---- Notify the user that the menu was created
if gui_enabled() then
--    WriteLog(SeverityType_VERBOSE, "GUI is enabled");
--   
--    -- Create the menu entry
--    register_menu("Lua Dialog Test",dialog_menu,MENU_TOOLS_UNSORTED)
--    WriteLog(SeverityType_VERBOSE, "menu entry 'Lua Dialog Test' registered")
--    
--    --WriteLog(SeverityType_INFO, "proto_can.prefs.byte_swap = " .. proto_can.prefs.byte_swap)
--
    local splash = TextWindow.new("Hello!");
    splash:set("Wireshark has been enhanced with a useless feature.\n")
    splash:append("Go to 'Tools->Lua Dialog Test' and check it out!\n")
--    
--    
    local dissectList = Dissector.list()
    splash:append("Dissectors found:")
    splash:append(#dissectList)
    splash:append("\n")
    for i=1, #dissectList do
        splash:append(dissectList[i])
        splash:append("\n")
    end
--else
--    WriteLog(SeverityType_ERROR, "GUI is not enabled. Script will be stopped!")
end
]]

