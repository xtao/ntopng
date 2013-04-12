--
-- (C) 2013 - ntop.org
--


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

function rev(a,b)
   return (a > b)
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

