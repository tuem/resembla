#!/usr/bin/env ruby

this_dir = File.expand_path(File.dirname(__FILE__))
$LOAD_PATH.unshift(this_dir) unless $LOAD_PATH.include?(this_dir)

require 'grpc'
require 'multi_json'
require 'resembla_services_pb'

include Resembla

def main(queries)
  stub = Resembla::Server::ResemblaService::Stub.new('localhost:50051', :this_channel_is_insecure)
  queries.each do |query|
    puts 'query: ' + query
    stub.find(Resembla::Server::ResemblaRequest.new(query: query)).results.each do |response|
      puts '  text: ' + response.text
    end
  end
end

main ARGV
