--
-- (C) 2013 - ntop.org
--

function firstToUpper(str)
return (str:gsub("^%l", string.upper))
end

function pairsByKeys(t, f)
   local a = {}
   for n in pairs(t) do table.insert(a, n) end
   table.sort(a, f)
   local i = 0      -- iterator variable
   local iter = function ()   -- iterator function
		   i = i + 1
		   if a[i] == nil then return nil
		   else return a[i], t[a[i]]
		   end
		end
   return iter
end

function asc(a,b)
   return (a < b)
end

function rev(a,b)
   return (a > b)
end

--for _key, _value in pairsByKeys(vals, rev) do
--   print(_key .. "=" .. _value .. "\n")
--end

function round(num, idp)
   local mult = 10^(idp or 0)
   return math.floor(num * mult + 0.5) / mult
end

-- Convert bytes to human readable format
function bytesToSize(bytes)
   precision = 2
   kilobyte = 1024;
   megabyte = kilobyte * 1024;
   gigabyte = megabyte * 1024;
   terabyte = gigabyte * 1024;

   if ((bytes >= 0) and (bytes < kilobyte)) then
      return bytes;
   elseif ((bytes >= kilobyte) and (bytes < megabyte)) then
      return round(bytes / kilobyte, precision) .. ' KB';
   elseif ((bytes >= megabyte) and (bytes < gigabyte)) then
      return round(bytes / megabyte, precision) .. ' MB';
   elseif ((bytes >= gigabyte) and (bytes < terabyte)) then
      return round(bytes / gigabyte, precision) .. ' GB';
   elseif (bytes >= terabyte) then
      return round(bytes / terabyte, precision) .. ' TB';
   else
      return bytes .. ' B';
   end
end

-- Convert bits to human readable format
function bitsToSize(bits)
   precision = 2
   kilobit = 1024;
   megabit = kilobit * 1024;
   gigabit = megabit * 1024;
   terabit = gigabit * 1024;

   if ((bits >= kilobit) and (bits < megabit)) then
      return round(bits / kilobit, precision) .. ' Kbit';
   elseif ((bits >= megabit) and (bits < gigabit)) then
      return round(bits / megabit, precision) .. ' Mbit';
   elseif ((bits >= gigabit) and (bits < terabit)) then
      return round(bits / gigabit, precision) .. ' Gbit';
   elseif (bits >= terabit) then
      return round(bits / terabit, precision) .. ' Tbit';
   else
      return round(bits, precision) .. ' bps';
   end
end

function formatPackets(amount)
  local formatted = amount
  while true do  
     formatted, k = string.gsub(formatted, "^(-?%d+)(%d%d%d)", '%1,%2')
     if (k==0) then
      break
     end
  end
  return formatted.." Pkts"
end

function split(pString, pPattern)
   local Table = {}  -- NOTE: use {n = 0} in Lua-5.0
   local fpat = "(.-)" .. pPattern
   local last_end = 1
   local s, e, cap = pString:find(fpat, 1)
   while s do
      if s ~= 1 or cap ~= "" then
	 table.insert(Table,cap)
      end
   last_end = e+1
   s, e, cap = pString:find(fpat, last_end)
end
if last_end <= #pString then
   cap = pString:sub(last_end)
   table.insert(Table, cap)
end
   return Table
end


function secondsToTime(seconds)
   days = math.floor(seconds / 86400)
   hours =  math.floor((seconds / 3600) - (days * 24))
   minutes = (seconds / 60) - (days * 1440) - (hours * 60)
   sec = seconds % 60

   if(days > 0) then
      msg = days .. " days, "
   else
      msg = ""
   end

   msg = msg .. string.format("%02d:%02d:%02d", hours, minutes, sec);
   return msg
end
