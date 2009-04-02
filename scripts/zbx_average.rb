#!/usr/bin/ruby

# Zabbix Item average script
# Copyright 2008 A. Nelson  nelsonab (at) pobox.removethisword.com
# This software is released under the terms of the GNU General Public Licence

# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.

# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.

# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>

require "mysql"
require "optparse"
require "ostruct"

PROGRAM = File.basename($0)

class ZabbixMysql < Mysql
  def initialize(host,user,pass,db)
    super(host,user,pass,db)
  end

  def gethistory(itemid)
#       Returns the history information for itemid
#       Values are returned in a hash of {delta,units,key,values}
#       If the value_type is unknown an exception is raised      
    iteminfo = query("select key_,value_type,delta,Units from items where itemid=#{itemid}")
    row = iteminfo.fetch_row
    key = row[0]
    value_type = row[1].to_i
    delta = row[2].to_i ? true : false
    units = row[3]

    hist_table = ""
    data_type = ""
    case value_type
      when 0
        hist_table = "history"
        data_type =".to_f"
      when 3
        hist_table = "history_uint"
        data_type=".to_i"
      else
        raise "Unknown Value Type"
    end

    results = query("select * from #{hist_table} where itemid=#{itemid}")
    values = []
    results.each_hash do |row|
      time = ::Time.at(row["clock"].to_i)
      val = eval("row[\"value\"]#{data_type}")
      values <<[time,val]
    end
    return {'delta' => delta, 'units' => units, 'key' => key, 'values' => values}
  end
end

class Zbx_Average_App

  def initialize
    @options = OpenStruct.new
    parse
    @options.host = "localhost" if !@options.host
    @options.user = "root" if !@options.user
    @options.password = "" if !@options.password
    @options.database = "zabbix" if !@options.database
    @options.time = 60 if !@options.time
    @options.total = false if !@options.total
    @options.verbose = false if !@options.verbose
    @options.numberonly = false if !@options.numberonly
    @options.nokey = false if !@options.nokey
    
    @zbxdb = ZabbixMysql.new(@options.host, @options.user, @options.password, @options.database)
  end #end initialize



  def parse
    usage = "#{PROGRAM} [options] itemid\n"
    usage << "itemid is the item to print a total of\n"
    usage << "Defaults :  time = 1 hour, host = 'localhost', user = 'root'\n"
    usage << "            database = 'zabbix', password = ''\n\n"
    opts = OptionParser.new(usage) do |opts|
      opts.on("-h","--host HOST","Host to connect to") do |h|
        @options.host = h
      end
      opts.on("-u", "--user USER", "User to connect to database with") do |u|
        @options.user = u
      end
      opts.on("-f", "--password PASSWORD", "Password to use") do |p|
        @options.password = p
      end
      opts.on("-d", "--database DATABASE", "Database to connect to") do |d|
        @options.database = d
      end
      opts.on("-t", "--time TIME", "Time in minutes to compute average") do |t|
        @options.time = t.to_i
      end
      opts.on("-T", "--total", "Print an total not average over time") do
        @options.total = true
      end
      opts.on("-v", "--verbose", "Print verbose output") do
        @options.verbose = true
      end
      opts.on("-n", "--number", "Only print the resulting numbers") do 
        @options.numberonly = true
      end
      opts.on("-K", "--nokey", "Do not show the item key") do
        @options.nokey = true
      end
      opts.on("-?", "--help", "Show this help message") do
        puts opts
      end 
    end


    begin
      @itemid = opts.parse!(ARGV)
    rescue Exception => e
      puts e, "", opts
      exit
    end
    if @itemid.length != 1 
      puts "Incorrect number of options"
      puts opts
      exit
    else
      @options.itemid = @itemid[0].to_i
    end
  end  #end parse

  def run
    results = @zbxdb.gethistory(@options.itemid)
    values = results['values']
    units = results['units']
    delta = results['delta']
    key = results['key']

    lasttime = values.last[0]
    cutoff = lasttime - @options.time*60
    i = values.length-1
    found = false
    while !found && i>=0
      found = true if values[i][0]<=cutoff
      i-=1
    end
    values.slice!(0..i) if i>0
    values.each do |value|
      puts "#{value.join(",")}" if @options.verbose
    end

    if delta
      total = 0
      for i in 0..values.length-2
        total += (values[i+1][0]-values[i][0]).to_i*values[i+1][1]
      end
    else
      total = values.last[0]-values.first[0]
    end

    outputstr = @options.numberonly || @options.nokey ? "" : "#{key} "
    avg = total/(values.last[0]-values.first[0]).to_i
    if @options.total
      outputstr << "#{@options.numberonly ? "" : "Total: "}#{total}"
    else
      outputstr << "#{@options.numberonly ? "" : "Average: "}#{avg}"
    end
    outputstr << units if !@options.numberonly
    puts outputstr
  end

end

zbx_app = Zbx_Average_App.new
zbx_app.run

