ifname = _GET["if"]
interface.find("any")
hosts_stats = interface.getHosts()


-- ###########################

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

-- ###########################

tot = 0
_hosts_stats = {}
for key, value in pairs(hosts_stats) do
     _hosts_stats[value] = key
       tot = tot +value
end

-- Print up to this number of entries
max_num_entries = 7

-- Print entries whose value >= 5% of the total
threshold = (tot * 3) / 100


print "[\n"
num = 0
accumulate = 0
for key, value in pairsByKeys(_hosts_stats, rev) do
   if(key < threshold) then
     break
   end

   if(num > 0) then
      print ",\n"
   end

   print("\t { \"label\": \"" .. value .."\", \"value\": ".. key .." }")
   accumulate = accumulate + key
   num = num + 1

   if(num == max_num_entries) then
     break
end
end

-- In case there is some leftover do print it as "Other"
if(accumulate < tot) then
      print(",\n\t { \"label\": \"Other\", \"value\": ".. (tot-accumulate) .." }")
end

print "\n]"




