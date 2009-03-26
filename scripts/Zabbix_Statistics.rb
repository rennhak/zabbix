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
`rm -rf Zabbix_Statistics`

## Cluster Subdirs
currentDir = Dir.pwd

### Make subdirectories for both clusters
## Topdirs
Dir.mkdir( "Zabbix_Statistics" )

%w[equinx fairness].each { |cluster| Dir.chdir( "Zabbix_Statistics" ); Dir.mkdir( cluster.capitalize ); Dir.chdir( cluster.capitalize ); Dir.mkdir( lastMonth.year.to_s ); Dir.chdir( lastMonth.year.to_s ); Dir.mkdir( lastMonthFormatted.to_s ); Dir.chdir( currentDir ) }

# Get session cookie
`curl -d "name=#{username}" -d "password=#{password}" -d 'login=1' -d 'form=1' -d 'form_refresh=1' -d 'enter=Enter' -c "#{cookie}" "#{zabbixLoginURL}"`

# The Zabbix DB schema is a nightmare, if you can find where the graphid and the hostid corrolate,
# please change the code to be more robust.
#
# These values depend on the Zabbix Server Configuration, should this change we're screwed.

equinxHostID = {
    "WWW1" => 10052,
    "WWW2" => 10053,
    "DB1" => 10054,
    "DB2" => 10055
}


fairness = {
    # server name => hash
    "WWW1"  => {
        # graphid => description
        448 => "CPU Statistics",
        455 => "File System ROOT",
        462 => "File System Var",
        469 => "Load Average",
        476 => "Memory",
        483 => "Incoming Network",
        490 => "Outgoing Network"
    },
    "WWW2"  => {
         449 => "CPU Statistics",
         456 => "File System ROOT",
         463 => "File System Var",
         470 => "Load Average",
         477 => "Memory",
         484 => "Incoming Network",
         491 => "Outgoing Network"
    },
    "DB1"   => {
         446 => "CPU Statistics",
         453 => "File System ROOT",
         460 => "File System Var",
         467 => "Load Average",
         474 => "Memory",
         481 => "Incoming Network",
         488 => "Outgoing Network"
    },
    "DB2"   => {
         447 => "CPU Statistics",
         454 => "File System ROOT",
         461 => "File System Var",
         468 => "Load Average",
         475 => "Memory",
         482 => "Incoming Network",
         489 => "Outgoing Network"
    }
}


equinx = {
    # server name => hash
    "WWW1" => {
        # graphid => description
        387 => "CPU Statistics",
        388 => "File System ROOT",
        389 => "File System Var",
        393 => "Load Average",
        390 => "Memory",
        391 => "Incoming Network",
        392 => "Outgoing Network"
    },
    "WWW2" => {
        445 => "CPU Statistics",
        452 => "File System ROOT",
        459 => "File System Var",
        446 => "Load Average",
        473 => "Memory",
        480 => "Incoming Network",
        487 => "Outgoing Network"
    },
    "DB1" =>  {
        443 => "CPU Statistics",
        450 => "File System ROOT",
        457 => "File System Var",
        464 => "Load Average",
        471 => "Memory",
        478 => "Incoming Network",
        485 => "Outgoing Network"
    },
    "DB2" =>  {
        444 => "CPU Statistics",
        451 => "File System ROOT",
        458 => "File System Var",
        465 => "Load Average",
        472 => "Memory",
        479 => "Incoming Network",
        486 => "Outgoing Network"
    }
}


fairnessHostID = {
    "WWW1" => 10048,
    "WWW2" => 10049,
    "DB1" =>  10050,
    "DB2" =>  10051
}




### Generate PNG's
#
# This generates for $cluster (all servers), all graphs for (last) one month

%w[equinx fairness].each do |cluster|
    Dir.chdir( currentDir )                 # go to sane start position
    Dir.chdir( "Zabbix_Statistics" )
    Dir.chdir( cluster.capitalize.to_s )    # cd $cluster
    (eval( cluster )).each_pair do |serverName, hash|
        name = "#{cluster.to_s}HostID"
        hostID = ( eval(name)[ serverName.to_s ] ).to_s
        hash.each_pair do |graphid, description|
            #puts "Cluster: ,#{cluster}', name : ,#{name}', hostID: ,#{hostID}'"
            `curl -b "#{cookie}" -d "graphid=#{graphid}" -d "period=#{oneMonth}" -d 'width=1256' -d "stime=#{lastMonthsFirst}" "#{zabbixBaseURL}/chart2.php" > "#{cluster.capitalize}_-_#{serverName}_-_#{description.to_s.gsub(" ","_")}_-_Month_-_#{lastMonthsFirst.gsub(/......$/,"")}.png"`
         end
    end
end

# This generates for Equnix (all servers), all graphs for each day of last month
%w[equinx fairness].each do |cluster|
    Dir.chdir( currentDir )                 # go to sane start position
    Dir.chdir( "Zabbix_Statistics" )
    Dir.chdir( cluster.capitalize.to_s )    # cd $cluster
    Dir.chdir( lastMonth.year.to_s )        # cd $year
    Dir.chdir( lastMonthFormatted.to_s )    # cd $month

    ( eval( cluster ) ).each_pair do |serverName, hash|
        name = "#{cluster.to_s}HostID"
        hostID = ( eval( name)[ serverName.to_s ] ).to_s
        hash.each_pair do |graphid, description|
            1.upto( lastMonthsDays.to_i ) do |day|
                tmp = sprintf( "%.2i", day )
                lastMonthsDay   = "#{lastMonth.year}#{lastMonthFormatted}#{tmp}0000"
                `curl -b "#{cookie}" -d "graphid=#{graphid}" -d "period=#{oneDay}" -d 'width=1257' -d "stime=#{lastMonthsDay}" "#{zabbixBaseURL}/chart2.php" > "#{cluster.capitalize}_-_#{serverName}_-_#{description.to_s.gsub(" ","_")}_-_Day_-_#{lastMonthsDay.gsub(/....$/,"")}.png"`
            end
        end
    end
end

Dir.chdir( currentDir )                     # go back to origin path

### Compress the whole result and make it available via webserver
# (http://$ip:$port/zabbix_dumps/$file)
# 
filename  = "Zabbix_Statistics_-_#{lastMonthShort}.tar.bz2"
targetDir = "/var/www/zabbix_dumps"
`apack #{filename} Zabbix_Statistics`

# Move the result to a place where we can download it
`mv #{filename} /var/www/zabbix_dumps/.`
`chmod 644 #{targetDir}/#{filename}`

filesize = File.size( "#{targetDir}/#{filename}" )

# Send the mail + mime encoded attachment
to = "me@example.com" # this could also be in /etc/aliases
`echo "Dear Admins,\n\nplease find here a dump of Zabbix RRD's Machines.\n\nTime: #{lastMonthShort.to_s}\n\nFormat: Tar + Bzip2\nSize: #{filesize.to_s} byte\nURL: http://$ip:$port/zabbix_dumps/#{filename}\n(This will of course only work inside the Centillion Network).\n\nRegards,\n\n\tYour hard-working Blackbox Cron @ $vhost/bin/Zabbix_Statistics.rb\n\n\n----\nPS: Don't forget to delete this archive from the webserver otherwise we'll eventually run out of space.\n" > /tmp/tmp_mailbody`
`mutt -F /root/.muttrc -s "Zabbix RDD's Dump for #{lastMonthShort.to_s}" #{to} < /tmp/tmp_mailbody`
`rm -f /tmp/tmp_mailbody`


# Delete stuff which lies around
Dir.chdir( currentDir )
`rm -rf Zabbix_Statistics`


