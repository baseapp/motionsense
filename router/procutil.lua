#!/usr/bin/lua

package.path = package.path .. ";/usr/lib/basetrack/?.lua"

require "posix"

local P = {}
procutil = P

--returns pid, sid, error message
function procutil.daemonize()
	local cpid = posix.fork()
	if cpid == 0 then
		-- fork again
		local cpid_ = posix.fork()
		if cpid_ ~= 0 then
			posix._exit(0)
		end
		-- this is child of second fork
		local devnull = posix.open("/dev/null", posix.O_RDWR)
		posix.dup2(devnull, 0)
		posix.dup2(devnull, 1)
		posix.dup2(devnull, 2)
		local sid = posix.setpid('s')
		if sid == nil then
			return cpid, sid,"error in setsid"
		end
		--main body
		return cpid, sid, "child"
	end
	if cpid > 0 then
		while posix.wait(cpid) ~= nil do end
		return cpid, 0, "parent" --forked and setsid succesfully
	else
		return cpid, 0, "error forking"
	end
end

--returns pid, error message
function procutil.fork()
	local cpid = posix.fork()
	if cpid == 0 then
		--if child process
		return cpid, "child"
	end
	if cpid > 0 then
		return cpid, "parent" --forked and setsid succesfully
	else
		return cpid, "error forking"
	end
end

-- return true, fd on success, false otherwise
function procutil.getlock(lockfilename)
	local fd = posix.open(lockfilename, posix.O_WRONLY + posix.O_CREAT, "+rw")
	if not fd then
		return false 
	end
	-- Set lock on file
	local lock = {
		l_type = posix.F_WRLCK;     -- Exclusive lock
		l_whence = posix.SEEK_SET;  -- Relative to beginning of file
		l_start = 0;            -- Start from 1st byte
		l_len = 0;              -- Lock whole file
	}
	local result = posix.fcntl(fd, posix.F_SETLK, lock)
	if result == nil then
		return false
	end
	return true, fd
end

-- releases lock on file, fd is file descriptor(int)
function procutil.releaselock(fd)
	-- Release the lock
	local lock = {
		l_type = posix.F_UNLCK;     -- release lock
		l_whence = posix.SEEK_SET;  -- Relative to beginning of file
		l_start = 0;            -- Start from 1st byte
		l_len = 0;              -- Lock whole file
	}
	posix.fcntl(fd, posix.F_SETLK, lock)
end

-- command is program name, params is table of params
function procutil.getoutput(command, params)
	local r,w = posix.pipe()
	local cpid = posix.fork()
	if cpid == 0 then --child writes to pipe
		--close unused read end
		local devnull = posix.open("/dev/null", posix.O_RDWR)
		posix.close(r)
		posix.dup2(devnull, 0)
		posix.dup2(w, 1)
		posix.dup2(devnull, 2)
	    posix.exec(command, params)
	    posix._exit(-1)
    elseif cpid > 0 then
    	--parent reads from pipe, close write end
    	posix.close(w)
    	local buf = ''
    	while true do
    		local tmp = posix.read(r, 100)
    		if tmp ~= nil and #tmp > 0 then
    			buf = buf .. tmp
    		else
    			break
    		end
    	end
    	-- TODO, check exit value, to see if entry exists or not
    	while posix.wait(cpid) ~= nil do end
    	posix.close(r)
    	return buf
    end
end

return procutil