#!/usr/bin/ruby -w

require 'dm-core'
# require 'dm-validations'
# require 'dm-timestamps'

DataMapper.setup(:default, "sqlite3://#{Dir.pwd}/toopaste.sqlite3")

class Groups
    include DataMapper::Resource

    property :groupid,    Integer,  :serial   => true
    property :name,       String,   :size     => 64, :nullable => false
end

#DataMapper.auto_upgrade!


