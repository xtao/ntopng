--
-- (C) 2013 - ntop.org
--

-- Note that ifname can be set by Lua.cpp so don't touch it if already defined
if((ifname == nil) and (_GET ~= nil)) then	
 ifname = _GET["ifname"]	
 if(ifname == nil) then	  
   ifname = _SESSION["ifname"]
 end
end

l4_keys = {
   { "TCP", "tcp" },
   { "UDP", "udp" },
   { "ICMP", "icmp" },
   { "Other IP", "other_ip" }
}


function sendHTTPHeaderIfName(mime, ifname, maxage)
   print('HTTP/1.1 200 OK\r\n')
   print('Set-Cookie: session='.._SESSION["session"]..'; max-age=' .. maxage .. '; http-only\r\n')
   if(ifname ~= nil) then print('Set-Cookie: ifname=' .. ifname .. '\r\n') end
   print('Content-Type: '.. mime ..'\r\n')
   print('\r\n')
end

function sendHTTPHeaderLogout(mime)
   sendHTTPHeaderIfName(mime, nil, 0)
end

function sendHTTPHeader(mime)
   sendHTTPHeaderIfName(mime, nil, 3600)
end

function findString(str, tofind)
   str1    = string.gsub(str, "-", "_")
   tofind1 = string.gsub(tofind, "-", "_")
   rsp = string.find(str1, tofind1, 1)

   --print(str1 .. "/" .. tofind1.."\n")
   --print(rsp)
   --print("\n")

   return(rsp)
end

function printASN(asn, asname)
   if(asn > 0) then
      return("<A HREF='http://as.robtex.com/as"..asn..".html' title='"..asname.."'>"..asn.."</A>")
   else
      return("")
   end
end

function shortHostName(name)
   local chunks = {name:match("(%d+)%.(%d+)%.(%d+)%.(%d+)")}
   if (#chunks == 4) then
      return(name)
   else
      chunks = {name:match("%w+:%w+:%w+:%w+:%w+:%w+")}
      io.write(#chunks.."\n")
      if (#chunks == 1) then
	 return(name)
      end

      for token in string.gmatch(name, "([%w-]+).") do
	 return(token)
      end
   end

   return(name)
end


function l4Label(proto)
   local id

   for id, _ in ipairs(l4_keys) do
      local l = l4_keys[id][1]
      local key = l4_keys[id][2]

      if(key == proto) then
	 return(l)
      end
   end

   return(firstToUpper(proto))
end

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
      return bytes .. " Bytes";
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

function formatValue(amount)
   local formatted = amount
   while true do
      formatted, k = string.gsub(formatted, "^(-?%d+)(%d%d%d)", '%1,%2')
      if (k==0) then
	 break
      end
   end
   return formatted
end

function formatPackets(amount)
   return formatValue(amount).." Pkts"
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

string.split = function(s, p)
		  local temp = {}
    local index = 0
		  local last_index = string.len(s)

    while true do
       local i, e = string.find(s, p, index)

        if i and e then
            local next_index = e + 1
            local word_bound = i - 1
	   table.insert(temp, string.sub(s, index, word_bound))
            index = next_index
        else            
            if index > 0 and index <= last_index then
	       table.insert(temp, string.sub(s, index, last_index))
            elseif index == 0 then
                temp = nil
            end
            break
	 end
      end

    return temp
   end

function formatEpoch(epoch)
   return(os.date("%d/%m/%Y %X", epoch))
end

function secondsToTime(seconds)
   if(seconds < 1) then
      return("< 1 sec")
   end

   days = math.floor(seconds / 86400)
   hours =  math.floor((seconds / 3600) - (days * 24))
   minutes = math.floor((seconds / 60) - (days * 1440) - (hours * 60))
   sec = seconds % 60
   msg = ""

   if(days > 0) then
      years = math.floor(days/365)

      if(years > 0) then
	 days = days % 365

	 msg = years .. " year"
	 if(years > 1) then
	    msg = msg .. "s"
	 end

	 msg = msg .. ", "
      end
      msg = msg .. days .. " day"
      if(days > 1) then msg = msg .. "s" end
      msg = msg .. ", "
   end

   if(hours > 0) then
      msg = msg .. string.format("%d ", hours)
      if(hours > 1) then
	 msg = msg .. "hour"
      else
	 msg = msg .. "hour"
      end

      if(hours > 1) then msg = msg .. "s" end
      msg = msg .. ", "
   end

   if(minutes > 0) then
      msg = msg .. string.format("%d min", minutes)
   end

   if(sec > 0) then
      if((string.len(msg) > 0) and (minutes > 0)) then msg = msg .. ", " end
      msg = msg .. string.format("%d sec", sec);
   end

   return msg
end

function starts(String,Start)
   return string.sub(String,1,string.len(Start))==Start
end

function ends(String,End)
   return End=='' or string.sub(String,-string.len(End))==End
end


-- #################################################################

categories = {
   {"1_1", "Drug Abuse", {
       {"Websites that feature information on illegal drug activities including: drug promotion, preparation, cultivation, trafficking, distribution, solicitation, etc."},
 }},
   {"1_2", "Hacking", {
       {"Websites that depict illicit activities surrounding the unauthorized modification or access to programs, computers, equipment and websites."},
 }},
   {"1_3", "Illegal or Unethical", {
       {"Websites that feature information, methods, or instructions on fraudulent actions or unlawful conduct (non-violent) such as scams, counterfeiting, tax evasion, petty theft, blackmail, etc."},
 }},
   {"1_4", "Discrimination", {
       {"Sites that promote the identification of racial groups, the denigration or subjection of groups, or the superiority of any group."},
 }},
   {"1_5", "Violence", {
       {"This category includes sites that depict offensive material on brutality, death, cruelty, acts of abuse, mutilation, etc."},
 }},
   {"1_6", "Proxy Avoidance", {
       {"Websites that provide information or tools on how to bypass Internet access controls and browse the Web anonymously, includes anonymous proxy servers."},
 }},
   {"1_7", "Plagiarism", {
       {"Websites that provide, distribute or sell school essays, projects, or diplomas."},
 }},
   {"1_8", "Child Abuse", {
       {"Websites that have been verified by the Internet Watch Foundation to contain or distribute images of non-adult children that are depicted in a state of abuse"},
 }},
   {"2_1", "Alternative Beliefs", {
       {"Websites that provide information about or promote religions not specified in Traditional Religions or other unconventional, cultic, or folkloric beliefs and practices. Sites that promote or offer methods, means of instruction, or other resources to affect or influence real events through the use of spells, curses, magic powers, satanic or supernatural beings."},
 }},
   {"2_2", "Abortion", {
       {"Websites pertaining to abortion data, information, legal issues, and organizations."},
 }},
   {"2_3", "Adult Materials", {
       {"Mature content websites (18+ years and over) that feature or promote sexuality, strip clubs, sex shops, etc. excluding sex education, without the intent to sexually arouse."},
 }},
   {"2_4", "Advocacy Groups", {
       {"This category caters to organizations that campaign or lobby for a cause by building public awareness, raising support, influencing public policy, etc."},
 }},
   {"2_5", "Gambling", {
       {"Sites that cater to gambling activities such as betting, lotteries, casinos, including gaming information, instruction, and statistics."},
 }},
   {"2_6", "Extremist Groups", {
       {"Sites that feature radical militia groups or movements with aggressive anti-government convictions or beliefs."},
 }},
   {"2_7", "Nudity and Risque", {
       {"Mature content websites (18+ years and over) that depict the human body in full or partial nudity without the intent to sexually arouse."},
 }},
   {"2_8", "Pornography", {
       {"Mature content websites (18+ years and over) which present or display sexual acts with the intent to sexually arouse and excite."},
 }},
   {"2_9", "Tasteless", {
       {"Tasteless"},
 }},
   {"2_10", "Weapons", {
       {"Websites that feature the legal promotion or sale of weapons such as hand guns, knives, rifles, explosives, etc."},
 }},
   {"2_11", "Homosexuality", {
       {"Homosexuality"},
 }},
   {"2_12", "Marijuana", {
       {"Sites that provide information about or promote the cultivation, preparation, or use of marijuana."},
 }},
   {"2_13", "Sex Education", {
       {"Educational websites that provide information or discuss sex and sexuality, without utilizing pornographic materials."},
 }},
   {"2_14", "Alcohol", {
       {"Websites which legally promote or sell alcohol products and accessories."},
 }},
   {"2_15", "Tobacco", {
       {"Websites which legally promote or sell tobacco products and accessories."},
 }},
   {"2_16", "Lingerie and Swimsuit", {
       {"Websites that utilizes images of semi-nude models in lingerie, undergarments and swimwear for the purpose of selling or promoting such items."},
 }},
   {"2_17", "Sports Hunting and War Games", {
       {"Web pages that feature sport hunting, war games, paintball facilities, etc. Includes all related clubs, organizations and groups."},
 }},
   {"3_1", "Feeware and Software Downloads", {
       {"Sites whose primary function is to provide freeware and software downloads. Cell phone ringtones/images/games, computer software updates for free downloads are all included in this category."},
 }},
   {"3_2", "File Sharing and Storage", {
       {"Websites that permit users to utilize Internet servers to store personal files or for sharing, such as with photos."},
 }},
   {"3_3", "Streaming Media", {
       {"Websites that allow the downloading of MP3 or other multimedia files."},
 }},
   {"3_4", "Peer-to-peer File Sharing", {
       {"Websites that allow users to share files and data storage between each other."},
 }},
   {"3_5", "Internet Radio and TV", {
       {"Websites that broadcast radio or TV communications over the Internet."},
 }},
   {"3_6", "Internet Telephony", {
       {"Websites that enable telephone communications over the Internet."},
 }},
   {"4_1", "Malicious Websites", {
       {"Sites that host software that is covertly downloaded to a user's machine to collect information and monitor user activity, and sites that are infected with destructive or malicious software, specifically designed to damage, disrupt, attack or manipulate computer systems without the user's consent, such as virus or trojan horse."},
 }},
   {"4_2", "Phishing", {
       {"Counterfeit web pages that duplicate legitimate business web pages for the purpose of eliciting financial, personal or other private information from the users."},
 }},
   {"4_3", "Spam URLs", {
       {"Websites or webpages whose URLs are found in spam emails. These webpages often advertise sex sites, fraudulent wares, and other potentially offensive materials."},
 }},
   {"5_1", "Finance and Banking", {
       {"Financial Data and Services -- Sites that offer news and quotations on stocks, bonds, and other investment vehicles, investment advice, but not online trading. Includes banks, credit unions, credit cards, and insurance. Mortgage/insurance brokers apply here as opposed to Brokerage and Trading."},
 }},
   {"5_2", "Search Engines and Portals", {
       {"Sites that support searching the Web, news groups, or indices/directories. Sites of search engines that provide info exclusively for shopping or comparing prices, however, fall in Shopping and Auction."},
 }},
   {"5_3", "General Organizations", {
       {"Sites that cater to groups, clubs or organisations of individuals with similar interests, either professional, social, humanitarian or recreational in nature. Social and Affiliation Organizations: Sites sponsored by or that support or offer information about organizations devoted chiefly to socializing or common interests other than philanthropy or professional advancement. Not to be be confused with Advocacy Groups and Political Groups."},
 }},
   {"5_4", "Business", {
       {"Sites sponsored by or devoted to business firms, business associations, industry groups, or business in general. Information Technology companies are excluded in this category and fall in Information Technology."},
 }},
   {"5_5", "Information Technology", {
       {"Information Technology peripherals and services, cell phone services, cable TV/Internet suppliers."},
 }},
   {"5_6", "Government and Legal Organizations", {
       {"Government: Sites sponsored by branches, bureaus, or agencies of any level of government, except for the armed forces, including courts, police institutions, city-level government institutions. Legal Organizations: Sites that discuss or explain laws of various government entities."},
 }},
   {"5_7", "Armed Forces", {
       {"Websites related to organized military and armed forces, excluding civil and extreme military organizations."},
 }},
   {"5_8", "Web Hosting", {
       {"Sites of organizations that provide hosting services, or top-level domain pages of Web communities."},
 }},
   {"5_9", "Secure Websites", {
       {"Sites that institute security measures such as authentication, passwords, registration, etc."},
 }},
   {"5_10", "Web-based Applications", {
       {"Sites that mimic desktop applications such as word processing, spreadsheets, and slide-show presentations."},
 }},
   {"6_1", "Advertising", {
       {"Sites that provide advertising graphics or other ad content files, including ad servers (domain name often with “ad.” , such as ad.yahoo.com). If a site is mainly for online transactions, it is rated as Shopping and Auctions. Includes pay-to-surf and affiliated advertising programs."},
 }},
   {"6_2", "Brokerage and Trading", {
       {"Sites that support active trading of securities and management of investments. Real estate broker does not apply here, and falls within Shopping and Auction. Sites that provide supplier and buyer info/ads do not apply here either since they do not provide trading activities."},
 }},
   {"6_3", "Games", {
       {"Sites that provide information about or promote electronic games, video games, computer games, role-playing games, or online games. Includes sweepstakes and giveaways. Sport games are not included in this category, but time consuming mathematic game sites that serve little education purpose are included in this category."},
 }},
   {"6_4", "Web-based Email", {
       {"Sites that allow users to utilize electronic mail services."},
 }},
   {"6_5", "Entertainment", {
       {"Sites that provide information about or promote motion pictures, non-news radio and television, music and programming guides, books, humor, comics, movie theatres, galleries, artists or review on entertainment, and magazines. Includes book sites that have personal flavor or extra-material by authors to promote the books."},
 }},
   {"6_6", "Arts and Culture", {
       {"Websites that cater to fine arts, cultural behaviors and backgrounds including conventions, artwork and paintings, music, languages, customs, etc. Also includes institutions such as museums, libraries and historic sites. Sites that promote historical, cultural heritage of certain area, but not purposely promoting travel."},
 }},
   {"6_7", "Education", {
       {"Educational Institutions: Sites sponsored by schools, other educational facilities and non-academic research institutions, and sites that relate to educational events and activities. Educational Materials: Sites that provide information about, sell, or provide curriculum materials. Sites that direct instruction, as well as academic journals and similar publications where scholars and professors submit academic/research articles."},
 }},
   {"6_8", "Health and Wellness", {
       {"Sites that provide information or advice on personal health or medical services, procedures, or devices, but not drugs. Includes self-help groups. This category includes cosmetic surgery providers, children's hospitals, but not sites of medical care for pets, which fall in Society and Lifestyle."},
 }},
   {"6_9", "Job Search", {
       {"Sites that offer information about or support the seeking of employment or employees. Includes career agents and consulting services that provide job postings."},
 }},
   {"6_10", "Medicine", {
       {"Prescribed Medications: Sites that provide information about approved drugs and their medical use. Supplements and Unregulated Compounds: Sites that provide information about or promote the sale or use of chemicals not regulated by the FDA (such as naturally occurring compounds). This category includes sites of online shopping for medicine, as it is a sensitive category separated from regular shopping."},
 }},
   {"6_11", "News and Media", {
       {"Sites that offer current news and opinion, including those sponsored by newspapers, general-circulation magazines, or other media. This category includes TV and Radio sites, as long as they are not exclusively for entertainment purpose, but excludes academic journals. Alternative Journals: Online equivalents to supermarket tabloids and other fringe publications."},
 }},
   {"6_12", "Social Networking", {
       {"Includes websites that aid in the coordination of heterosexual relationships and companionship. Includes legal and non-sexual sites related to on-line dating, personal ads, dating services, clubs, etc."},
 }},
   {"6_13", "Political Organizations", {
       {"Sites that are sponsored by or provide information about political parties and interest groups focused on elections or legislation. This is not to be confused with Government and Legal Organizations, and Advocacy Groups."},
 }},
   {"6_14", "Reference", {
       {"Websites that provide general reference data in the form of libraries, dictionaries, thesauri, encyclopedias, maps, directories, standards, etc."},
 }},
   {"6_15", "Global Religion", {
       {"Sites that provide information about or promote Buddhism, Bahai, Christianity, Christian Science, Hinduism, Islam, Judaism, Mormonism, Shinto, and Sikhism, as well as atheism."},
 }},
   {"6_16", "Shopping and Auction", {
       {"Websites that feature on-line promotion or sale of general goods and services such as electronics, flowers, jewelry, music, etc, excluding real estate. Also includes on-line auction services such as eBay, Amazon, Priceline."},
 }},
   {"6_17", "Society and Lifestyles", {
       {"This category contains sites that deal with everyday life issues and preferences such as passive hobbies (gardening, stamp collecting, pets), journals, blogs, etc."},
 }},
   {"6_18", "Sports Travel", {
       {"Includes sites that pertain to recreational sports and active hobbies such as fishing, hunting, jogging, canoeing, archery, chess, as well as organized, professional and competitive sports. Websites feature travel related resources such as accommodations, transportation (rail, airlines, cruise ships), agencies, resort locations, tourist attractions, advisories, etc."},
 }},
   {"6_19", "Personal Vehicles", {
       {"Websites that contain information on private use or sale of autos, boats, planes, motorcycles, etc., including parts and accessories."},
 }},
   {"6_20", "Dynamic Content", {
       {"URLs that are generated dynamically by a Web server."},
 }},
   {"6_21", "Miscellaneous", {
       {"This category houses URLs that cannot be definitively categorized due to lack of or ambiguous content."},
 }},
   {"6_22", "Folklore", {
       {"UFOs, fortune telling, horoscopes, fen shui, palm reading, tarot reading, and ghost stories."},
 }},
   {"6_23", "Web Chat", {
       {"Sites that host Web chat services, or that support or provide information about chat via HTTP or IRC."},
 }},
   {"6_24", "Instant Messaging", {
       {"Sites that allow users to communicate in real-time over the Internet."},
 }},
   {"6_25", "Newsgroups and Message Boards", {
       {"Sites for online personal and business clubs, discussion groups, message boards, and list servers; includes 'blogs' and 'mail magazines.'"},
 }},
   {"6_26", "Digital Postcards", {
       {"Sites for sending/viewing digital post cards."},
 }},
   {"6_27", "Child Education", {
       {"Websites developed for children age 12 and under. Includes educational games, tools, organizations and schools. Note that children's hospitals are rated as Health."},
 }},
   {"6_28", "Real Estate", {
       {"Websites that promote the sale or renting of real estate properties."},
 }},
   {"6_29", "Restaurant and Dining", {
       {"Websites related to restaurants and dining, includes locations, food reviews, recipes, catering services, etc."},
 }},
   {"6_30", "Personal Websites and Blogs", {
       {"Private web pages that host personal information, opinions and ideas of the owners."},
 }},
   {"6_31", "Content Servers", {
       {"Websites that host servers that distribute content for subscribing websites. Includes image and Web servers."},
 }},
   {"6_32", "Domain Parking", {
       {"Sites that simply are place holders of domains without meaningful content."},
 }},
   {"6_33", "Personal Privacy", {
       {"Sites providing online banking, trading, health care, and others that contain personal privacy information."},
 }}
};

function getCategory(_cat)
   cat = string.gsub(_cat, "\n", "")

   if(starts(cat, "error") or (cat == "''") or (cat == "") or starts(cat, "-") or starts(cat, "Local")) then
      return("")
   else

      for id, _ in ipairs(categories) do
	 local key = categories[id][1]
	 local name = categories[id][2]

	 if(key == cat) then
	    return(name)
	 end
      end

      return(cat)
   end
end


function abbreviateString(str, len)
   if(str == nil) then
      return("")
   else
      if(string.len(str) < len) then
	 return(str)
      else
	 return(string.sub(str, 1, len).."...")
      end
   end
end

function bit(p)
   return 2 ^ (p - 1)  -- 1-based indexing
end

-- Typical call:  if hasbit(x, bit(3)) then ...
function hasbit(x, p)
   return x % (p + p) >= p       
end

function setbit(x, p)
   return hasbit(x, p) and x or x + p
end

function clearbit(x, p)
   return hasbit(x, p) and x - p or x
end

function isBroadMulticast(ip)
   if(ip == "0.0.0.0") then return(true) end

   -- print(ip)
   t = string.split(ip, "%.")
   -- print(table.concat(t, "\n"))
   if(t == nil) then 
      return(false) -- Might be an IPv6 address
   else
      if(tonumber(t[1]) >= 224)  then return(true) end
   end

   return(false)
end


function addGauge(name, url, maxValue, width, height)
   if(url ~= nil) then print('<A HREF="'..url..'">') end
   print('<canvas id="'..name..'" height="'..height..'" width="'..width..'"></canvas>\n')
--   print('<div id="'..name..'-text" style="font-size: 12px;"></div>\n')
   if(url ~= nil) then print('</A>') end

print [[
 <script type="text/javascript">

var opts = {
fontSize: 40,
  lines: 12, // The number of lines to draw
  angle: 0.15, // The length of each line
  lineWidth: 0.44, // The line thickness
  pointer: {
    length: 0.85, // The radius of the inner circle
    strokeWidth: 0.051, // The rotation offset
    color: '#000000' // Fill color
  },
  limitMax: 'false',   // If true, the pointer will not go past the end of the gauge

  colorStart: '#6FADCF',   // Colors
  colorStop: '#8FC0DA',    // just experiment with them
  strokeColor: '#E0E0E0',   // to see which ones work best for you
  generateGradient: true
};
]]

print('var target = document.getElementById("'..name..'"); // your canvas element\n')
print('var '..name..' = new Gauge(target).setOptions(opts);\n')
--print(name..'.setTextField(document.getElementById("'..name..'-text"));\n')
print(name..'.maxValue = '..maxValue..'; // set max gauge value\n')
print("</script>\n")
end

-- Compute the difference in seconds between local time and UTC.
function get_timezone()
   local now = os.time()
   return os.difftime(now, os.time(os.date("!*t", now)))
end


-- Return the first 'howmany' hosts 
function getTopInterfaceHosts(howmany, localHostsOnly)
   hosts_stats = interface.getHostsInfo()
   ret = {}
   sortTable = {}
   n = 0
   for k,v in pairs(hosts_stats) do 
      if((not localHostsOnly) or ((v["localhost"] ~= nil) and (v["ip"] ~= nil))) then
	 sortTable[v["bytes.sent"]+v["bytes.rcvd"]+n] = k
	 n = n +0.01
      end
   end

   n = 0
   for _v,k in pairsByKeys(sortTable, rev) do 
      if(n < howmany) then 
	 ret[k] = hosts_stats[k]
	 n = n+1
      else
	 break
      end
   end

   return(ret)
end