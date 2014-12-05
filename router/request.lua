#!/usr/bin/lua

local P = {}

request = P

local https = require("ssl.https")
local http  = require("socket.http")
local url   = require("socket.url")
local ltn12 = require("ltn12")

--Helper for priniting nested table
function request.deep_print(tbl)
    for i, v in pairs(tbl) do
        if type(v) == "table" then 
            request.deep_print(v)
        else 
            print(i, v) 
        end
    end
end


function request.http_request( args )
--http.request(url [, body])
--http.request{
--  url = string,
--  [sink = LTN12 sink,]
--  [method = string,]
--  [headers = header-table,]
--  [source = LTN12 source],
--  [step = LTN12 pump step,]
--  [proxy = string,]
--  [redirect = boolean,]   
--  [create = function]
--}
--
--
    local resp, r = {}, {}
    
    local params = ""
    if args.method == nil or args.method == "GET" then
        -- prepare query parameters like http://xyz.com?q=23&a=2
        if args.params then
            for i, v in pairs(args.params) do
                params = params .. i .. "=" .. v .. "&"
            end
        end
    end
    params = string.sub(params, 1, -2)

    local parsed_url = url.parse(args.base_url)
    -- parsed_url = {
    --   scheme = "http",
    --   authority = "www.example.com",
    --   path = "/cgilua/index.lua"
    --   query = "a=2",
    --   fragment = "there",
    --   host = "www.puc-rio.br",
    -- }

    if parsed_url.scheme == 'http' then
        local client, code, headers, status = http.request{  
                                                        url=args.base_url, sink=ltn12.sink.table(resp),
                                                        method=args.method or "GET", headers=args.headers, source=args.source,
                                                        step=args.step, proxy=args.proxy, redirect=args.redirect, create=args.create 
                                                    }
    elseif parsed_url.scheme == 'https' then
        local client, code, headers, status = https.request{  
                                                        url=args.base_url, sink=ltn12.sink.table(resp),
                                                        method=args.method or "GET", headers=args.headers, source=args.source,
                                                        step=args.step, proxy=args.proxy, redirect=args.redirect, create=args.create 
                                                    }
    else
        return nil
    end

    r['code'], r['headers'], r['status'], r['response'] = code, headers, status, resp

    return r
end

-- ignores max_size right now, TODO
function request.download(base_url, path, maxsize)
    local parsed_url = url.parse(base_url)

    local resp = {}

    if parsed_url.scheme == 'http' then
        local client, code, headers, status = http.request{  
                                                        url=base_url, sink=ltn12.sink.table(resp),
                                                        method="GET" 
                                                    }
        print(#resp, headers, code, status)
    elseif parsed_url.scheme == 'https' then
        local client, code, headers, status = https.request{  
                                                        url=base_url, sink=ltn12.sink.table(resp),
                                                        method="GET"
                                                    }
        print(#resp, headers, code, status)
    else
        return nil
    end

    local f,err = io.open(path,"w")
    if not f then return print(err) end
    for k, v in ipairs(resp) do
        f:write(v)
    end

    f:close()

end


function request.test()
	base_url = "https://httpbin.org/"
    -- Normal GET request
    endpoint = "/user-agent"
    print(endpoint)
    request.deep_print(request.http_request{base_url=base_url, endpoint=endpoint})
    -- GET request with parameters
    endpoint = "/get"
    print(endpoint)
    request.deep_print(request.http_request{base_url=base_url, endpoint=endpoint, params={age=23, name    ="kracekumar"}})
    -- POST request with form
    endpoint = "/post"
    print(endpoint)
    local req_body = "a=2"
    local headers = {
    ["Content-Type"] = "application/x-www-form-urlencoded";
    ["Content-Length"] = #req_body;
    }
    request.deep_print(request.http_request{base_url=base_url..endpoint, method="POST", source=ltn12.source.string(req_body), headers=headers})
    -- PATCH Method
    endpoint = "/patch"
    print(endpoint)
    request.deep_print(request.http_request{base_url=base_url..endpoint, method="PATCH"})
    -- PUT Method
    endpoint = "/put"
    print(endpoint)
    request.deep_print(request.http_request{base_url=base_url..endpoint, method="PUT", source    =ltn12.source.string("a=23")})
    -- Delete method
    endpoint = "/delete"
    print(endpoint)
    request.deep_print(request.http_request{base_url=base_url..endpoint, method="DELETE",     source=ltn12.source.string("a=23")})

end

-- request.deep_print(request.http_request{base_url="https://parabolagnulinux.org/"})
-- request.deep_print(request.http_request{base_url="http://parabolagnulinux.org/"})


return request