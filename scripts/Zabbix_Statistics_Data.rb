#!/usr/bin/ruby -w
#

# (c) 2009, Bjoern Rennhak, Centillion
# Works with Zabbix 1.6.x

#
# Run-time:
#
#   the curl downloading takes approx. 15 min
#   the compression part 300mb+ -> xxmb takes approx, 10-15 min
#


#
# FIXME:
#
#     o build it for parallel downloads (fork processes e.g. 10 or so and wait for them)
#     o remove the hardcoding from the script regarding the db values of graphs
#     o restructure script from procedural to oo based
#


require 'date'
require 'hpricot'

# FIXME: This is namespace pollution, repair this.
# Usage: Date.today.dim or (Date.today<<1).dim
class Date
   def dim
     d,m,y = mday,month,year
     d += 1 while Date.valid_civil?(y,m,d)
     d - 1
   end
end 


# Variables
today             = Date.today

lastMonth         = today << 1
lastMonthsDays    = lastMonth.dim
lastMonthFormatted = sprintf( "%.2i", lastMonth.month )
lastMonthShort    = "#{lastMonth.year}#{lastMonthFormatted}"
lastMonthsFirst   = "#{lastMonthShort}010000"

zabbixBaseURL     = "http://localhost/zabbix"
zabbixLoginFile   = "index.php"
zabbixLoginURL    = "#{zabbixBaseURL}/#{zabbixLoginFile}"

username          = "statistics"
password          = "PgwyVM006Ur67zUPwkgY"

tmpDir            = "/tmp"
cookieName        = "tmp_zabbix_cookie_#{today.to_s}"

cookie            = "#{tmpDir}/#{cookieName}"

oneDay            = "86400"
oneMonth          = (lastMonthsDays.to_i * oneDay.to_i).to_s


# Clean
`rm -rf Zabbix_Statistics_-_Values`

## Cluster Subdirs
currentDir = Dir.pwd

### Make subdirectories for both clusters
## Topdirs
Dir.mkdir( "Zabbix_Statistics_-_Values" )

%w[equinx fairness].each { |cluster| Dir.chdir( "Zabbix_Statistics_-_Values" ); Dir.mkdir( cluster.capitalize ); Dir.chdir( cluster.capitalize ); Dir.mkdir( lastMonth.year.to_s ); Dir.chdir( lastMonth.year.to_s ); Dir.mkdir( lastMonthFormatted.to_s ); Dir.chdir( currentDir ) }

# Get session cookie
`curl -d "name=#{username}" -d "password=#{password}" -d 'login=1' -d 'form=1' -d 'form_refresh=1' -d 'enter=Enter' -c "#{cookie}" "#{zabbixLoginURL}"`

# The Zabbix DB schema is a nightmare, if you can find where the itemid and the hostid corrolate,
# please change the code to be more robust.
#
# These values depend on the Zabbix Server Configuration, should this change we're screwed.

fairness = {
    # server name => hash
    "WWW1"  => {
        # itemid => description
        22327 => "CPU Statistics - Idle Time",
        22328 => "CPU Statistics - Nice Time",
        22329 => "CPU Statistics - System Time",
        22331 => "CPU Statistics - Wait Time",
        22330 => "CPU Statistics - User Time",
        
        22362 => "File System ROOT - Free Disk Space in GB",
        22363 => "File System ROOT - Free Disk Space in Percent",
        22386 => "File System Var - Free Disk Space in GB",
        22387 => "File System Var - Free Disk Space in Percent",

        22390 => "Memory - Buffers",
        22391 => "Memory - Cached",
        22392 => "Memroy - Free",

        22297 => "Netowrk - Traffic incoming in KBps",
        22300 => "Network - Traffic outgoing in KBps",
        
        22325 => "System - Load Average",
        22326 => "System - Load Average 5",
        22324 => "System - Load Average 15"
    },
    "WWW2"  => {
        # itemid => description
        22225 => "CPU Statistics - Idle Time",
        22226 => "CPU Statistics - Nice Time",
        22227 => "CPU Statistics - System Time",
        22229 => "CPU Statistics - Wait Time",
        22228 => "CPU Statistics - User Time",
        
        22260 => "File System ROOT - Free Disk Space in GB",
        22261 => "File System ROOT - Free Disk Space in Percent",
        22284 => "File System Var - Free Disk Space in GB",
        22285 => "File System Var - Free Disk Space in Percent",

        22288 => "Memory - Buffers",
        22289 => "Memory - Cached",
        22290 => "Memroy - Free",

        22195 => "Netowrk - Traffic incoming in KBps",
        22198 => "Network - Traffic outgoing in KBps",
        
        22223 => "System - Load Average",
        22224 => "System - Load Average 5",
        22222 => "System - Load Average 15"
    },
    "DB1"   => {
       # itemid => description
        22429 => "CPU Statistics - Idle Time",
        22430 => "CPU Statistics - Nice Time",
        22431 => "CPU Statistics - System Time",
        22433 => "CPU Statistics - Wait Time",
        22432 => "CPU Statistics - User Time",
        
        22464 => "File System ROOT - Free Disk Space in GB",
        22465 => "File System ROOT - Free Disk Space in Percent",
        22488 => "File System Var - Free Disk Space in GB",
        22489 => "File System Var - Free Disk Space in Percent",

        22492 => "Memory - Buffers",
        22493 => "Memory - Cached",
        22494 => "Memroy - Free",

        22399 => "Netowrk - Traffic incoming in KBps",
        22402 => "Network - Traffic outgoing in KBps",
        
        22427 => "System - Load Average",
        22428 => "System - Load Average 5",
        22426 => "System - Load Average 15"
    },
    "DB2"   => {
        # itemid => description
        22531 => "CPU Statistics - Idle Time",
        22532 => "CPU Statistics - Nice Time",
        22533 => "CPU Statistics - System Time",
        22535 => "CPU Statistics - Wait Time",
        22534 => "CPU Statistics - User Time",
        
        22566 => "File System ROOT - Free Disk Space in GB",
        22567 => "File System ROOT - Free Disk Space in Percent",
        22590 => "File System Var - Free Disk Space in GB",
        22591 => "File System Var - Free Disk Space in Percent",

        22594 => "Memory - Buffers",
        22595 => "Memory - Cached",
        22596 => "Memroy - Free",

        22501 => "Netowrk - Traffic incoming in KBps",
        22504 => "Network - Traffic outgoing in KBps",
        
        22529 => "System - Load Average",
        22530 => "System - Load Average 5",
        22528 => "System - Load Average 15"
    }
}

equinx = {
    # server name => hash
    "WWW1"  => {
        # itemid => description
        22633 => "CPU Statistics - Idle Time",
        22634 => "CPU Statistics - Nice Time",
        22635 => "CPU Statistics - System Time",
        22637 => "CPU Statistics - Wait Time",
        22636 => "CPU Statistics - User Time",
        
        22668 => "File System ROOT - Free Disk Space in GB",
        22669 => "File System ROOT - Free Disk Space in Percent",
        22692 => "File System Var - Free Disk Space in GB",
        22693 => "File System Var - Free Disk Space in Percent",

        22696 => "Memory - Buffers",
        22697 => "Memory - Cached",
        22698 => "Memroy - Free",

        22603 => "Netowrk - Traffic incoming in KBps",
        22606 => "Network - Traffic outgoing in KBps",
        
        22631 => "System - Load Average",
        22632 => "System - Load Average 5",
        22630 => "System - Load Average 15"
    },
    "WWW2"  => {
        # itemid => description
        22735 => "CPU Statistics - Idle Time",
        22736 => "CPU Statistics - Nice Time",
        22737 => "CPU Statistics - System Time",
        22739 => "CPU Statistics - Wait Time",
        22738 => "CPU Statistics - User Time",
        
        22770 => "File System ROOT - Free Disk Space in GB",
        22771 => "File System ROOT - Free Disk Space in Percent",
        22794 => "File System Var - Free Disk Space in GB",
        22795 => "File System Var - Free Disk Space in Percent",

        22798 => "Memory - Buffers",
        22799 => "Memory - Cached",
        22800 => "Memroy - Free",

        22705 => "Netowrk - Traffic incoming in KBps",
        22708 => "Network - Traffic outgoing in KBps",
        
        22733 => "System - Load Average",
        22734 => "System - Load Average 5",
        22732 => "System - Load Average 15"
    },
    "DB1"   => {
       # itemid => description
        22837 => "CPU Statistics - Idle Time",
        22838 => "CPU Statistics - Nice Time",
        22839 => "CPU Statistics - System Time",
        22841 => "CPU Statistics - Wait Time",
        22840 => "CPU Statistics - User Time",
        
        22872 => "File System ROOT - Free Disk Space in GB",
        22873 => "File System ROOT - Free Disk Space in Percent",
        22896 => "File System Var - Free Disk Space in GB",
        22897 => "File System Var - Free Disk Space in Percent",

        22900 => "Memory - Buffers",
        22901 => "Memory - Cached",
        22902 => "Memroy - Free",

        22807 => "Netowrk - Traffic incoming in KBps",
        22810 => "Network - Traffic outgoing in KBps",
        
        22835 => "System - Load Average",
        22836 => "System - Load Average 5",
        22834 => "System - Load Average 15"
    },
    "DB2"   => {
        # itemid => description
        22939 => "CPU Statistics - Idle Time",
        22940 => "CPU Statistics - Nice Time",
        22941 => "CPU Statistics - System Time",
        22943 => "CPU Statistics - Wait Time",
        22942 => "CPU Statistics - User Time",
        
        22974 => "File System ROOT - Free Disk Space in GB",
        22975 => "File System ROOT - Free Disk Space in Percent",
        22998 => "File System Var - Free Disk Space in GB",
        22999 => "File System Var - Free Disk Space in Percent",

        23002 => "Memory - Buffers",
        23003 => "Memory - Cached",
        23004 => "Memroy - Free",

        22909 => "Netowrk - Traffic incoming in KBps",
        22912 => "Network - Traffic outgoing in KBps",
        
        22937 => "System - Load Average",
        22938 => "System - Load Average 5",
        22936 => "System - Load Average 15"
    }
}

equinxHostID = {
    "WWW1" => 10052,
    "WWW2" => 10053,
    "DB1" => 10054,
    "DB2" => 10055
}


fairnessHostID = {
    "WWW1" => 10052,
    "WWW2" => 10053,
    "DB1" => 10054,
    "DB2" => 10055
}


# http://192.168.0.13:2080/zabbix/history.php?itemid=22731&action=showvalues&stime=200902010000&period=2419200&from=0&plaintext=As+plain+text
# Result structure is <Date> <Unix time> <Value> wrapped in a <pre> tag and some description at the
# beginning.


### Generate PNG's
#
# This generates for $cluster (all servers), all graphs for (last) one month


%w[equinx fairness].each do |cluster|
    Dir.chdir( currentDir )                 # go to sane start position
    Dir.chdir( "Zabbix_Statistics_-_Values" )
    Dir.chdir( cluster.capitalize.to_s )    # cd $cluster
    (eval( cluster )).each_pair do |serverName, hash|
        name = "#{cluster.to_s}HostID"
        hash.each_pair do |itemid, description|
            # Construct temp fn + fn
            fn_tmp = "#{cluster.capitalize}_-_#{serverName}_-_#{description.to_s.gsub(" ","_")}_-_Month_-_#{lastMonthsFirst.gsub(/......$/,"")}.htm"
            fn = ( File.basename( fn_tmp, ".htm" ) ).to_s + ".txt"
            
            # Download file
            `curl -b "#{cookie}" -d "itemid=#{itemid}" -d "action=showvalues" -d "stime=#{lastMonthsFirst}" -d "period=#{oneMonth}" -d "from=0" -d "plaintext=As+plain+text" "#{zabbixBaseURL}/history.php" > "#{fn_tmp}"`
        end
    end
end


# This generates for Equnix (all servers), all graphs for each day of last month
%w[equinx fairness].each do |cluster|
    Dir.chdir( currentDir )                 # go to sane start position
    Dir.chdir( "Zabbix_Statistics_-_Values" )
    Dir.chdir( cluster.capitalize.to_s )    # cd $cluster
    Dir.chdir( lastMonth.year.to_s )        # cd $year
    Dir.chdir( lastMonthFormatted.to_s )    # cd $month

    ( eval( cluster ) ).each_pair do |serverName, hash|
        name = "#{cluster.to_s}HostID"
        hash.each_pair do |itemid, description|
            1.upto( lastMonthsDays.to_i ) do |day|
                tmp = sprintf( "%.2i", day )
                lastMonthsDay   = "#{lastMonth.year}#{lastMonthFormatted}#{tmp}0000"

                # Construct temp fn + fn
                fn_tmp = "#{cluster.capitalize}_-_#{serverName}_-_#{description.to_s.gsub(" ","_")}_-_Day_-_#{lastMonthsDay.gsub(/....$/,"")}.ht"
                fn = ( File.basename( fn_tmp, ".htm" ) ).to_s + ".txt"

                # Download file
                `curl -b "#{cookie}" -d "itemid=#{itemid}" -d "action=showvalues" -d "stime=#{lastMonthsDay}" -d "period=#{oneDay}" -d "from=0" -d "plaintext=As+plain+text"  "#{zabbixBaseURL}/history.php" > "#{fn_tmp}"`
            end
        end
    end
end


# htm -> txt
Dir.chdir( currentDir )
Dir.chdir( "Zabbix_Statistics_-_Values" )
Dir["**/*.htm"].each do |file| 
    newFile = File.basename( file, ".htm" ).to_s + ".txt"
    File.open( newFile, File::WRONLY|File::TRUNC|File::CREAT, 0666 ) do |f|
         open( file ) { |dt| f.write( Hpricot( dt ).search("//pre").innerHTML ) }
    end
end

Dir.chdir( currentDir )                     # go back to origin path

### Compress the whole result and make it available via webserver
# (http://$ip:$port/zabbix_dumps/$file)
# 
filename  = "Zabbix_Statistics_-_Values_-_#{lastMonthShort}.tar.bz2"
targetDir = "/var/www/zabbix_dumps"
`apack #{filename} Zabbix_Statistics_-_Values`

# Move the result to a place where we can download it
`mv #{filename} /var/www/zabbix_dumps/.`
`chmod 644 #{targetDir}/#{filename}`

filesize = File.size( "#{targetDir}/#{filename}" )

# Send the mail + mime encoded attachment
to = "$email" # this is in /etc/aliases
`echo "Dear Admins,\n\nplease find here a dump of Zabbix Value Data from the $Machines.\n\nTime: #{lastMonthShort.to_s}\n\nFormat: Tar + Bzip2\nSize: #{filesize.to_s} byte\nURL: http://$ip:$port/zabbix_dumps/#{filename}\n(This will of course only work inside the Network).\n\nShould some data be empty (drops timeouts etc.) then the dump can be empty too. (Reason: A SQL Query failed because of uncomplete data.)\n\nRegards,\n\n\tYour hard-working Cron @ $vhost/bin/Zabbix_Statistics.rb\n\n\n----\nPS: Don't forget to delete this archive from the webserver otherwise we'll eventually run out of space.\n($mypath/var/www/zabbix_dumps/..)\n" > /tmp/tmp_mailbody`
`mutt -F /root/.muttrc -s "Zabbix Value Data Dump for #{lastMonthShort.to_s}" #{to} < /tmp/tmp_mailbody`
`rm -f /tmp/tmp_mailbody`


# Delete stuff which lies around
Dir.chdir( currentDir )
`rm -rf Zabbix_Statistics_-_Values`


