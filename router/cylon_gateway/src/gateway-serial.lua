#!/usr/bin/lua

package.path = package.path .. ";/usr/bin/cylon/?.lua"

local bit = require("bit")
local json = require "dkjson"
require "posix"
require "request"
require "procutil"

-- does the actual upload(after forking), then calls handle_response
function upload(upload_table)
	print("Upload")
	if (type(upload_table) ~= "table") then
		print("expected table, is ", type(upload_table))
	end

	-- if time since uploaded > 10s and upload_table not empty, upload(function called even if mac etc are nil)
	if next(upload_table, nil) ~= nil then

		-- base_url = "http://172.16.0.9:800/"
		-- base_url = "https://httpbin.org/" endpoint = '/post'
		base_url = upload_site
		
		-- POST request with form
		print(base_url)
		
		local req_body = json.encode(upload_table, {indent=true})
		
		local headers = {
		-- ["Content-Type"] = "application/x-www-form-urlencoded";
		["Content-Type"] =  "application/json";
		["Content-Length"] = #req_body;
		}

		local retval = request.http_request{base_url=base_url, method="POST", source=ltn12.source.string(req_body), headers=headers}
		request.deep_print(retval)
		local res = retval.response[1]
		-- response.handle(res)
		-- request.http_request{base_url=base_url, endpoint=endpoint, method="POST", source=ltn12.source.string(req_body), headers=headers}
	end
end

function parse_dump(str)
	if not str then return end
	
	if remainder then 
		str = remainder .. str 
		-- print("REMAINDER", #remainder, remainder) 
	end
	remainder = string.match(str, "([^\n\r]+)$")

	-- remove remainder from string
	local removed_len
	if remainder then
		removed_len = #remainder
	else 
		removed_len = 0
	end
	str = string.sub(str, 1, #str - removed_len)
	
	for line in string.gmatch(str, "([^\n\r]+)[\n\r]+") do
		if #line > 0 then
			print("PACKET", #line, line)
			table.insert(packets_list, line)
		end
	end
end

function read_output(fd)
	return posix.read(fd, 256)
end

------------------------------------------------------------------------------
-- using this file
local port = '/dev/ttyATH0'
-- os.execute('/bin/stty -F ' .. port ..' 115200')
ser = posix.open(port, bit.bor(posix.O_RDONLY, posix.O_NONBLOCK))
if not ser then
	print("couldn't open file")
	posix._exit(-1)
end

-- remainder
remainder = '\n'
-- last upload_time for uploading in chunks
last_upload_time = posix.time()
-- upload at this many seconds interval
upload_time = 30
--stores list of packets
packets_list = {}
-- the buffer that is uploaded
upload_table = {}
-- url to post to
upload_site = "http://devhu.com/cylon"

while true do
	parse_dump(read_output(ser))

	posix.wait(-1, posix.WNOHANG)

	if (posix.time() - last_upload_time) > (upload_time) then
		-- upload updates last_upload_time
		last_upload_time = posix.time()
		
		-- fill buffer to be uploaded as json
		upload_table["mac"] = self_mac
		upload_table["packets"] = packets_list
		
		-- change daemonize to fork to see output
		cpid = procutil.fork()

		if cpid == 0 then
			-- is child
			upload(upload_table)
			os.exit()
		end
		packets_list = {}
		upload_table = {}
	end

	posix.nanosleep(0, 30000000)
end