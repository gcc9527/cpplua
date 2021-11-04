
tools = {}



function tools.luaGc(curTime)
	if tools.time and curTime then
		return
	end

	tools.time = curTime + 180
	collectgarbage("collect")
	logInfo("tools.luaGc excute lua gc collect")
end

local function GetTableFStr(...)
	local arg = {...}	
	local t = arg[1]
	if not t then return end
	local tp = type(t)
	if "table" == tp then
		local prefix = arg[2]
		if not prefix then
			prefix = ""
		end
		local minor_prefix = arg[3]
		if not minor_prefix then
			minor_prefix = "    "
		end
		local path = arg[4]
		if not path or "table" ~= type(path) then
			path = {}
		end
		-- 这里进行一次深拷贝
		local root = {}
		for k, v in pairs(path) do
			root[k] = v
		end
		table.insert(root, t)
		local indexstr, resultstr = "", prefix.."{".."\n"
		local pass = false
		for k, v in pairs(t) do
			if "string" == type(k) then
				indexstr = string.format("%s[\"%s\"]", prefix..minor_prefix, k)
			elseif "number" == type(k) then
				indexstr = string.format("%s[%d]", prefix..minor_prefix, k)
			end
			local ttp = type(v)
			if "table" == ttp then
				resultstr = resultstr..indexstr.."="
				pass = true
				for key, val in pairs(root) do
					if v == val then
						pass = false
						resultstr = resultstr.."\n"
						break
					end
				end
				if pass and "_G" ~= k then
					local ret = GetTableFStr(v, prefix..minor_prefix, minor_prefix, root)
					resultstr = resultstr.."\n"..ret
				end
			else
				if "string" == ttp then
					resultstr = resultstr..indexstr.."=".."\""..v.."\""
				elseif "function" == ttp then
					resultstr = resultstr..indexstr.."="..tostring(v)
				elseif "userdata" == ttp then
					resultstr = resultstr..indexstr.."="..tostring(v)
				elseif "number" == ttp then
					resultstr = resultstr..indexstr.."="..string.format("%d",v)
				elseif "boolean" == ttp then
					resultstr = resultstr..indexstr.."="..string.format("%s", tostring(v))
				end
				resultstr = resultstr
			end
			resultstr = resultstr.."\n"
		end
		resultstr = resultstr..prefix.."}\n"
		return resultstr
	else
		if "string" == type(t) then
			return "\""..t.."\""
		else
			return tostring(t)
		end
	end
end

local function get_table_fstr(t, ...)
	local param = {...}

	local result
	if "table" == type(t) then
		if not param[1] then
			result=tostring(t).."=\n"
		else
			result=tostring(param[1])..tostring(t).."=\n"
		end
		return result..GetTableFStr(t, param[1], param[2])
	else
		return t, ...
	end
end


tools.pt = function(t, ... )
	logDebug(get_table_fstr(t, ...))
end


tools.split = function(str, delimiter)
	-- 默认以空格为分隔符
	delimiter = delimiter or ' '
	if delimiter == '' then
		return {str}
	end

	local pos,arr = 0, {}

	for st,sp in function() return string.find(str, delimiter, pos, true) end do
		table.insert(arr, string.sub(str, pos, st - 1))
		pos = sp + 1
	end
	table.insert(arr, string.sub(str, pos))

	return arr
end