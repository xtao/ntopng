ifname = _GET["if"]
interface.find("any")
interface.getNdpiStats()

print "[\n"

num = 0
for key, value in pairs(ifstats) do
   if(num > 0) then
      print ",\n"
   end

   print("\t { \"label\": \"" .. key .."\", \"value\": ".. value .." }")
   num = num + 1
end

print "\n]"

